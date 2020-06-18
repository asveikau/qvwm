/*
 * taskbar.cc
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "taskbar.h"
#include "tbutton.h"
#include "sbutton.h"
#include "startmenu.h"
#include "event.h"
#include "icon.h"
#include "paging.h"
#include "pager.h"
#include "qvwmrc.h"
#include "indicator.h"
#include "desktop.h"
#include "tooltip.h"
#include "callback.h"
#include "timer.h"

int Taskbar::BASE_HEIGHT;
int Taskbar::INC_HEIGHT;

Taskbar::Taskbar(Qvwm* qvWm, int width, unsigned int rows)
: hiding(False)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  Dim dRoot(DisplayWidth(display, screen), DisplayHeight(display, screen));
  Dim base;
  char str[128];
  int len;
  XRectangle ink, log;

  if (UseClock) {
    /*
     * Calculate clock width.
     */
    time_t t;
    struct tm* tm;
    
    time(&t);
    tm = localtime(&t);
    
    // set values with max font width
    tm->tm_sec = 59;
    tm->tm_min = 59;
    tm->tm_hour = 23;
    tm->tm_mday = 31;
    tm->tm_yday = 365;
    
    // if ClockLocaleName is not specified in the configuration files
    if (ClockLocaleName == NULL)
      ClockLocaleName = LocaleName;

    setlocale(LC_TIME, ClockLocaleName);

    strftime(str, sizeof(str), ClockFormat, tm);

    len = strlen(str);
    XmbTextExtents(fsTaskbarClock, str, len, &ink, &log);
    clockWidth = log.width + IC_MARGIN*2;

    timer->SetTimeout(1000,
		      new Callback<Taskbar>(this, &Taskbar::AdvanceClock));
  }
  else
    clockWidth = 0;
  indWidth = 0;

  dTbox.width = clockWidth + TBOX_MARGIN * 2;
  dTbox.height = TaskbarButton::BUTTON_HEIGHT;

  base.width = (width > clockWidth + 6) ? width : clockWidth + 6;
  base.height = (rows - 1) * INC_HEIGHT + BASE_HEIGHT;
  if (base.height > dRoot.height / 2)
    base.height = (dRoot.height / 2 - BASE_HEIGHT) / INC_HEIGHT * INC_HEIGHT
      + BASE_HEIGHT;
 
  rc[BOTTOM] = Rect(0, dRoot.height - base.height, dRoot.width, base.height);
  rc[TOP] = Rect(0, 0, dRoot.width, base.height);
  rc[LEFT] = Rect(0, 0, base.width, dRoot.height);
  rc[RIGHT] = Rect(dRoot.width - base.width, 0, base.width, dRoot.height);

  /*
   * Create taskbar.
   */
  attributes.background_pixel = TaskbarColor.pixel;
  attributes.override_redirect = True;
  attributes.event_mask = ExposureMask | ButtonPressMask |
                          ButtonReleaseMask | Button1MotionMask |
			  EnterWindowMask | LeaveWindowMask;
  valueMask = CWBackPixel | CWOverrideRedirect | CWEventMask;

  frame = XCreateWindow(display, root,
			-10, -10, 10, 10,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  attributes.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                          Button1MotionMask;
  attributes.background_pixel = TaskbarColor.pixel;
  attributes.cursor = cursor[SYS];
  valueMask = CWBackPixel | CWCursor | CWEventMask;

  w = XCreateWindow(display, frame,
		    0, 0, 10, 10,
		    0, CopyFromParent, InputOutput, CopyFromParent,
		    valueMask, &attributes);

  /*
   * Create taskbar box.
   */
  attributes.event_mask = ExposureMask | EnterWindowMask | LeaveWindowMask |
                          PointerMotionMask;
  valueMask = CWBackPixel | CWEventMask;

  tbox = XCreateWindow(display, w,
		       0, 0, dTbox.width, dTbox.height,
		       0, CopyFromParent, InputOutput, CopyFromParent,
		       valueMask, &attributes);

  XMapWindow(display, w);
  XMapWindow(display, tbox);

  if (TaskbarImage) {
    imgTaskbar = CreateImageFromFile(TaskbarImage, timer);
    if (imgTaskbar) {
      imgTaskbar->SetBackground(frame);
      imgTWin = NULL;
      imgTBox = NULL;
    }
    else {
      delete [] TaskbarImage;
      TaskbarImage = NULL;
    }
  }

  ctrlMenu = new Menu(TaskbarMenuItem, fsCtrlMenu, NULL, qvWm);

  toolTip = new Tooltip();
}

Taskbar::~Taskbar()
{
  XDestroyWindow(display, frame);

  if (TaskbarImage) {
    QvImage::Destroy(imgTaskbar);
    QvImage::Destroy(imgTWin);
    QvImage::Destroy(imgTBox);
  }

  delete toolTip;
  delete ctrlMenu;
}

/*
 * MapTaskbar --
 *   Map the taskbar.
 */
