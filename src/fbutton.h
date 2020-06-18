/*
 * fbutton.h
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

#ifndef _FBUTTON_H_
#define _FBUTTON_H_

#include "button.h"

class Qvwm;
class QvImage;

/*
 * FrameButton class
 */
class FrameButton : public Button {
protected:
  QvImage* img;
  Qvwm* qvWm;             // pointer to Qvwm that this button belong to

  static QvImage* imgButton[4];
  static char* desc[4];

  static const int BUTTON_WIDTH = 12;
  static const int BUTTON_HEIGHT = 10;

public:
  // Frame button name
  enum ButtonName { MINIMIZE, MAXIMIZE, CLOSE, RESTORE };

public:
  FrameButton(Qvwm* qvwm, Window parent, const Rect& rc);
  virtual ~FrameButton();

  void ChangeImage(ButtonName bn);
  void DrawButton();
  virtual void ExecButtonFunc(ButtonState bs) = 0;
  void Button1Press();

  static void Initialize();
};

class FrameButton1 : public FrameButton {
public:
  FrameButton1(Qvwm* qvWm, Window parent, const Rect& rc)
    : FrameButton(qvWm, parent, rc) {}

  void ExecButtonFunc(ButtonState bs);
};

class FrameButton2 : public FrameButton {
public:
  FrameButton2(Qvwm* qvWm, Window parent, const Rect& rc)
    : FrameButton(qvWm, parent, rc) {}
  void ExecButtonFunc(ButtonState bs);
};

class FrameButton3 : public FrameButton {
public:
  FrameButton3(Qvwm* qvWm, Window parent, const Rect& rc)
    : FrameButton(qvWm, parent, rc) {}
  void ExecButtonFunc(ButtonState bs);
};

#endif // _FBUTTON_H_
