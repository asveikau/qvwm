/*
 * extra_image.h
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

#ifndef EXTRA_IMAGE_H_
#define EXTRA_IMAGE_H_

#ifdef USE_IMLIB

#include <Imlib.h>
#include "image.h"

class ExtraImage : public QvImage {
private:
  ImlibImage* m_im;

public:
  static ImlibData* m_idImlib;

private:
  int LoadImage(char* filename);

public:
  ExtraImage(char* filename);
  ExtraImage(char* data, int len);
  ~ExtraImage();

  static void Initialize();
};

#endif // USE_IMLIB

#endif // EXTRA_IMAGE_H_
