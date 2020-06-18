/*
 * pixmap_image.cc
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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include "main.h"
#include "pixmap_image.h"
#include "extra_image.h"

//
// Xpm file format able to be read
//   " <width> <height> <num_colors> <chars_per_pixel> "
//   "<char> c <color>"
//   ...
//   "data for line0"
//   ...
//
// - ignore lines not beginning with "
// - " must be the first character of a line
//

PixmapImage::PixmapImage(char* filename)
{
  FILE* fp;
  struct stat st;
  char* raw;

#ifdef USE_IMLIB
  m_im = NULL;
#endif

  if ((fp = fopen(filename, "r")) == NULL) {
    QvwmError("cannot open file: '%s'", filename);
    m_error = IMG_OPEN_ERROR;
    return;
  }

  if (fstat(fileno(fp), &st) < 0) {
    QvwmError("cannot access file: '%s'", filename);
    m_error = IMG_FILE_ERROR;
    fclose(fp);
    return;
  }
  
  raw = new char[st.st_size];

  if (fread(raw, st.st_size, 1, fp) < 1) {
    QvwmError("cannot read file: '%s'", filename);
    m_error = IMG_FILE_ERROR;
    fclose(fp);
    return;
  }

  m_error = CreateImage(raw, st.st_size);

  fclose(fp);

  OutputError(filename);
}

PixmapImage::PixmapImage(char** data)
{
#ifdef USE_IMLIB
  m_im = NULL;
#endif

  m_error = CreatePixmap(data);
}

PixmapImage::PixmapImage(char* raw, int len)
{
#ifdef USE_IMLIB
  m_im = NULL;
#endif

  m_error = CreateImage(raw, len);
}

// ~QvImage is not executed when using Imlib
PixmapImage::~PixmapImage()
{
#ifdef USE_IMLIB
  if (m_im) {
    Imlib_destroy_image(ExtraImage::m_idImlib, m_im);
    Imlib_free_pixmap(ExtraImage::m_idImlib, m_pix);

    if (m_gc)
      XFreeGC(display, m_gc);
  }
#endif  
}

int PixmapImage::CreateImage(char* raw, int len)
{
  char** data;
  int error;

  data = ParseXpmData(raw, len);
  if (data == NULL)
    return IMG_DATA_ERROR;
  
  error = CreatePixmap(data);
  
  for (int i = 0; data[i] != NULL; i++)
    delete data[i];
  delete [] data;

  return error;
}

int PixmapImage::CreatePixmap(char** data)
{
#ifdef USE_IMLIB
  m_im = Imlib_create_image_from_xpm_data(ExtraImage::m_idImlib, data);
  if (m_im == NULL)
    return IMG_UNKNOWN_ERROR;
  
  m_size = Dim(m_im->rgb_width, m_im->rgb_height);
  
  Imlib_render(ExtraImage::m_idImlib, m_im, m_size.width, m_size.height);
  
  m_pix = Imlib_move_image(ExtraImage::m_idImlib, m_im);
  m_mask = Imlib_move_mask(ExtraImage::m_idImlib, m_im);
  
  XGCValues gcv;
  gcv.clip_mask = m_mask;
  m_gc = XCreateGC(display, root, GCClipMask, &gcv);
  
  m_import = True;  // pixmap and mask are managed by Imlib
  
  return 0;
#else
  XpmAttributes attr;
  int error;

  attr.valuemask = XpmColormap | XpmDepth;
  attr.colormap = DefaultColormap(display, screen);
  attr.depth = DefaultDepth(display, screen);

  error = XpmCreatePixmapFromData(display, root, data, &m_pix, &m_mask, &attr);
  if (error == XpmSuccess || error == XpmColorError) {
    XGCValues gcv;
    gcv.clip_mask = m_mask;
    m_gc = XCreateGC(display, root, GCClipMask, &gcv);
    m_size = Dim(attr.width, attr.height);
    return 0;
  }
  
  switch (error) {
  case XpmColorFailed:
    return IMG_COLOR_ERROR;
    
  case XpmNoMemory:
    return IMG_MEMORY_ERROR;
    
  default:
    return IMG_UNKNOWN_ERROR;
  }
#endif // USE_IMLIB
}

char** PixmapImage::ParseXpmData(char* raw, int len)
{
  char* ptr;
  int width, height, num_colors, chars_per_pixel;
  char** data;
  int i, dlen;

  // parse pixmap information
  if ((ptr = GetNextData(raw, len)) != NULL) {
    if (sscanf(ptr, "%d %d %d %d", &width, &height, &num_colors,
	       &chars_per_pixel) != 4)
      return NULL;
  }
  else
    return NULL;

  // allocate data buffer
  data = new char*[1 + num_colors + height + 1];
  data[1 + num_colors + height] = NULL;  // terminate

  data[0] = new char[80];
  sprintf(data[0], "%d %d %d %d", width, height, num_colors, chars_per_pixel);

  // parse color data
  for (i = 1; i < 1 + num_colors; i++) {
    if ((ptr = GetNextData(raw, len)) != NULL) {
      data[i] = new char[raw - ptr];
      memcpy(data[i], ptr, raw - ptr - 1);
      data[i][raw - ptr - 1] = '\0';
    }
    else {
      for (int j = 0; j < i; j++)
	delete data[j];
      delete [] data;
      return NULL;
    }
  }

  // parse pixel data
  dlen = width * chars_per_pixel;

  for (; i < 1 + num_colors + height; i++) {
    if ((ptr = GetNextData(raw, len)) != NULL) {
      data[i] = new char[dlen + 1];
      memcpy(data[i], ptr, dlen);
      data[i][dlen] = '\0';
    }
    else {
      for (int j = 0; j < i; j++)
	delete data[j];
      delete [] data;
      return NULL;
    }
  }      

  return data;
}

// return the next data enclosed with "
char* PixmapImage::GetNextData(char*& raw, int& len)
{
  char* ptr;

  while (*raw != '\"' && len > 0) {
    raw++;
    len--;
  }
  raw++;
  len--;

  if (len > 0)
    ptr = raw;
  else
    return NULL;

  while (*raw != '\"' && len > 0) {
    raw++;
    len--;
  }
  raw++;
  len--;

  if (len > 0)
    return ptr;
  else
    return NULL;
}
