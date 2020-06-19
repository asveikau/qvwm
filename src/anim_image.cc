/*
 * anim_image.cc
 *
 * Copyright (C) 1995-2001 Kenichi Kourai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with qvwm; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ANIM_IMAGE

#include <stdlib.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "anim_image.h"
#include "pixmap_image.h"
#include "timer.h"
#include "callback.h"
#include "qvwmrc.h"
#ifdef USE_IMLIB
#include "extra_image.h"
#endif

//
// animation file format
//   'ANIM' <# of images(4B)>                       ; header
//   'DATA' <image data size(4B)> <interval(4B)>
//          <image name length(4B)> <image name(4B align)>
//          <data(4B align)>                        ; image data
//     or
//   'REFE' <frame number(4B)> <interval(4B)>       ; reference(backward only)
//
//   All value is big endian.
//   <interval> is the time during which the image is displayed.
//

AnimImage::AnimImage()
{
  m_imageNum = 0;
  m_images = NULL;
  m_import = True;
}

AnimImage::AnimImage(char* filename, Timer* timer)
{
  FILE* fp;
  struct stat st;
  int num;
  char** array;
  int* len;
  int* ival;

  m_images = NULL;
  m_import = True;  // m_pix, m_mask and m_gc of AnimImage are fake

  if ((fp = fopen(filename, "r")) == NULL) {
    QvwmError("cannot open animation file: '%s'", filename);
    m_error = -1;
    return;
  }

  if (fstat(fileno(fp), &st) < 0) {
    QvwmError("cannot access animation file: '%s'", filename);
    m_error = -1;
    fclose(fp);
    return;
  }

  if (ParseHeader(fp, &num, filename) == -1) {
    m_error = -1;
    fclose(fp);
    return;
  }

  // use the first image if not animating image
  if (!ImageAnimation)
    num = 1;

  array = new char*[num];
  len = new int[num];
  ival = new int[num];

  for (int i = 0; i < num; i++) {
    if (ReadImageData(fp, &array[i], &len[i], &ival[i], st.st_size, filename)
	== -1) {
      m_error = -1;
      fclose(fp);

      for (int j = 0; j < i; j++)
	delete array[j];
      delete [] array;
      delete [] len;
      delete [] ival;

      return;
    }
  }

  CreateImages(array, len, ival, num, timer, filename);

  for (int j = 0; j < num; j++)
    delete array[j];
  delete [] array;
  delete [] len;

  fclose(fp);

  if (m_imageNum > 0)
    SetCurrentImage();
}

int AnimImage::ParseHeader(FILE* fp, int* num, char* filename)
{
  char id[4];

  // read and check animation file id
  if (fread(id, 4, 1, fp) < 1) {
    QvwmError("cannot read animation file: '%s'", filename);
    return -1;
  }
  if (memcmp(id, "ANIM", 4) != 0) {
    QvwmError("'%s' is not animation file", filename);
    return -1;
  }
    
  // read # of images
  if (GetBWord(fp, num) == -1) {
    QvwmError("cannot read animation image file: '%s'", filename);
    return -1;
  }
  
  return 0;
}

int AnimImage::ReadImageData(FILE* fp, char** data, int* len, int* ival,
			     int filesize, char* filename)
{
  char id[4];
  char pad[3];
  int namlen;
  char name[256]; // XXX contained image name length is limited by 256

  // read and check the start of image section
  if (fread(id, 4, 1, fp) < 1) {
    QvwmError("cannot read image section: '%s'", filename);
    m_error = 1;
    return -1;
  }

  if (memcmp(id, "DATA", 4) == 0) {
    // read image data length
    if (GetBWord(fp, len) == -1) {
      QvwmError("cannot read contained image length: '%s'", filename);
      return -1;
    }
    if (*len < 0) {
      QvwmError("contained image length is negative: '%s'", filename);
      return -1;
    }

    // read image display interval
    if (GetBWord(fp, ival) == -1) {
      QvwmError("cannot read interval: '%s'", filename);
      return -1;
    }
    if (*ival < 0) {
      QvwmError("interval is negative: '%s'", filename);
      return -1;
    }

    // read contained image name length
    if (GetBWord(fp, &namlen) == -1) {
      QvwmError("cannot read contained image name length: '%s'", filename);
      return -1;
    }
    if (namlen > 0) {
      if (ftell(fp) + namlen > filesize) {
	QvwmError("contained image name length is too large: '%s'", filename);
	return -1;
      }

      if ((namlen & 3) != 0)
	namlen = (namlen + 3) / 4 * 4;
      
      // name is not used in qvwm now...
      if (fread(name, namlen, 1, fp) < 1) {
	QvwmError("cannot read contained image name: '%s'", filename);
	return -1;
      }
    }
    else if (namlen < 0) {
      QvwmError("contained image name length is negative: '%s'", filename);
      return -1;
    }

    if (ftell(fp) + *len > filesize) {
      QvwmError("contained image length is too large: '%s'", filename);
      return -1;
    }

    *data = new char[*len];

    if (fread(*data, *len, 1, fp) < 1) {
      QvwmError("cannot read contained image data: '%s'", filename);
      delete *data;
      return -1;
    }

    // read until 4 byte alignment
    if ((*len & 3) != 0)
      fread(pad, 4 - (*len & 3), 1, fp);
  }
  else if (memcmp(id, "REFE", 4) == 0) {
    if (GetBWord(fp, len) == -1) {
      QvwmError("cannot read contained image length: '%s'", filename);
      return -1;
    }
    if (*len < 0) {
      QvwmError("contained image length is negative: '%s'", filename);
      return -1;
    }

    if (GetBWord(fp, ival) == -1) {
      QvwmError("cannot read interval: '%s'", filename);
      return -1;
    }
    if (*ival < 0) {
      QvwmError("interval is negative: '%s'", filename);
      return -1;
    }

    *data = NULL;  // stands for frame reference
  }
  else {
    QvwmError("'%s' is not animation file", filename);
    return -1;
  }

  return 0;
}

int AnimImage::GetBWord(FILE* fp, int* word)
{
  unsigned char data[4];

  if (fread(data, 4, 1, fp) < 1)
    return -1;

  *word = (int)(data[3] + ((int)data[2] << 8) + ((int)data[1] << 16)
		+ ((int)data[0] << 24));
  
  return 0;
}

void AnimImage::CreateImages(char** array, int* len, int* ival, int num,
			     Timer* timer, char* filename)
{
  m_imageNum = num;
  m_curNum = 0;
  m_speed = ival;
  m_timer = timer;

  Init();

  for (int i = 0; i < m_imageNum; i++) {
    if (array[i] == NULL && len[i] < i)
      m_images[i] = m_images[len[i]]->Duplicate();
    else
      m_images[i] = CreateImageFromData(array[i], len[i]);

    if (m_images[i] == NULL) {
      QvwmError("unsupported image is contained in %s", filename);
      m_error = -1;
      return;
    }
    else if ((m_error = m_images[i]->GetError())) {
      m_images[i]->OutputError(filename);
      return;
    }
  }
}

QvImage* AnimImage::CreateImageFromData(char* data, int len)
{
  QvImage* img;

  if (strncmp(data, "/* XPM */", 9) == 0)
    img = new PixmapImage(data, len);
  else {
#ifdef USE_IMLIB
    img = new ExtraImage(data, len);
#else
    img = NULL;
#endif
  }

  return img;
}

