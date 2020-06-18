/*
 * anim_image.h
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

#ifndef ANIM_IMAGE_H_
#define ANIM_IMAGE_H_

#include "image.h"
#include "misc.h"

class AnimImage : public QvImage {
private:
  int m_imageNum;
  QvImage** m_images;
  int* m_speed;
  Timer* m_timer;

  int m_curNum;
  Window m_winDisp;
  Point m_ptDisp;
  Bool m_bgImg;
  
private:
  void Init();

  int ParseHeader(FILE* fp, int* num, char* filename);
  int ReadImageData(FILE* fp, char** data, int* len, int* ival, int filesize,
		    char* filename);
  int GetBWord(FILE* fp, int* word);

  void CreateImages(char** array, int* len, int* ival, int num,
		    Timer* timer, char* filename);
  QvImage* CreateImageFromData(char* data, int len);

protected:
  void SetCurrentImage();
  void ChangeNextImage();
  void Display();

public:
  AnimImage();
  AnimImage(char* filename, Timer* timer);
  ~AnimImage();

  AnimImage* Duplicate();

  void Display(Window win, const Point& pt);
  void SetBackground(Window win);

  QvImage* GetOffsetImage(const Point& pt);
};

#endif // ANIM_IMAGE_H_
