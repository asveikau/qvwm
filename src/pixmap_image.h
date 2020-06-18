/*
 * pixmap_image.h
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

#ifndef PIXMAP_IMAGE_H_
#define PIXMAP_IMAGE_H_

#ifdef USE_IMLIB
#include <Imlib.h>
#endif
#include "image.h"

class PixmapImage : public QvImage {
private:
#ifdef USE_IMLIB
  ImlibImage* m_im;
#endif

private:
  int CreateImage(char* raw, int len);
  int CreatePixmap(char** data);
  char** ParseXpmData(char* raw, int len);
  char* GetNextData(char*& raw, int& len);

protected:
  ~PixmapImage();

public:
  PixmapImage() {}
  PixmapImage(char* raw, int len);
  PixmapImage(char* filename);
  PixmapImage(char** data);
};

#endif // PIXMAP_IMAGE_H_