void AnimImage::Init()
{
  m_images = new QvImage*[m_imageNum];
  
  for (int i = 0; i < m_imageNum; i++)
    m_images[i] = NULL;  // for correct destruction in error

  m_winDisp = None;
  m_ptDisp = Point(0, 0);
  m_bgImg = False;

  if (m_imageNum > 1 && m_timer)
    m_timer->SetTimeout(m_speed[m_curNum],
			new Callback<AnimImage>(this, &AnimImage::Display));
}

AnimImage::~AnimImage()
{
  if (m_images == NULL)
    return;

  for (int i = 0; i < m_imageNum; i++)
    QvImage::Destroy(m_images[i]);

  delete [] m_images;

  delete [] m_speed;

  if (m_imageNum > 1 && m_timer)
    m_timer->ResetTimeout(new Callback<AnimImage>(this, &AnimImage::Display));
}

AnimImage* AnimImage::Duplicate()
{
  AnimImage* img = new AnimImage();
  int i;

  img->m_imageNum = m_imageNum;
  img->m_curNum = m_curNum;
  img->m_timer = m_timer;
  img->m_speed = new int[m_imageNum];

  for (i = 0; i < m_imageNum; i++)
    img->m_speed[i] = m_speed[i];

  img->Init();

  for (i = 0; i < img->m_imageNum; i++)
    img->m_images[i] = m_images[i]->Duplicate();

  if (img->m_imageNum > 0)
    img->SetCurrentImage();

  return img;
}

void AnimImage::SetCurrentImage()
{
  m_pix = m_images[m_curNum]->GetPixmap();
  m_mask = m_images[m_curNum]->GetMask();
  m_gc = m_images[m_curNum]->GetGC();
  m_size = m_images[m_curNum]->GetSize();
}

void AnimImage::ChangeNextImage()
{
  if (++m_curNum >= m_imageNum)
    m_curNum = 0;

  SetCurrentImage();
}

void AnimImage::Display(Window win, const Point& pt, const Dim &size)
{
  m_winDisp = win;
  m_ptDisp = pt;
  m_size = size;

  QvImage::Display(win, pt);
}

void AnimImage::Display()
{
  ChangeNextImage();

  if (m_cbPre)
    m_cbPre->Execute();

  if (m_cbDisp)
    m_cbDisp->Execute();  // for special purpose
  else if (m_winDisp != None) {
    if (m_bgImg) {
      QvImage::SetBackground(m_winDisp);
      XClearArea(display, m_winDisp, 0, 0, 0, 0, True);
    }
    else {
      XClearArea(display, m_winDisp,
		 m_ptDisp.x, m_ptDisp.y, m_size.width, m_size.height, False);
      QvImage::Display(m_winDisp, m_ptDisp, m_size);
      XFlush(display);
    }
  }

  if (m_cbPost)
    m_cbPost->Execute();

  if (m_imageNum > 0 && m_timer)
    m_timer->SetTimeout(m_speed[m_curNum],
			new Callback<AnimImage>(this, &AnimImage::Display));
}

void AnimImage::SetBackground(Window win)
{
  m_bgImg = True;
  m_winDisp = win;

  QvImage::SetBackground(win);
}

QvImage* AnimImage::GetOffsetImage(const Point& pt)
{
  AnimImage* img = new AnimImage();
  int i;

  img->m_imageNum = m_imageNum;
  img->m_curNum = m_curNum;
  img->m_timer = m_timer;
  img->m_speed = new int[m_imageNum];

  for (i = 0; i < m_imageNum; i++)
    img->m_speed[i] = m_speed[i];

  img->Init();

  for (i = 0; i < img->m_imageNum; i++)
    img->m_images[i] = m_images[i]->GetOffsetImage(pt);

  if (img->m_imageNum > 0)
    img->SetCurrentImage();

  return img;
}

#endif