void Taskbar::MapTaskbar()
{
  XMapWindow(display, frame);

  RaiseTaskbar();

  ASSERT(rootQvwm);
  ASSERT(rootQvwm->tButton);

  rootQvwm->tButton->MapButton();         // Start menu

  if (TaskbarAutoHide) {
    BasicCallback* cb;
    cb = new Callback<Taskbar>(this, &Taskbar::HideTaskbar);
    timer->SetTimeout(TaskbarHideDelay, cb);
  }
    
  if (!hiding)
    paging->RaisePagingBelt();

  // update rcScreen sizes 
  if (TaskbarAutoHide)
    rcScreen = GetScreenRectOnHiding();
  else
    rcScreen = GetScreenRectOnShowing();
}

/*
 * UnmapTaskbar -- Unmap the taskbar.  This is called when the taskbar
 * is disabled with QWVM_DISABLE_TASKBAR
 */
void Taskbar::UnmapTaskbar()
{
  rcScreen = rootQvwm->GetRect();
  XUnmapWindow(display, frame);
}


void Taskbar::EnableTaskbar()
{
  MapTaskbar(); 
  desktop.RecalcAllWindows();
  desktop.RedrawAllIcons();
  // YYY - possibly should also update pager stuff, as below in MoveTaskbar()
}

void Taskbar::DisableTaskbar()
{
  UnmapTaskbar();
  desktop.RecalcAllWindows();
  desktop.RedrawAllIcons();
  // YYY - possibly should also update pager stuff, as below in MoveTaskbar()
}


/*
 * MoveTaskbar --
 *   Move the taskbar to the tp position.
 */
void Taskbar::MoveTaskbar(TaskbarPos tp)
{
  pos = tp;

  switch (pos) {
  case BOTTOM:
    XDefineCursor(display, frame, cursor[Y_RESIZE]);
    XMoveResizeWindow(display, w, 0, 4, rc[BOTTOM].width, rc[BOTTOM].height-4);

    TaskbarButton::rcTButton.x = 2;
    TaskbarButton::rcTButton.y = 0;

    if (TaskbarImage) {
      QvImage::Destroy(imgTWin);
      imgTWin = imgTaskbar->GetOffsetImage(Point(0, 4));
      imgTWin->SetBackground(w);
    }
    break;

  case TOP:
    XDefineCursor(display, frame, cursor[Y_RESIZE]);
    XMoveResizeWindow(display, w, 0, 0, rc[TOP].width, rc[TOP].height-4);

    TaskbarButton::rcTButton.x = 2;
    TaskbarButton::rcTButton.y = 2;

    if (TaskbarImage) {
      QvImage::Destroy(imgTWin);
      imgTWin = imgTaskbar->GetOffsetImage(Point(0, 0));
      imgTWin->SetBackground(w);
    }
    break;

  case LEFT:
    XDefineCursor(display, frame, cursor[X_RESIZE]);
    XMoveResizeWindow(display, w, 0, 0, rc[LEFT].width-4, rc[LEFT].height);

    TaskbarButton::rcTButton.x = 2;
    TaskbarButton::rcTButton.y = 2;

    if (TaskbarImage) {
      QvImage::Destroy(imgTWin);
      imgTWin = imgTaskbar->GetOffsetImage(Point(0, 0));
      imgTWin->SetBackground(w);
    }
    break;

  case RIGHT:
    XDefineCursor(display, frame, cursor[X_RESIZE]);
    XMoveResizeWindow(display, w, 4, 0, rc[RIGHT].width-4, rc[RIGHT].height);

    TaskbarButton::rcTButton.x = 0;
    TaskbarButton::rcTButton.y = 2;

    if (TaskbarImage) {
      QvImage::Destroy(imgTWin);
      imgTWin = imgTaskbar->GetOffsetImage(Point(4, 0));
      imgTWin->SetBackground(w);
    }
    break;
  }

  // relocate the contents of taskbar box
  MoveResizeTaskbarBox();
  DrawTaskbarBox();

  RedrawAllTaskbarButtons();

  XMoveResizeWindow(display, frame, rc[pos].x, rc[pos].y, rc[pos].width,
		    rc[pos].height);

  desktop.RecalcAllWindows();
  desktop.RedrawAllIcons();
  if (UsePager) {
    ASSERT(pager);
    pager->RecalcPager();
  }

  ASSERT(paging);

  if (!hiding)
    paging->RaisePagingBelt();
}

/*
 * MoveTaskbar --
 *   Drag the taskbar to move.
 */
