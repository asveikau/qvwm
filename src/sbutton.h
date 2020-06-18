/*
 * sbutton.h
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

#ifndef _SBUTTON_H_
#define _SBUTTON_H_

#include "tbutton.h"

/*
 * StartButton class
 */
class StartButton : public TaskbarButton {
public:
  int buttonWidth;

private:
  void FillBackground();
  void DrawName();

public:
  StartButton(Qvwm* qvWm, int some_width);
  ~StartButton() {}
  
  void DrawButton();
  void DrawActive();

  void ExecButtonFunc(ButtonState state) {}
  void Button1Press();
  void Button3Release(const Point& ptRoot) {}
  void Enter();
};

#endif // _SBUTTON_H_
