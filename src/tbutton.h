/*
 * tbutton.h
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

#ifndef _TBUTTON_H_
#define _TBUTTON_H_

#include "button.h"

class QvImage;
class Qvwm;

/*
 * TaskbarButton
 */
class TaskbarButton : public Button {
private:
  QvImage* img;                    // image of the left side
  char* shortname;                 // short title fitted to button size
  Bool focus;
  Qvwm* qvWm;                      // ptr to corresponding qvwm

protected:
  char* name;                      // taskbar button title
  static const int TB_MARGIN = 4;

public:
  static Rect rcTButton;           // the size of taskbar button at that time
  static XFontSet* fsb;            // ref to bold font set
  static QvImage* imgTile;

  static const int SYMBOL_SIZE = 16;	// pixmap size
  static int BUTTON_HEIGHT;		// tbutton height

private:
  virtual void FillBackground();
  virtual void DrawName();

public:
  TaskbarButton(Qvwm* qvwm, const Rect& rc, QvImage* image);
  ~TaskbarButton();

  void ChangeName();
  void CalcShortName();
  Bool CheckFocus() const { return focus; }
  void SetFocus() { focus = True; }
  void ResetFocus() { focus = False; }
  QvImage* GetImage() const { return img; }
  void SetImage(QvImage* image);
  void MoveResizeButton(const Rect& rect);

  virtual void DrawButton();
  virtual void MapButton();
  virtual void UnmapButton();
  virtual void ExecButtonFunc(ButtonState bs);
  virtual void Button1Press();
  virtual void Button3Release(const Point& ptRoot);
  virtual void Enter();

  static void Initialize();
};

#endif // _TBUTTON_H_