void Taskbar::MoveTaskbar(const Point& ptRoot)
{
  XEvent ev;
  Point ptNew;
  Rect rcRoot = rootQvwm->GetRect();
  Bool pointer = False;

  if (DisableDesktopChange || DisableTaskbarDragging)
    return;

  /*
   *  Limits to taskbar movements
   */
  switch (pos) {
  case BOTTOM:
    if (ptRoot.y == rcRoot.height-1 || ptRoot.y == rcRoot.height-2)
      return;
    break;

  case TOP:
    if (ptRoot.y == 0 || ptRoot.y == 1)
      return;
    break;

  case LEFT:
    if (ptRoot.x == 0 || ptRoot.x == 1)
      return;
    break;

  case RIGHT:
    if (ptRoot.x == rcRoot.width-1 || ptRoot.x == rcRoot.width-2)
      return;
    break;
  }
  
  if (!OpaqueMove) {
    XGrabServer(display);
    XDrawRectangle(display, root, gcXor, rc[pos].x, rc[pos].y,
		   rc[pos].width, rc[pos].height);
  }

  while (1) {
    XMaskEvent(display,
	       Button1MotionMask | ButtonReleaseMask | ExposureMask |
	       ButtonPressMask | PointerMotionMask,
	       &ev);
    switch (ev.type) {
    case MotionNotify:
      if (!OpaqueMove)
	XDrawRectangle(display, root, gcXor, rc[pos].x, rc[pos].y,
		       rc[pos].width, rc[pos].height);

      ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
      pos = desktop.GetDesktopArea(ptNew);

      if (OpaqueMove)
	MoveTaskbar(pos);
      else
	XDrawRectangle(display, root, gcXor, rc[pos].x, rc[pos].y,
		       rc[pos].width, rc[pos].height);
      break;

    case ButtonRelease:
      if (LockDragState) {
	pointer = True;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, None, CurrentTime);
	break;
      }
      else
	goto decide;
      
    case ButtonPress:
      if (pointer) {
	XUngrabPointer(display, CurrentTime);
	goto decide;
      }
      break;

    case Expose:
      event.ExposeProc((const XExposeEvent &)ev);
      break;
    }
  }

decide:
  if (!OpaqueMove) {
    XDrawRectangle(display, root, gcXor, rc[pos].x, rc[pos].y,
		   rc[pos].width, rc[pos].height);
    XUngrabServer(display);
  }

  ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
  pos = desktop.GetDesktopArea(ptNew);
  MoveTaskbar(pos);
}  

/*
 * ResizeTaskbar --
 *   Resize the taskbar.
 */
void Taskbar::ResizeTaskbar(const Point& ptRoot)
{
  RectPt resize;
  XEvent ev;
  Rect rcNew = rc[pos], rcOld = rc[pos];
  Point ptNew;
  Rect rcRoot = rootQvwm->GetRect();
  Bool pointer = False;
  int ctype;

  if (DisableDesktopChange)
    return;

  if (!OpaqueResize) {
    XGrabServer(display);
    XDrawRectangle(display, root, gcXor,
		   rcNew.x, rcNew.y, rcNew.width, rcNew.height);
  }

  while (1) {
    XMaskEvent(display,
	       Button1MotionMask | ButtonReleaseMask | ExposureMask |
	       ButtonPressMask | PointerMotionMask,
	       &ev);
    switch (ev.type) {
    case MotionNotify:
      if (!OpaqueResize)
	XDrawRectangle(display, root, gcXor,
		       rcNew.x, rcNew.y, rcNew.width, rcNew.height);

      ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
      resize = RectPt(0, 0, 0, 0);

      switch (pos) {
      case BOTTOM:
	resize.top = ptNew.y - ptRoot.y;
	break;

      case TOP:
	resize.bottom = ptNew.y - ptRoot.y;
	break;

      case LEFT:
	resize.right = ptNew.x - ptRoot.x;
	break;

      case RIGHT:
	resize.left = ptNew.x - ptRoot.x;
	break;
      }
      
      rcNew.x = rcOld.x + resize.left;
      rcNew.y = rcOld.y + resize.top;
      rcNew.width = rcOld.width - resize.left + resize.right;
      rcNew.height = rcOld.height - resize.top + resize.bottom;
      
      /*
       * Adjust.
       */
      switch (pos) {
      case BOTTOM:
      case TOP:
	if (rcNew.height > rcRoot.height/2)
	  rcNew.height = rcRoot.height / 2;
	rcNew.height = rcNew.height / INC_HEIGHT * INC_HEIGHT + 3;
	if (rcNew.height < 6)
	  rcNew.height = 6;
	break;

      case LEFT:
      case RIGHT:
	if (rcNew.width > rcRoot.width/2)
	  rcNew.width = rcRoot.width / 2;
	if (rcNew.width < 6)
	  rcNew.width = 6;
	break;
      }
      
      if (pos == BOTTOM)
	rcNew.y = rcRoot.height - rcNew.height;
      if (pos == RIGHT)
	rcNew.x = rcRoot.width - rcNew.width;
      
      if (OpaqueResize) {
	rc[pos] = rcNew;
	MoveTaskbar(pos);
      }
      else 
	XDrawRectangle(display, root, gcXor,
		       rcNew.x, rcNew.y, rcNew.width, rcNew.height);
      break;
      
    case ButtonRelease:
      if (LockDragState) {
	pointer = True;
	if (pos == BOTTOM || pos == TOP)
	  ctype = Y_RESIZE;
	else
	  ctype = X_RESIZE;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, cursor[ctype],
		     CurrentTime);
	break;
      }
      else
	goto decide;

    case ButtonPress:
      if (pointer) {
	XUngrabPointer(display, CurrentTime);
	goto decide;
      }
      break;

    case Expose:
      event.ExposeProc((const XExposeEvent &)ev);
      break;
    }
  }

