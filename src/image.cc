/*
 * image.cc
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
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "image.h"
#include "callback.h"

QvImage::QvImage()
{
  Init();
}

QvImage::QvImage(Pixmap pix, GC gc, const Dim& size)
{
  Init();

  m_pix = pix;
  m_gc = gc;
  m_size = size;
}

void QvImage::Init()
{
  m_pix = None;
  m_mask = None;
  m_gc = NULL;

  m_refcnt = 1;
  m_error = 0;

  m_cbPre = NULL;
  m_cbDisp = NULL;
  m_cbPost = NULL;

  m_import = False;
}

QvImage::~QvImage()
{
  ASSERT(m_refcnt == 0);

  if (m_import)
    return;

  if (m_pix)
    XFreePixmap(display, m_pix);
  if (m_mask)
    XFreePixmap(display, m_mask);
  if (m_gc)
    XFreeGC(display, m_gc);

  delete m_cbPre;
  delete m_cbDisp;
  delete m_cbPost;
}

QvImage* QvImage::Duplicate()
{
  m_refcnt++;

  return this;
}

void QvImage::Destroy(QvImage* img)
{
  if (img == NULL)
    return;

  if (--img->m_refcnt == 0)
    delete img;
}

void QvImage::Display(Window win, const Point& pt)
{
  if (win == None)
    return;

  ASSERT(m_refcnt > 0)

  XSetClipOrigin(display, m_gc, pt.x, pt.y);
  XCopyArea(display, m_pix, win, m_gc,
            0, 0, m_size.width, m_size.height, pt.x, pt.y);
}

void QvImage::SetBackground(Window win)
{
  XSetWindowBackgroundPixmap(display, win, m_pix);
}

/*
 * Adjust pixmap according to a start point(pt).
 *
 *         size.width
 *        +-------+---+      +---+-------+
 *        |4      |2  |      |1  |3      |
 * size.  |    off|   |  =>  +---+-------+
 * height +-------+---+      |2  |4      |
 *        |3      |1  |      |   |       |
 *        +-------+---+      +---+-------+
 *            pix                 fpix
 */
QvImage* QvImage::GetOffsetImage(const Point& pt)
{
  Pixmap fpix;
  Point offset;

  fpix = XCreatePixmap(display, root, m_size.width, m_size.height, depth);

  offset.x = pt.x % m_size.width;
  offset.y = pt.y % m_size.height;

  // copy original bottom-right to top-left
  XCopyArea(display, m_pix, fpix, gc,
	    offset.x, offset.y,
	    m_size.width - offset.x, m_size.height - offset.y,
	    0, 0);

  // copy original top-right to bottom-left
  XCopyArea(display, m_pix, fpix, gc,
	    offset.x, 0, m_size.width - offset.x, offset.y,
	    0, m_size.height - offset.y);

  // copy original bottom-left to top-right
  XCopyArea(display, m_pix, fpix, gc,
	    0, offset.y, offset.x, m_size.height - offset.y,
	    m_size.width - offset.x, 0);

  // copy original top-left to bottom-right
  XCopyArea(display, m_pix, fpix, gc,
	    0, 0, offset.x, offset.y,
	    m_size.width - offset.x, m_size.height - offset.y);

  GC gcNew = XCreateGC(display, root, 0, 0);
  QvImage* img = new QvImage(fpix, gcNew, m_size);

  return img;
}

void QvImage::OutputError(char* filename)
{
  switch (m_error) {
  case IMG_DATA_ERROR:
    QvwmError("data error: '%s'", filename);
    break;

  case IMG_COLOR_ERROR:
    QvwmError("color error: '%s'", filename);
    break;

  case IMG_MEMORY_ERROR:
    QvwmError("not enough memory: '%s'", filename);
    break;
    
  case IMG_OPEN_ERROR:
    QvwmError("cannot open file: '%s'", filename);
    break;
    
  case IMG_FILE_ERROR:
    QvwmError("cannot read file: '%s'", filename);
    break;

  case IMG_UNKNOWN_ERROR:
    QvwmError("unknown error: '%s'", filename);
    break;
  }
}
