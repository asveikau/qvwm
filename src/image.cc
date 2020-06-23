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
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "image.h"
#include "callback.h"
#include <new>

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

  m_scaleCache = NULL;
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

  Destroy(m_scaleCache);

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

static Pixmap Scale(
  Display *display,
  Window root,
  Pixmap srcPixmap,
  GC gc,
  const Dim &oldSize,
  const Dim &newSize,
  double xscale,
  double yscale)
{
  XImage *img = NULL;
  char *dst = NULL;
  Pixmap pixmap = 0;

  img = XGetImage(display, srcPixmap, 0, 0, oldSize.width, oldSize.height, AllPlanes, ZPixmap);

  if (img) {
    int bpp = img->bits_per_pixel/8;
    const int pad = 8;
    int line_width = (newSize.width * img->bits_per_pixel + pad - 1) / pad * pad / 8;
    dst = (char*)calloc(newSize.height, line_width);
    if (dst) {

      //
      // XXX this algorithm is very crude!
      //

      char *row = dst;
      for (int y = 0; y<newSize.height; ++y) {
        char *pixel = row;
        for (int x = 0; x<newSize.width; ++x) {
          int srcX = x / xscale;
          int srcY = y / yscale;
          if (bpp) {
            memcpy(pixel, img->data + img->bytes_per_line * srcY + (srcX * bpp), bpp);
            pixel += bpp;
          }
          else {
            unsigned char *p = (unsigned char*)pixel;
            unsigned char *s = (unsigned char*)img->data + img->bytes_per_line * srcY + srcX/8;
            unsigned char bit = (((*s) & (1<<(srcX % 8))) ? 1 : 0) << (x%8);
            if (bit)
              *p |= bit;
            else
              *p &= ~bit;
            if (x%8 == 7)
              ++pixel;
          }
        }
        row += line_width;
      }

      if (bpp) {
        int depth = img->depth;
        XDestroyImage(img);
        img = NULL;
        XImage *dstImage = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, dst, newSize.width, newSize.height, pad, line_width);
        if (dstImage) {
          dst = NULL; // XImage* now owns data
          pixmap = XCreatePixmap(display, root, newSize.width, newSize.height, depth);
          if (pixmap)
            XPutImage(display, pixmap, gc, dstImage, 0, 0, 0, 0, newSize.width, newSize.height);
          XDestroyImage(dstImage);
        }
      }
      else {
        pixmap = XCreatePixmapFromBitmapData(display, root, dst, newSize.width, newSize.height, 1, 0, 1);
      }
    }
    if (img)
      XDestroyImage(img);
    free(dst);
  }

  return pixmap;
}

QvImage* QvImage::Scale(float xscale, float yscale)
{
  Dim newSize(m_size.width * xscale, m_size.height * yscale);
  QvImage *r = NULL;
  Pixmap pixmap = 0;
  Pixmap mask = 0;

  if (m_scaleCache)
  {
    if (m_scaleCache->m_size.width == newSize.width && m_scaleCache->m_size.height == newSize.height)
    {
      r = m_scaleCache;
      r->m_refcnt++;
      return r;
    }
    Destroy(m_scaleCache);
    m_scaleCache = NULL;
  }

  GC gc = XCreateGC(display, root, 0, 0);

  pixmap = ::Scale(display, root, m_pix, gc, m_size, newSize, xscale, yscale);

  if (pixmap) {
    r = new (std::nothrow) QvImage(pixmap, gc, newSize);
    if (!r) {
      XFreePixmap(display, pixmap);
    }
    else {
      if (m_mask) {
        r->m_mask = ::Scale(display, root, m_mask, gc, m_size, newSize, xscale, yscale);
        if (r->m_mask) {
          XGCValues gcv;
          gcv.clip_mask = r->m_mask;
          XChangeGC(display, gc, GCClipMask, &gcv);
        }
      }
      gc = 0;
    }
  }

  if (gc)
    XFreeGC(display, gc);

  if (r) {
    m_scaleCache = r;
    r->m_refcnt++;
  }

  return r;
}

void QvImage::Display(Window win, const Point& pt, const Dim &size)
{
  if (win == None)
    return;

  ASSERT(m_refcnt > 0)

  if (((&size) == &m_size) || (size.width == m_size.width && size.height == m_size.height)) {
    XSetClipOrigin(display, m_gc, pt.x, pt.y);
    XCopyArea(display, m_pix, win, m_gc,
              0, 0, m_size.width, m_size.height, pt.x, pt.y);
  }
  else if (m_size.width && m_size.height) {
    float xscale = (size.width + 0.0f) / m_size.width;
    float yscale = (size.height + 0.0f) / m_size.height;

    QvImage *scaled = Scale(xscale, yscale);
    if (scaled) {
      scaled->Display(win, pt);
      Destroy(scaled);
    }
  }
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

void QvImage::OutputError(const char* filename)
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