decide:
  if (!OpaqueResize) {
    XDrawRectangle(display, root, gcXor,
		   rcNew.x, rcNew.y, rcNew.width, rcNew.height);
    XUngrabServer(display);
  }

  // update rcScreen sizes 
  if (TaskbarAutoHide)
    rcScreen = GetScreenRectOnHiding();
  else
    rcScreen = GetScreenRectOnShowing();

  rc[pos] = rcNew;
  MoveTaskbar(pos);
}  

/*
 * RaiseTaskbar --
 *   Raise the taskbar.
 */
void Taskbar::RaiseTaskbar()
{
  Window win[2];
  int nwindows = 0;
  Window menuFrame = Menu::GetMappedMenuFrame();

  if (OnTopTaskbar) {
    if (menuFrame != None)
      win[nwindows++] = menuFrame;
  }
  else
    win[nwindows++] = desktop.GetTopWindow();
  win[nwindows++] = frame;
  
  ASSERT(paging);

  if (hiding) {
    paging->RaisePagingBelt();
    if (OnTopTaskbar)
      XRaiseWindow(display, frame);
    XRestackWindows(display, win, nwindows);
  }
  else {
    if (OnTopTaskbar)
      XRaiseWindow(display, frame);
    XRestackWindows(display, win, nwindows);
    paging->RaisePagingBelt();
  }
}

/*
 * MoveResizeTaskbarBox --
 *    Move and resize taskbar box.
 */
void Taskbar::MoveResizeTaskbarBox()
{
  Rect rcRoot = rootQvwm->GetRect();
  StartButton *sButton = (StartButton *)rootQvwm->tButton;
  Point pt;

  indWidth = Indicator::RedrawAllIndicators();
  dTbox.width = indWidth + IC_MARGIN + clockWidth + TBOX_MARGIN;

  switch (pos) {
  case BOTTOM:
    pt = Point(rc[BOTTOM].width - dTbox.width - 2, 0);
    XMoveResizeWindow(display, tbox, pt.x, pt.y,
		      dTbox.width, TaskbarButton::BUTTON_HEIGHT);
    buttonArea = rcRoot.width - sButton->buttonWidth - dTbox.width - BETWEEN_SPACE*2;

    if (TaskbarImage) {
      QvImage::Destroy(imgTBox);
      imgTBox = imgTaskbar->GetOffsetImage(Point(pt.x, pt.y + 4));
      imgTBox->SetBackground(tbox);
    }
    break;

  case TOP:
    pt = Point(rc[TOP].width - dTbox.width - 2, 2);
    XMoveResizeWindow(display, tbox, pt.x, pt.y,
		      dTbox.width, TaskbarButton::BUTTON_HEIGHT);
    buttonArea = rcRoot.width - (sButton->buttonWidth + dTbox.width + BETWEEN_SPACE*2);

    if (TaskbarImage) {
      QvImage::Destroy(imgTBox);
      imgTBox = imgTaskbar->GetOffsetImage(pt);
      imgTBox->SetBackground(tbox);
    }
    break;

  case LEFT:
    if (rc[LEFT].width < sButton->buttonWidth + dTbox.width + 19) {
      if (rc[LEFT].width - 9 > dTbox.width) {
	pt = Point((rc[LEFT].width - dTbox.width - 4) / 2 + 1,
		   rc[LEFT].height - dTbox.height - 6);
	XMoveResizeWindow(display, tbox,
			  pt.x, pt.y, dTbox.width, dTbox.height);
      }
      else {
	dTbox.width = rc[LEFT].width - 9;
	if (dTbox.width <= 0)
	  dTbox.width = 1;
	pt = Point(3, rc[LEFT].height - dTbox.height - 6);
	XMoveResizeWindow(display, tbox,
			  pt.x, pt.y, dTbox.width, dTbox.height);
      }
    }
    else {
      pt = Point(rc[LEFT].width - dTbox.width - 8, TaskbarButton::rcTButton.y);
      XMoveResizeWindow(display, tbox,
			pt.x, pt.y, dTbox.width, TaskbarButton::BUTTON_HEIGHT);
    }

    buttonArea = rcRoot.height - (TaskbarButton::rcTButton.y +
				  TaskbarButton::BUTTON_HEIGHT + 12);
    if (rc[LEFT].width < sButton->buttonWidth + dTbox.width + 19)
      buttonArea -= dTbox.height + 9;

    if (TaskbarImage) {
      QvImage::Destroy(imgTBox);
      imgTBox = imgTaskbar->GetOffsetImage(pt);
      imgTBox->SetBackground(tbox);
    }
    break;
    
  case RIGHT:
    if (rc[RIGHT].width < sButton->buttonWidth + dTbox.width + 19) {
      if (rc[RIGHT].width - 9 > dTbox.width) {
	pt = Point((rc[RIGHT].width - dTbox.width - 4) / 2 - 1,
		   rc[RIGHT].height - dTbox.height - 6);
	XMoveResizeWindow(display, tbox,
			  pt.x, pt.y, dTbox.width, dTbox.height);
      }
      else {
	dTbox.width = rc[RIGHT].width - 9;
	if (dTbox.width <= 0)
	  dTbox.width = 1;
	pt = Point(4, rc[RIGHT].height - dTbox.height - 6);
	XMoveResizeWindow(display, tbox,
			  pt.x, pt.y, dTbox.width, dTbox.height);
      }
    }
    else {
      pt = Point(rc[RIGHT].width - dTbox.width - 8,
		 TaskbarButton::rcTButton.y);
      XMoveResizeWindow(display, tbox,
			pt.x, pt.y, dTbox.width, TaskbarButton::BUTTON_HEIGHT);
    }

    buttonArea = rcRoot.height - (TaskbarButton::rcTButton.y +
				  TaskbarButton::BUTTON_HEIGHT + 12);
    if (rc[RIGHT].width < sButton->buttonWidth + dTbox.width + 19)
      buttonArea -= dTbox.height + 9;

    if (TaskbarImage) {
      QvImage::Destroy(imgTBox);
      imgTBox = imgTaskbar->GetOffsetImage(Point(pt.x + 4, pt.y));
      imgTBox->SetBackground(tbox);
    }
    break;
  }
}

