/*
 * dialog.h
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

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include "util.h"
#include "resource.h"

class Timer;
class QvImage;

/*
 * Dialog class
 */
class Dialog {
protected:
  Window frame;                    // frame
  Rect rc;                         // position and size
  char* name;                      // title name

public:
  static QvImage* imgDialog;
  static Timer* dlgTimer;

public:
  Dialog(const Rect& rect = Rect(0, 0, 1, 1));
  virtual ~Dialog();

  Window GetFrameWin() { return frame; }
  void SetRect(const Rect& rect);
  void SetTitle(char* dlgname);

  virtual void MapDialog();
  virtual void UnmapDialog();
  virtual void DrawClientWin();
  virtual ResourceId EventLoop();
  virtual void Exposure(Window win);
  virtual ResourceId Button1Press(Window win);
  virtual ResourceId FindShortCutKey(char key);

  static void Initialize();
};

#endif // _DIALOG_H_
