/*
 * move.cc
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "qvwmrc.h"
#include "paging.h"
#include "event.h"
#include "pager.h"
#include "mini.h"
#include "key.h"
#include "desktop.h"
#include "info_display.h"

/*
 * MoveWindow --
 *   Move the window with mouse or keyboard.
 */
void Qvwm::MoveWindow(const Point& ptRoot, Bool mouseMove)
{
  XEvent ev;
  Rect rcNew, rcShown;
  Point ptNew, ptOld, ptMove;
  Window junkRoot;
  unsigned int junkBW, junkDepth;
  Bool first = True;
  Bool pointer = False;
  int move;

  if (this == rootQvwm)
    return;

  if (!CheckStatus(PRESS_FRAME))
    return;

  ptOld = ptRoot;

  XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync,
		CurrentTime);
  if (ClickToFocus)
    XSetInputFocus(display, root, RevertToParent, CurrentTime);
  if (!OpaqueMove)
    XGrabServer(display);

  XGetGeometry(display, frame, &junkRoot, &rcNew.x, &rcNew.y,
	       (unsigned int *)&rcNew.width, (unsigned int *)&rcNew.height,
	       &junkBW, &junkDepth);
  rcShown = rcNew;
  
  if (!OpaqueMove) {
    XDrawRectangle(display, root, gcXor, rcNew.x, rcNew.y, rcNew.width,
		   rcNew.height);
    rcShown = rcNew;
  }

  if (UseInfoDisplay) {
    Point pt(rcNew.x + paging->origin.x, rcNew.y + paging->origin.y);
    infoDisp->NotifyPosition(pt);
    infoDisp->Map();
  }

  while (1) {
    XMaskEvent(display, KeyPressMask | PointerMotionMask | ButtonReleaseMask |
	       ButtonPressMask | ExposureMask, &ev);
    switch (ev.type) {
    case KeyPress:
      // when moving with mouse, ignore key events
      if (mouseMove)
	break;

      if (first) {
	ptNew = ptRoot;
	XDefineCursor(display, root, cursor[SYS]);
	first = False;
	pointer = True;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, None, CurrentTime);
      }

      move = MOVEMENT;
      if (ev.xkey.state & ShiftMask)
	move = (int)(move * ShiftMoveRatio);
      if (ev.xkey.state & ControlMask)
	move = (int)(move * CtrlMoveRatio);

      switch (XKeycodeToKeysym(display, ev.xkey.keycode, 0)) {
      case XK_Up:
	ptNew.y -= move;
	break;

      case XK_Down:
	ptNew.y += move;
	break;

      case XK_Right:
	ptNew.x += move;
	break;

      case XK_Left:
	ptNew.x -= move;
	break;

      case XK_Return:
	goto decide;
      }
      XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
      // to be continued to next case statement

    case MotionNotify:
      if (!(pointer || (ev.xbutton.state & Button1Mask) ||
	    (ev.xbutton.state & Button2Mask)))
	return;

      {
	Window w = ev.xmotion.window;
	XEvent ev;
	while (XCheckTypedWindowEvent(display, w, MotionNotify, &ev))
	  ;
      }

      if (first) {
	XDefineCursor(display, root, cursor[SYS]);
	first = False;
      }

      if (!OpaqueMove)
	XDrawRectangle(display, root, gcXor, rcShown.x, rcShown.y,
		       rcShown.width, rcShown.height);

      if (ev.type == MotionNotify)
	ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);

      ASSERT(paging);

      ptMove = paging->HandlePaging(ptNew);
      ptNew.x -= ptMove.x;
      ptNew.y -= ptMove.y;

      rcNew.x += ptNew.x - ptOld.x;
      rcNew.y += ptNew.y - ptOld.y;
      ptOld = ptNew;
      
      rcShown = rcNew;

      if (!(ev.xmotion.state & NoSnappingMask))
	FixWindowPos(rcShown);

      if (OpaqueMove) {
	XMoveWindow(display, frame, rcShown.x, rcShown.y);
	if (UsePager) {
	  Point pt(rcShown.x + paging->origin.x, rcShown.y + paging->origin.y);

	  ASSERT(mini);
	  ASSERT(pager);

	  mini->MoveMiniature(pager->ConvertToPagerPos(pt));
	}
      }
      else {
	XDrawRectangle(display, root, gcXor, rcShown.x, rcShown.y,
		       rcShown.width, rcShown.height);
      }

      if (UseInfoDisplay) {
	Point pt(rcShown.x + paging->origin.x, rcShown.y + paging->origin.y);
	infoDisp->NotifyPosition(pt);
      }

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
      if (pointer)
	goto decide;
      break;

    case Expose:
      event.ExposeProc((const XExposeEvent &)ev);
      if (UseInfoDisplay)
	infoDisp->Draw();
      break;
    }
  }

 decide:
  if (!OpaqueMove) {
    XDrawRectangle(display, root, gcXor, rcShown.x, rcShown.y,
		   rcShown.width, rcShown.height);
    XUngrabServer(display);
  }

  if (UseInfoDisplay)
    infoDisp->Unmap();

  if (ClickToFocus)
    SetFocusToActiveWindow();
  if (pointer)
    XUngrabPointer(display, CurrentTime);
  XUngrabKeyboard(display, CurrentTime);

  XMoveWindow(display, frame, rcShown.x, rcShown.y);
  rcShown.x += paging->origin.x;
  rcShown.y += paging->origin.y;
  ConfigureNewRect(rcShown);
  if (UsePager) {
    ASSERT(pager);
    Point ptMini(pager->ConvertToPagerPos(Point(rcShown.x, rcShown.y)));

    ASSERT(mini);
    mini->MoveMiniature(ptMini);
  }

  if (UseTaskbar && OnTopTaskbar)
    taskBar->RaiseTaskbar();
  if (UsePager && OnTopPager)
    pager->RaisePager();

  ResetStatus(PRESS_FRAME);
}