/*
 * DrawTaskbarFrame --
 *   Draw the taskbar frame.
 */
void Taskbar::DrawTaskbarFrame()
{
  if (hiding)
    return;

  switch (pos) {
  case BOTTOM:
    XSetForeground(display, gc, gray.pixel);
    XDrawLine(display, frame, gc, rc[BOTTOM].x, 0, rc[BOTTOM].width-1, 0);
    XSetForeground(display, gc, white.pixel);
    XDrawLine(display, frame, gc, rc[BOTTOM].x, 1, rc[BOTTOM].width-1, 1);
    break;

  case TOP:
    XSetForeground(display, gc, darkGray.pixel);
    XDrawLine(display, frame, gc, rc[TOP].x, rc[TOP].height-2,
	      rc[TOP].width-1, rc[TOP].height-2);
    XSetForeground(display, gc, darkGrey.pixel);
    XDrawLine(display, frame, gc, rc[TOP].x, rc[TOP].height-1,
	      rc[TOP].width-1, rc[TOP].height-1);
    break;

  case LEFT:
    XSetForeground(display, gc, darkGray.pixel);
    XDrawLine(display, frame, gc, rc[LEFT].width-2, rc[LEFT].y,
	      rc[LEFT].width-2, rc[LEFT].height-1);
    XSetForeground(display, gc, darkGrey.pixel);
    XDrawLine(display, frame, gc, rc[LEFT].width-1, rc[LEFT].y,
	      rc[LEFT].width-1, rc[LEFT].height-1);
    break;

  case RIGHT:
    XSetForeground(display, gc, gray.pixel);
    XDrawLine(display, frame, gc, 0, rc[RIGHT].y, 0, rc[RIGHT].height-1);
    XSetForeground(display, gc, white.pixel);
    XDrawLine(display, frame, gc, 1, rc[RIGHT].y, 1, rc[RIGHT].height-1);
    break;
  }
}

void Taskbar::DrawTaskbar()
{
  if (hiding)
    return;

  StartButton *sButton = (StartButton *)rootQvwm->tButton;
  int x, y;
  XPoint xp[3];

  XClearWindow(display, w);

  switch (pos) {
  case BOTTOM:
  case TOP:
    // between start button and taskbar buttons
    // we need no shit (IKu)
    break;

  case LEFT:
  case RIGHT:
    // between start button and taskbar buttons
    x = TaskbarButton::rcTButton.x;
    y = TaskbarButton::BUTTON_HEIGHT + 2;

    XSetForeground(display, gc, darkGray.pixel);
    XDrawLine(display, w, gc, x, y + 1, x + rc[pos].width - 7, y + 1);
    XSetForeground(display, gc, white.pixel);
    XDrawLine(display, w, gc, x, y + 2, x + rc[pos].width - 7, y + 2);
    
    xp[0].x = rc[pos].width - 9;
    xp[0].y = y + 5;
    xp[1].x = x + 2;
    xp[1].y = y + 5;
    xp[2].x = x + 2;
    xp[2].y = y + 6;
    XSetForeground(display, gc, white.pixel);
    XDrawLines(display, w, gc, xp, 3, CoordModeOrigin);

    xp[0].x = x + 2;
    xp[0].y = y + 7;
    xp[1].x = x + rc[pos].width - 9;
    xp[1].y = y + 7;
    xp[2].x = x + rc[pos].width - 9;
    xp[2].y = y + 5;
    XSetForeground(display, gc, darkGray.pixel);
    XDrawLines(display, w, gc, xp, 3, CoordModeOrigin);

    if (rc[pos].width < sButton->buttonWidth + dTbox.width + 19) {
      // line next to taskbar box
      y = rc[pos].height - dTbox.height - 9;

      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, w, gc, x, y, x + rc[pos].width - 7, y);
      XSetForeground(display, gc, white.pixel); 
      XDrawLine(display, w, gc, x, y + 1, x + rc[pos].width - 7, y + 1);
    }
    break;
  }
}

/*
 * DrawTaskbarBox --
 *   Draw the indicators and the clock in the taskbar box.
 */
