/*
 * button.h
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

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "misc.h"

class Tooltip;
class QvImage;

/*
 * Button class
 */
class Button {
public:
  // Button state
  enum ButtonState { NORMAL, PUSH };

protected:
  Window parent;
  Window frame;
  Rect rc;
  ButtonState state;

  QvImage* imgBack;
  QvImage* imgActiveBack;

  Tooltip* toolTip;

public:
  static XContext context;

public:
  Button(Window pWin, const Rect& rect);
  virtual ~Button();
  void SetState(ButtonState bs) { state = bs; }
  Bool CheckState(ButtonState bs) const { return state & bs; }
  Rect GetRect() const { return rc; }
  void SetRect(const Rect& rect) { rc = rect; }
  Window GetFrameWin() const { return frame; }
  Window GetParent() const { return parent; }

  virtual void MapButton();
  virtual void UnmapButton();
  virtual void DrawButton();
  virtual void MoveResizeButton(const Rect& rect);
  virtual void SetBgImage(QvImage* img, const Point& off);
  virtual void SetBgActiveImage(QvImage* img, const Point& off);
  virtual void ExecButtonFunc(ButtonState state) = 0;
  virtual void Button1Press();
  virtual void Button3Release(const Point& ptRoot);
  virtual void Enter();
  virtual void Leave();
  virtual void PointerMotion();

  static void Initialize();
};

#endif // _BUTTON_H_
