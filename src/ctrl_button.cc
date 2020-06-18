/*
 * ctrl_button.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "misc.h"
#include "util.h"
#include "event.h"
#include "qvwmrc.h"
#include "paging.h"
#include "function.h"

/*
 * CreateCtrlButton --
 *   Create control button window. This is the child of title window.
 */
void Qvwm::CreateCtrlButton(const Rect& rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                          EnterWindowMask | LeaveWindowMask |
			  Button1MotionMask | OwnerGrabButtonMask;
  attributes.background_pixel = darkGray.pixel;
  valueMask = CWBackPixel | CWEventMask;

  ctrl = XCreateWindow(display, title,
		       rect.x, rect.y, rect.width, rect.height,
		       0, CopyFromParent, InputOutput, CopyFromParent,
		       valueMask, &attributes);
  XSaveContext(display, ctrl, context, (caddr_t)this);
}

/*
 * DrawCtrlMenuMark --
 *   Draw the pixmap of control menu button.
 */
void Qvwm::DrawCtrlMenuMark()
{
  if (!CheckFlags(CTRL_MENU))
    return;

  if (CheckFocus()) {
    if(TitlebarImage)
      imgTitlebar->SetBackground(None);

    if (TitlebarActiveImage)
      imgActiveTitlebar->SetBackground(ctrl);
    else
      XSetWindowBackground(display, ctrl, TitlebarActiveColor.pixel);
  }
  else {
    if (TitlebarActiveImage)
      imgActiveTitlebar->SetBackground(None);

    if(TitlebarImage)
      imgTitlebar->SetBackground(ctrl);
    else
      XSetWindowBackground(display, ctrl, TitlebarColor.pixel);
  }
  
  XClearWindow(display, ctrl);

  imgSmall->Display(ctrl, Point(0, 0));
}

void Qvwm::CtrlButton1Press(Time clickTime, const Point& ptRoot,
			     unsigned int state)
{
  /*
   * Double click.
   */
  if (IsDoubleClick(menuClickTime, clickTime, event.ptPrevRoot, ptRoot)) {
    if (CheckMenuMapped())
      umReserved = True;
    
    if (state & ControlMask)
      QvFunction::execFunction(Q_KILL, this);
    else
      QvFunction::execFunction(Q_CLOSE, this);
  }
  else {
    if (CheckMenuMapped())
      umReserved = True;
    else {
      Menu::UnmapAllMenus();
      
      if (!CheckFocus())
	SetFocus();
      RaiseWindow(True);
      
      int borderWidth, topBorder, titleHeight, titleEdge;
      GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);
      
      ASSERT(ctrlMenu);
      ctrlMenu->SetQvwm(this);
      
      ASSERT(paging);
      Point pt(rc.x - paging->origin.x + topBorder,
	       rc.y - paging->origin.y + topBorder + titleHeight);
      int dir;
      
      pt = ctrlMenu->GetFixedMenuPos(pt, dir);
      if (dir & GD_UP) {
	Rect rcMenu = ctrlMenu->GetRect();
	pt.y = rc.y - paging->origin.y + topBorder - rcMenu.height;
      }
      
      ctrlMenu->MapMenu(pt.x, pt.y, dir);
    }
  }
  menuClickTime = clickTime;
}