void Taskbar::DrawTaskbarBox()
{
  XPoint xp[3];

  DrawClock();

  xp[0].x = dTbox.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = dTbox.height - 2;

  XSetForeground(display, gc, darkGray.pixel);
  XDrawLines(display, tbox, gc, xp, 3, CoordModeOrigin);

  xp[0].x = dTbox.width - 1;
  xp[0].y = 0;
  xp[1].x = dTbox.width - 1;
  xp[1].y = dTbox.height - 1;
  xp[2].x = 0;
  xp[2].y = dTbox.height - 1;

  XSetForeground(display, gc, white.pixel);
  XDrawLines(display, tbox, gc, xp, 3, CoordModeOrigin);
}

/*
 * DrawClock --
 *   Draw the clock in the right or bottom side.
 */
void Taskbar::DrawClock()
{
  time_t t;
  struct tm *tm;
  int len;
  Point pt;
  XRectangle ink, log;

  XClearArea(display, tbox, 1, 1, dTbox.width - 2, dTbox.height - 2, False);

  time(&t);
  tm = localtime(&t);

  strftime(strClk, sizeof(strClk), ClockFormat, tm);
  len = strlen(strClk);

  XmbTextExtents(fsTaskbar, strClk, len, &ink, &log);
  pt.x = indWidth + IC_MARGIN + (clockWidth - log.width) / 2 - log.x;
  pt.y = (TaskbarButton::BUTTON_HEIGHT - log.height) / 2 - log.y;

  XSetForeground(display, gc, ClockStringColor.pixel);
  XmbDrawString(display, tbox, fsTaskbarClock, gc, pt.x, pt.y, strClk, len);
}

void Taskbar::AdvanceClock()
{
  time_t t;
  struct tm *tm;
  char str[128];

  time(&t);
  tm = localtime(&t);
  strftime(str, sizeof(str), ClockFormat, tm);
  
  if (strcmp(str, strClk) != 0)
    DrawClock();

  timer->SetTimeout(1000, new Callback<Taskbar>(this, &Taskbar::AdvanceClock));
}

/*
 * Exposure --
 *   Process expose event.
 */
void Taskbar::Exposure(Window win)
{
  if (win == frame)
    DrawTaskbarFrame();
  if (win == w)
    DrawTaskbar();
  else if (win == tbox)
    DrawTaskbarBox();
}

void Taskbar::Button1Press()
{
  Menu::UnmapAllMenus(False);

  if (!OnTopTaskbar) {
    RaiseTaskbar();

    if (UsePager && OnTopPager)
      pager->RaisePager();
  }
}

/*
 * Button1Motion --
 *   Process drag.
 */
void Taskbar::Button1Motion(Window win, const Point& ptRoot)
{
  if (win == w)
    MoveTaskbar(ptRoot);
  else if (win == frame)
    ResizeTaskbar(ptRoot);
}

void Taskbar::Button3Release(const Point& ptRoot)
{
  int dir;

  Menu::UnmapAllMenus(False);
  rootQvwm->SetFocus();

  Point pt = ctrlMenu->GetFixedMenuPos(ptRoot, dir);
  ctrlMenu->MapMenu(pt.x, pt.y, dir);
}

/*
 * IsTaskbarWindows --
 *   Return True if the window is taskbar window.
 */
Bool Taskbar::IsTaskbarWindows(Window win)
{
  if (win == w || win == frame || win == tbox)
    return True;

  return False;
}

void Taskbar::Enter(Window win, const Point& ptRoot, int detail)
{
  if (win == frame) {
    if (!TaskbarAutoHide)
      return;

    if (detail != NotifyInferior)
      if (!Menu::CheckAnyMenusMapped())
	if (hiding) {
	  BasicCallback* cb;
	  cb= new Callback<Taskbar>(this, &Taskbar::ShowTaskbar);
	  timer->SetTimeout(TaskbarShowDelay, cb);
	}
  }
  else if (win == tbox) {
    time_t t;
    struct tm *tm;
    
    time(&t);
    tm = localtime(&t);

    strftime(strTip, sizeof(strTip), ClockMessageFormat, tm);
    toolTip->SetString(strTip, &fsTaskbar);

    toolTip->SetTimer();
  }
}

void Taskbar::Leave(Window win, const Point& ptRoot, int detail)
{
  if (win == frame) {
    if (!TaskbarAutoHide)
      return;

    if (detail != NotifyInferior)
      if (!Menu::CheckAnyMenusMapped())
	if (!hiding)
	  if (InRect(ptRoot, GetScreenRectOnShowing())) {
	    BasicCallback* cb;
	    cb = new Callback<Taskbar>(this, &Taskbar::HideTaskbar);
	    timer->SetTimeout(TaskbarHideDelay, cb);
	  }
  }
  else if (win == tbox)
    toolTip->Disable();
}

void Taskbar::PointerMotion()
{
  if (!toolTip->IsMapped())
    toolTip->ResetTimer();
}

/*
 * Get the rectangle of screen except taskbar on taskbar showing.
 */
