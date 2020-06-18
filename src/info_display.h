/*
 * info_display.h
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

#ifndef INFO_DISPLAY_H_
#define INFO_DISPLAY_H_

#include "misc.h"

class InfoDisplay {
private:
  Window frame;
  Rect rc;
  
  char strInfo[64];

public:
  InfoDisplay();
  ~InfoDisplay();

  void Map();
  void Unmap();

  void Draw();

  void NotifyPosition(const Point& pt);
  void NotifyShape(const Rect& rect);
};

#endif // INFO_DISPLAY_H_
