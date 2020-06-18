/*
 * taskbar.h
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

#ifndef _TASKBAR_H_
#define _TASKBAR_H_

#include "tbutton.h"
#include "menu.h"

class TaskbarButton;
class Qvwm;
class Tooltip;
class QvImage;

/*
 * Taskbar class
 */
class Taskbar {
friend class TaskbarButton;
friend class Indicator;
public:
  // Taskbar position
  enum TaskbarPos { BOTTOM = 0, TOP, LEFT, RIGHT };

private:
  Window w;
  Window frame;
  Rect rc[4];			// 4 positions where taskbar can exist
  int buttonArea;		// area size for taskbar button
  TaskbarPos pos;		// taskbar position
  Menu* ctrlMenu;		// popup menu on right-click  
  Bool hiding;
  QvImage* imgTaskbar;
  QvImage* imgTWin;

  Window tbox;
  Dim dTbox;			// taskbar box dimension
  int indWidth;			// indicators width
  int clockWidth;		// clock width
  char strClk[128];
  QvImage* imgTBox;

  Tooltip* toolTip;
  char strTip[128];

  static const int TBOX_MARGIN = 10;
  static const int IC_MARGIN = 6;
  static const int BETWEEN_SPACE = 3;	// space between tbuttons

public:
  static int BASE_HEIGHT;	// base height when TOP or BOTTOM
  static int INC_HEIGHT;	// gain

public:
  Taskbar(Qvwm* qvWm, int width, unsigned int rows);
  ~Taskbar();
  Window GetFrameWin() const { return frame; }
  Rect GetRect() const { return rc[pos]; }
  TaskbarPos GetPos() const { return pos; }
  Bool IsTaskbarWindows(Window win);
  Bool CheckMenuMapped() const { return ctrlMenu->CheckMapped(); }
  void UnmapMenu() { ctrlMenu->UnmapMenu(); } 
  Menu* GetMenu() const { return ctrlMenu; }
  Bool IsHiding() const { return hiding; }
  QvImage* GetTaskbarImage() const { return imgTaskbar; }

  void MapTaskbar();
  void UnmapTaskbar();
  void EnableTaskbar();
  void DisableTaskbar();
  void MoveTaskbar(TaskbarPos tp);
  void MoveTaskbar(const Point& ptRoot);
  void ResizeTaskbar(const Point& ptRoot);
  void RaiseTaskbar();
  void MoveResizeTaskbarBox();
  void DrawTaskbarFrame();
  void DrawTaskbar();
  void DrawTaskbarBox();
  void DrawClock();
  void AdvanceClock();
  void ShowTaskbar();
  void HideTaskbar();
  Rect GetScreenRectOnShowing() const;
  Rect GetScreenRectOnHiding() const;

  void RedrawAllTaskbarButtons();

  void Exposure(Window win);
  void Button1Press();
  void Button1Motion(Window win, const Point& ptRoot);
  void Button3Release(const Point& ptRoot);
  void Enter(Window win, const Point& ptRoot, int detail);
  void Leave(Window win, const Point& ptRoot, int detail);
  void PointerMotion();

  static void Initialize();
};

#endif // _TASKBAR_H_
