/*
 * accessary.h
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

#ifndef ACCESSORY_H_
#define ACCESSORY_H_

#include "misc.h"

class QvImage;

class Accessory {
private:
  enum Mode { ACC_BACK, ACC_ONTOP, ACC_APP };

private:
  Window m_win;
  QvImage* m_img;
  Rect m_rc;

  char* m_filename;
  char* m_pos;
  Mode m_mode;
  pid_t m_pid;

  Pixmap m_shapeMask;
  GC m_gcShape;

private:
  void CreateShapedWindow();
  Point CalcPosition(char* pos, int* gravity);
  void SetBgImage();

public:
  Accessory(char* filename, char* pos, char* mode);
  ~Accessory();

  void CreateAccessory();
  void MapAccessory();
  void EventLoop();
};

#endif // ACCESSORY_H_

