/*
 * image.h
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

#ifndef QV_IMAGE_H_
#define QV_IMAGE_H_

#include <stdio.h>
#include "misc.h"

class BasicCallback;

class QvImage {
protected:
  Pixmap m_pix;
  Pixmap m_mask;
  GC m_gc;
  Dim m_size;

  int m_refcnt;
  int m_error;
  Bool m_import;  // m_pix is create extenally

  BasicCallback* m_cbPre;
  BasicCallback* m_cbDisp;
  BasicCallback* m_cbPost;

private:
  void Init();

protected:
  virtual ~QvImage();

public:
  QvImage();
  QvImage(Pixmap pix, GC gc, const Dim& size);

  virtual QvImage* Duplicate();
  static void Destroy(QvImage* img);

  Pixmap GetPixmap() const { return m_pix; }
  Pixmap GetMask() const { return m_mask; }
  GC GetGC() const { return m_gc; }
  Dim GetSize() const { return m_size; }
  int GetError() const { return m_error; }

  virtual void Display(Window d, const Point& pt);
  virtual void SetBackground(Window win);

  virtual QvImage* GetOffsetImage(const Point& pt);

  void OutputError(char* filename);

  void SetPreCallback(BasicCallback* cb) { m_cbPre = cb; }
  void SetDisplayCallback(BasicCallback* cb) { m_cbDisp = cb; }
  void SetPostCallback(BasicCallback* cb) { m_cbPost = cb; }
};

#define IMG_DATA_ERROR    -1
#define IMG_COLOR_ERROR   -2
#define IMG_MEMORY_ERROR  -3
#define IMG_OPEN_ERROR    -4
#define IMG_FILE_ERROR    -5
#define IMG_UNKNOWN_ERROR -6

#endif // QV_IMAGE_H_
