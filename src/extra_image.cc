/*
 * extra_image.cc
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

#ifdef USE_IMLIB

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib.h>
#include "main.h"
#include "extra_image.h"

ImlibData* ExtraImage::m_idImlib;

ExtraImage::ExtraImage(char* filename)
{
  m_error = LoadImage(filename);
}

// create a temporary file because Imlib does not provide any functions
// to create an image from raw data...
ExtraImage::ExtraImage(char* data, int len)
{
  char filename[16] = "/tmp/qvwmXXXXXX";  /* must not be constant */
  int fd;

  if ((fd = mkstemp(filename)) < 0) {
    QvwmError("cannot create a temporary file");
    m_error = -1;
    return;
  }

  if (write(fd, data, len) < len) {
    QvwmError("cannot write to a temporary file");
    m_error = -1;
    close(fd);
    return;
  }
    
  m_error = LoadImage(filename);

  unlink(filename);
  close(fd);
}

int ExtraImage::LoadImage(char* filename)
{
  m_im = Imlib_load_image(m_idImlib, filename);
  if (m_im == NULL)
    return -1;
    
  m_size = Dim(m_im->rgb_width, m_im->rgb_height);
  
  Imlib_render(m_idImlib, m_im, m_size.width, m_size.height);

  m_pix = Imlib_move_image(m_idImlib, m_im);
  m_mask = Imlib_move_mask(m_idImlib, m_im);

  XGCValues gcv;
  gcv.clip_mask = m_mask;
  m_gc = XCreateGC(display, root, GCClipMask, &gcv);

  m_import = True;  // pixmap and mask are managed by Imlib

  return 0;
}

// ~QvImage is not executed because m_import is True
ExtraImage::~ExtraImage()
{
  Imlib_destroy_image(m_idImlib, m_im);
  Imlib_free_pixmap(m_idImlib, m_pix);

  if (m_gc)
    XFreeGC(display, m_gc);
}

void ExtraImage::Initialize()
{
  m_idImlib = Imlib_init(display);
}

#endif // USE_IMLIB