void Qvwm::FixWindowPos(Rect& rcShown)
{
  if (SnappingMove) {
    List<Qvwm>::Iterator i(&desktop.GetQvwmList());
    Rect rcSnap, rcTmp = rcShown;
    Bool xSnapped = False, ySnapped = False;
    
    for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
      if (qvWm->CheckStatus(MINIMIZE_WINDOW) || !qvWm->CheckMapped() ||
	  qvWm == this)
	continue;

      if (qvWm == rootQvwm) {
	if (!UseTaskbar)
	  continue;

	// don't snap against root window; snap against taskbar instead
	rcSnap = taskBar->GetRect();
      }
      else {
	rcSnap = qvWm->GetRect();
	rcSnap.x -= paging->origin.x;
	rcSnap.y -= paging->origin.y;
      }

      if (rcSnap.y + rcSnap.height >= rcTmp.y &&
	  rcSnap.y <= rcTmp.y + rcTmp.height) {
	if (abs((rcSnap.x + rcSnap.width) - rcTmp.x) <= SnappingMove) {
	  rcTmp.x = rcSnap.x + rcSnap.width;
	  if (abs((rcTmp.y + rcTmp.height) - (rcSnap.y + rcSnap.height))
	      <= SnappingMove)
	    rcTmp.y = rcSnap.y + rcSnap.height - rcTmp.height;
	  if (abs(rcTmp.y - rcSnap.y) <= SnappingMove)
	    rcTmp.y = rcSnap.y;
	}
	if (abs((rcTmp.x + rcTmp.width) - rcSnap.x) <= SnappingMove) {
	  rcTmp.x = rcSnap.x - rcTmp.width;
	  if (abs((rcTmp.y + rcTmp.height) - (rcSnap.y + rcSnap.height))
	      <= SnappingMove)
	    rcTmp.y = rcSnap.y + rcSnap.height - rcTmp.height;
	  if (abs(rcTmp.y - rcSnap.y) <= SnappingMove)
	    rcTmp.y = rcSnap.y;
	}
      }
      if (rcSnap.x + rcSnap.width >= rcTmp.x &&
	  rcSnap.x <= rcTmp.x + rcTmp.width) {
	if (abs((rcTmp.y + rcTmp.height) - rcSnap.y) <= SnappingMove) {
	  rcTmp.y = rcSnap.y - rcTmp.height;
	  if (abs((rcTmp.x + rcTmp.width) - (rcSnap.x + rcSnap.width))
	      <= SnappingMove)
	    rcTmp.x = rcSnap.x + rcSnap.width - rcTmp.width;
	  if (abs(rcTmp.x - rcSnap.x) <= SnappingMove)
	    rcTmp.x = rcSnap.x;
	}
	if (abs((rcSnap.y + rcSnap.height) - rcTmp.y) <= SnappingMove) {
	  rcTmp.y = rcSnap.y + rcSnap.height;
	  if (abs((rcTmp.x + rcTmp.width) - (rcSnap.x + rcSnap.width))
	      <= SnappingMove)
	    rcTmp.x = rcSnap.x + rcSnap.width - rcTmp.width;
	  if (abs(rcTmp.x - rcSnap.x) <= SnappingMove)
	    rcTmp.x = rcSnap.x;
	}
      }
      // done twice to correct edge snapping
      if (rcSnap.y + rcSnap.height >= rcTmp.y &&
	  rcSnap.y <= rcTmp.y + rcTmp.height) {
	if (abs((rcTmp.x + rcTmp.width) - rcSnap.x) <= SnappingMove)
	  rcTmp.x = rcSnap.x - rcTmp.width;
	if (abs((rcSnap.x + rcSnap.width) - rcTmp.x) <= SnappingMove)
	  rcTmp.x = rcSnap.x + rcSnap.width;
      }
    }
    if (rcTmp.x != rcShown.x && !xSnapped) {
      rcShown.x = rcTmp.x;
      xSnapped = True;
    }
    if (rcTmp.y != rcShown.y && !ySnapped) {
      rcShown.y = rcTmp.y;
    }
  }

  if (SnappingEdges) {
    if (abs(rcScreen.x + rcScreen.width - rcShown.x - rcShown.width)
	<= SnappingEdges)
      rcShown.x = rcScreen.x + rcScreen.width - rcShown.width;
    if (abs(rcScreen.x - rcShown.x) <= SnappingEdges)
      rcShown.x = rcScreen.x;
    if (abs(rcScreen.y + rcScreen.height - rcShown.y - rcShown.height)
	<= SnappingEdges)
      rcShown.y = rcScreen.y + rcScreen.height - rcShown.height;
    if (abs(rcScreen.y - rcShown.y) <= SnappingEdges)
      rcShown.y = rcScreen.y;
  }

  if (EdgeResistance) {
    if (rcShown.x < rcScreen.x && rcShown.x > rcScreen.x - EdgeResistance)
      rcShown.x = rcScreen.x;
    else if (rcShown.x + rcShown.width > rcScreen.x + rcScreen.width &&
	     rcShown.x + rcShown.width < rcScreen.x + rcScreen.width
	     + EdgeResistance)
      rcShown.x = rcScreen.x + rcScreen.width - rcShown.width;
    if (rcShown.y < rcScreen.y && rcShown.y > rcScreen.y - EdgeResistance)
      rcShown.y = rcScreen.y;
    else if (rcShown.y + rcShown.height > rcScreen.y + rcScreen.height &&
	     rcShown.y + rcShown.height < rcScreen.y + rcScreen.height
	     + EdgeResistance)
      rcShown.y = Max(rcScreen.y + rcScreen.height - rcShown.height,
		      rcScreen.y);
  }
}
