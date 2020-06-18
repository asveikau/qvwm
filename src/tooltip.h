/*
 * tooltip.h
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

#ifndef TOOLTIP_H_
#define TOOLTIP_H_

#include "misc.h"

class Tooltip {
private:
  Window m_win;

  char* m_str;
  XFontSet* m_fs;
  Rect m_rc;

  Bool m_mapped;
  
public:
  static XContext context;

public:
  Tooltip(char* str = NULL, XFontSet* fs = NULL);
  ~Tooltip();

  Bool IsMapped() const { return m_mapped; }
  void SetString(char* str, XFontSet* fs = NULL);

  void CreateWindow();
  void MapTooltip();
  void UnmapTooltip();
  void DrawTooltip();

  void ResizeTooltip();
  Point AdjustPosition(const Point& pt);

  void SetTimer();
  void ResetTimer();
  void Disable();

  static void Initialize();
};

#endif // TOOPTIP_H_
