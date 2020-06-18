/*
 * startmenu.h
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

#ifndef _STARTMENU_H_
#define _STARTMENU_H_

#include "menu.h"

class QvImage;

class StartMenu : public Menu {
private:
  Window logo;                      // window for logo mark in the left side
  Window* itemFocus;                // transparent window for each item
                                    // this includes the parts of logo.
private:
  static QvImage* imgLogoMark;           // logo mark
  static QvImage* imgStart[2];
  
  static const int LOGO_WIDTH = 21;

public:
  StartMenu(MenuElem* mItem);
  ~StartMenu();
  void MapMenu();
  void UnmapMenu();
  void DrawMenu(Window win);
  int FindItem(Window win);
  void ExecFunction(FuncNumber fn, int num);

  static void Initialize();
};

#endif // _STARTMENU_H_