Rect Taskbar::GetScreenRectOnShowing() const
{
  Rect rcRoot = rootQvwm->GetRect();

  switch (pos) {
  case TOP:
    return Rect(0, rc[TOP].height, rcRoot.width,
		rcRoot.height - rc[TOP].height);

  case BOTTOM:
    return Rect(0, 0, rcRoot.width, rcRoot.height - rc[BOTTOM].height);

  case LEFT:
    return Rect(rc[LEFT].width, 0,
		rcRoot.width - rc[LEFT].width, rcRoot.height);

  case RIGHT:
    return Rect(0, 0, rcRoot.width - rc[RIGHT].width, rcRoot.height);
  }

  ASSERT(False);
  return Rect(0, 0, 0, 0);
}

/*
 * Get the rectangle of screen except taskbar on taskbar hiding.
 */
Rect Taskbar::GetScreenRectOnHiding() const
{
  Rect rcRoot = rootQvwm->GetRect();

  switch (pos) {
  case TOP:
    return Rect(0, 2, rcRoot.width, rcRoot.height - 2);

  case BOTTOM:
    return Rect(0, 0, rcRoot.width, rcRoot.height - 2);

  case LEFT:
    return Rect(2, 0, rcRoot.width - 2, rcRoot.height);

  case RIGHT:
    return Rect(0, 0, rcRoot.width - 2, rcRoot.height);
  }

  ASSERT(False);
  return Rect(0, 0, 0, 0);
}

void Taskbar::ShowTaskbar()
{
  if (!hiding)
    return;

  hiding = False;

  Rect rcRoot = rootQvwm->GetRect();
  Point pt;

  if (GradTaskbarMotion) {
    switch (pos) {
    case TOP:
      pt = Point(0, 2 - rc[TOP].height);
      break;
      
    case BOTTOM:
      pt = Point(0, rcRoot.height - 2);
      break;
      
    case LEFT:
      pt = Point(2 - rc[LEFT].width, 0);
      break;
      
    case RIGHT:
      pt = Point(rcRoot.width - 2, 0);
      break;
    }
    
    for (int i = GradTaskbarMotionSpeed - 1; i >= 0; i--) {
      XMoveWindow(display, frame,
		  rc[pos].x + (pt.x - rc[pos].x) * i / GradTaskbarMotionSpeed,
		  rc[pos].y + (pt.y - rc[pos].y) * i / GradTaskbarMotionSpeed);

      XFlush(display);
      usleep(10000);
    }
  }
  else
    XMoveWindow(display, frame, rc[pos].x, rc[pos].y);

  XClearWindow(display, frame);

  RaiseTaskbar();

  if (UsePager && OnTopPager)
    pager->RaisePager();
}

void Taskbar::HideTaskbar()
{
  if (hiding)
    return;

  if (startMenu && startMenu->CheckMapped())
    return;

  hiding = True;

  Rect rcRoot = rootQvwm->GetRect();
  Point pt;

  switch (pos) {
  case TOP:
    pt = Point(0, 2 - rc[TOP].height);
    break;

  case BOTTOM:
    pt = Point(0, rcRoot.height - 2);
    break;

  case LEFT:
    pt = Point(2 - rc[LEFT].width, 0);
    break;

  case RIGHT:
    pt = Point(rcRoot.width - 2, 0);
    break;
  }

  if (GradTaskbarMotion) {
    for (int i = GradTaskbarMotionSpeed - 1; i >= 0; i--) {
      XMoveWindow(display, frame,
		  pt.x + (rc[pos].x - pt.x) * i / GradTaskbarMotionSpeed,
		  pt.y + (rc[pos].y - pt.y) * i / GradTaskbarMotionSpeed);

      XFlush(display);
      usleep(10000);
    }
  }
  else
    XMoveWindow(display, frame, pt.x, pt.y);

  XClearWindow(display, frame);

  if (OnTopTaskbar)
    RaiseTaskbar();
  if (UsePager && OnTopPager)
    pager->RaisePager();
}

/*
 * RedrawAllTaskbarButtons --
 *   Redraw all taskbar buttons.
 */
void Taskbar::RedrawAllTaskbarButtons()
{
  List<Qvwm>::Iterator i(&desktop.GetQvwmList());
  Qvwm* tmpQvwm;
  int buttonWidth;
  int row, col;
  int buttonNum = 0, count = 0;
  StartButton *sButton = (StartButton *)rootQvwm->tButton;
  int x, y;
  Point offFrame;
  Rect rcCurPage(paging->origin.x, paging->origin.y,
		 rcScreen.width, rcScreen.height);

  // get offset between frame and w
  switch (pos) {
  case BOTTOM:
    offFrame = Point(0, 4);
    break;
    
  case TOP:
  case LEFT:
    offFrame = Point(0, 0);
    break;

  case RIGHT:
    offFrame = Point(4, 0);
    break;
  }

  ASSERT(sButton);
  ASSERT(rootQvwm->tButton);

  XMoveWindow(display, sButton->GetFrameWin(),
	      TaskbarButton::rcTButton.x, TaskbarButton::rcTButton.y);

  /*
   * Taskbar buttons are unvisible when the taskbar is shown minimumly.
   */
  if (rc[pos].height == 6)
    TaskbarButton::rcTButton.y = -10000;
  if (rc[pos].width == 6)
    TaskbarButton::rcTButton.x = -10000;

  /*
   * Adjust the position and the size of the start button.
   */
  Rect rect = sButton->GetRect();

  rect.x = TaskbarButton::rcTButton.x;
  rect.y = TaskbarButton::rcTButton.y;

  if (rc[pos].width > sButton->buttonWidth + 6)
    rect.width = sButton->buttonWidth;
  else {
    rect.width = rc[pos].width - 6;
    if (rect.width == 0)
      rect.width = 1;
  }

  sButton->MoveResizeButton(rect);

  if (TaskbarImage) {
    sButton->SetBgImage(imgTaskbar, offFrame + Point(rect.x, rect.y));
    sButton->SetBgActiveImage(imgTaskbar,
			      offFrame + Point(rect.x, rect.y - 1));
    sButton->DrawButton();
  }

  // Count the number of taskbar buttons.
  for (tmpQvwm = i.GetHead(); tmpQvwm; tmpQvwm = i.GetNext()) {
    if (!tmpQvwm->CheckFlags(TRANSIENT) && !tmpQvwm->CheckFlags(NO_TBUTTON) &&
	(tmpQvwm->CheckMapped() || tmpQvwm->CheckStatus(MINIMIZE_WINDOW))) {
      if (TaskbarButtonInScr) {
	// count only taskbar buttons in the current page
	if (!Intersect(tmpQvwm->GetRect(), rcCurPage))
	  continue;
      }

      buttonNum++;
    }
  }
     
  if (buttonNum == 0)
    return;

  /*
   * According to taskbar position, determine the way arranging buttons.
   */
  switch (pos) {
  case BOTTOM:
  case TOP:
    row = rc[pos].height / (TaskbarButton::rcTButton.height + BETWEEN_SPACE);
    if (row == 0)
      row = 1;  // quick hack
    buttonWidth = buttonArea * row / RoundUp(buttonNum, row) - BETWEEN_SPACE;
    if (buttonWidth > TaskbarButton::rcTButton.width)
      buttonWidth = TaskbarButton::rcTButton.width;
    if (buttonWidth == 0)
      buttonWidth = 1;
    col = buttonArea / buttonWidth;
    break;

  case LEFT:
  case RIGHT:
    row = buttonArea / (TaskbarButton::rcTButton.height + BETWEEN_SPACE);
    col = (buttonNum + row - 1) / row;
    buttonWidth = (rc[pos].width - 5) / col - BETWEEN_SPACE;
    if (buttonWidth == 0)
      buttonWidth = 1;
    break;

  default:
    col = row = buttonWidth = 0;  // XXX for warning
    ASSERT(False);
  }

  /*
   * Reposition all taskbar buttons.
   */
  for (tmpQvwm = i.GetHead(); tmpQvwm; tmpQvwm = i.GetNext()) {
    if (tmpQvwm->CheckFlags(TRANSIENT) || tmpQvwm->CheckFlags(NO_TBUTTON) ||
	(!tmpQvwm->CheckMapped() && !tmpQvwm->CheckStatus(MINIMIZE_WINDOW)))
      continue;

    TaskbarButton* tb = tmpQvwm->tButton;

    if (TaskbarButtonInScr) {
      // show only taskbar buttons in the current page
      if (!Intersect(tmpQvwm->GetRect(), rcCurPage)) {
	tb->MoveResizeButton(Rect(-1, -1, 1, 1));  // hide
	continue;
      }
    }

    switch (pos) {
    case BOTTOM:
    case TOP:
      x = TaskbarButton::rcTButton.x + sButton->buttonWidth + BETWEEN_SPACE
	+ (buttonWidth + BETWEEN_SPACE) * (count % col);
      y = TaskbarButton::rcTButton.y
	+ (count / col) * (TaskbarButton::rcTButton.height + BETWEEN_SPACE);
      break;

    case LEFT:
    case RIGHT:
      x = TaskbarButton::rcTButton.x
	+ (count % col) * (buttonWidth + BETWEEN_SPACE);
      y = TaskbarButton::rcTButton.y + TaskbarButton::BUTTON_HEIGHT + 12
	+ (TaskbarButton::rcTButton.height + BETWEEN_SPACE) * (count / col);
      break;

    default:
      x = y = 0;  // XXX for warning
      ASSERT(False);
    }
    count++;
    
    tb->MoveResizeButton(Rect(x, y,
			      buttonWidth, TaskbarButton::rcTButton.height));

    if (TaskbarImage) {
      tb->SetBgImage(imgTaskbar, offFrame + Point(x, y));
      tb->SetBgActiveImage(imgTaskbar, offFrame + Point(x, y - 1));
      tb->DrawButton();
    }
  }
}                         

void Taskbar::Initialize()
{
  BASE_HEIGHT = TaskbarButton::BUTTON_HEIGHT + 6;
  INC_HEIGHT = TaskbarButton::BUTTON_HEIGHT + BETWEEN_SPACE;
}
