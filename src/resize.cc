/*
 * resize.cc
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
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "event.h"
#include "taskbar.h"
#include "tbutton.h"
#include "qvwmrc.h"
#include "paging.h"
#include "pager.h"
#include "mini.h"
#include "info_display.h"

#ifdef USE_SHAPE
static Bool ResizeWithShape(Display* display, XEvent* ev, char* arg);
#endif

/*
 * ResizeWindow --
 *   Resize the window with mouse or keyboard.
 */
void Qvwm::ResizeWindow(unsigned int pos, Bool mouseResize)
{
  XEvent ev;
  Rect rcNew, rcReal;
  Dim dNeed;
  Point ptBase, ptNew, ptMove;
  Window junkRoot;
  unsigned int junkBW, junkDepth;
  Bool first = True;
  Bool pointer = False;
  int borderWidth, topBorder, titleHeight, titleEdge;
  int ctype;

  if (this == rootQvwm)
    return;

  if (!CheckStatus(PRESS_FRAME))
    return;

  XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync,
		CurrentTime);
  if (ClickToFocus)
    XSetInputFocus(display, root, RevertToParent, CurrentTime);
  if (!OpaqueResize)
    XGrabServer(display);
  
  XGetGeometry(display, frame, &junkRoot, &rcNew.x, &rcNew.y,
	       (unsigned int *)&rcNew.width, (unsigned int *)&rcNew.height,
	       &junkBW, &junkDepth);

  if (!OpaqueResize)
    XDrawRectangle(display, root, gcXor,
		   rcNew.x, rcNew.y, rcNew.width, rcNew.height);

  /*
   * Remember base line.
   */
  if (pos & P_LEFT)
    ptBase.x = rcNew.x + rcNew.width;
  else if (pos & P_RIGHT)
    ptBase.x = rcNew.x;

  if (pos & P_TOP)
    ptBase.y = rcNew.y + rcNew.height;
  else if (pos & P_BOTTOM)
    ptBase.y = rcNew.y;

  /*
   * Calculate necessary frame size.
   */
  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);
  
  dNeed.width = borderWidth * 2;
  dNeed.height = borderWidth + topBorder + titleHeight + titleEdge;

  if (UseInfoDisplay) {
    Rect rect(rcNew.x + paging->origin.x, rcNew.y + paging->origin.y,
	      rcNew.width, rcNew.height);
    infoDisp->NotifyShape(rect);
    infoDisp->Map();
  }

  while (1) {
#ifdef USE_SHAPE
    if (OpaqueResize)
      XIfEvent(display, &ev, ResizeWithShape, NULL);
    else
#endif
    XMaskEvent(display, KeyPressMask | PointerMotionMask | ButtonReleaseMask |
	       ButtonPressMask | ExposureMask, &ev);
    switch (ev.type) {
    case KeyPress:
      // when resizing with mouse, ignore key events
      if (mouseResize)
	break;

      if (first) {
	pointer = True;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, None, CurrentTime);
      }
      ptNew = Point(ev.xkey.x_root, ev.xkey.y_root);

      switch (XKeycodeToKeysym(display, ev.xkey.keycode, 0)) {
      case XK_Up:
	if (first) {
	  pos = P_TOP;
	  ptBase.y = rcNew.y + rcNew.height;
	  ptNew.y = rcNew.y;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[Y_RESIZE]);
	  first = False;
	}
	if (pos == P_LEFT) {
	  pos |= P_TOP;
	  ptBase.y = rcNew.y + rcNew.height;
	  ptNew.y = rcNew.y;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[RD_RESIZE]);
	}
	else if (pos == P_RIGHT) {
	  pos |= P_TOP;
	  ptBase.y = rcNew.y + rcNew.height;
	  ptNew.y = rcNew.y;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[LD_RESIZE]);
	}
	ptNew.y -= MOVEMENT;
	break;

      case XK_Down:
	if (first) {
	  pos = P_BOTTOM;
	  ptBase.y = rcNew.y;
	  ptNew.y = rcNew.y + rcNew.height;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[Y_RESIZE]);
	  first = False;
	}
	if (pos == P_LEFT) {
	  pos |= P_BOTTOM;
	  ptBase.y = rcNew.y;
	  ptNew.y = rcNew.y + rcNew.height;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[LD_RESIZE]);
	}
	else if (pos == P_RIGHT) {
	  pos |= P_BOTTOM;
	  ptBase.y = rcNew.y;
	  ptNew.y = rcNew.y + rcNew.height;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[RD_RESIZE]);
	}
	ptNew.y += MOVEMENT;
	break;

      case XK_Right:
	if (first) {
	  pos = P_RIGHT;
	  ptBase.x = rcNew.x;
	  ptNew.x = rcNew.x + rcNew.width;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[X_RESIZE]);
	  first = False;
	}
	if (pos == P_TOP) {
	  pos |= P_RIGHT;
	  ptBase.x = rcNew.x;
	  ptNew.x = rcNew.x + rcNew.width;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[LD_RESIZE]);
	}
	else if (pos == P_BOTTOM) {
	  pos |= P_RIGHT;
	  ptBase.x = rcNew.x;
	  ptNew.x = rcNew.x + rcNew.width;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[RD_RESIZE]);
	}
	ptNew.x += MOVEMENT;
	break;

      case XK_Left:
	if (first) {
	  pos = P_LEFT;
	  ptBase.x = rcNew.x + rcNew.width;
	  ptNew.x = rcNew.x;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[X_RESIZE]);
	  first = False;
	}
	if (pos == P_TOP) {
	  pos |= P_LEFT;
	  ptBase.x = rcNew.x + rcNew.width;
	  ptNew.x = rcNew.x;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[RD_RESIZE]);
	}
	else if (pos == P_BOTTOM) {
	  pos |= P_LEFT;
	  ptBase.x = rcNew.x + rcNew.width;
	  ptNew.x = rcNew.x;
	  XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
	  XDefineCursor(display, root, cursor[LD_RESIZE]);
	}
	ptNew.x -= MOVEMENT;
	break;

      case XK_Return:
	goto decide;
      }
      XWarpPointer(display, None, root, 0, 0, 0, 0, ptNew.x, ptNew.y);
      // continue

    case MotionNotify:
      {
	Window w = ev.xmotion.window;
	XEvent ev;
	while (XCheckTypedWindowEvent(display, w, MotionNotify, &ev))
	  ;
      }

      if (!OpaqueResize)
	XDrawRectangle(display, root, gcXor, rcNew.x, rcNew.y,
		       rcNew.width, rcNew.height);

      if (ev.type == MotionNotify)
	ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);

      ASSERT(paging);

      ptMove = paging->HandlePaging(ptNew);
      ptNew -= ptMove;
      ptBase -= ptMove;
      rcNew.x -= ptMove.x;
      rcNew.y -= ptMove.y;
	
      FixWindowShape(rcNew, pos, ptBase, ptNew, dNeed);

      if (OpaqueResize) {
	rcReal = Rect(rcNew.x + paging->origin.x, rcNew.y + paging->origin.y,
		      rcNew.width, rcNew.height);
	ConfigureNewRect(rcReal, !FullOpaque);
	RedrawWindow(!FullOpaque);
      }
      else
	XDrawRectangle(display, root, gcXor, rcNew.x, rcNew.y,
		       rcNew.width, rcNew.height);

      if (UseInfoDisplay) {
	Rect rect(rcNew.x + paging->origin.x, rcNew.y + paging->origin.y,
		  rcNew.width, rcNew.height);
	infoDisp->NotifyShape(rect);
      }

      break;
      
    case ButtonRelease:
      if (LockDragState) {
	pointer = True;
	if (pos == P_LEFT || pos == P_RIGHT)
	  ctype = X_RESIZE;
	else if (pos == P_TOP || pos == P_BOTTOM)
	  ctype = Y_RESIZE;
	else if (pos == (P_TOP | P_LEFT) || pos == (P_BOTTOM | P_RIGHT))
	  ctype = RD_RESIZE;
	else
	  ctype = LD_RESIZE;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, cursor[ctype],
		     CurrentTime);
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

#ifdef USE_SHAPE
    default:
      if (shapeSupport) {
	if (ev.type == event.shapeEventBase + ShapeNotify) {
	  const XShapeEvent& xev = (const XShapeEvent &)ev;
	  event.ShapeNotifyProc(xev);
	}
      }
      break;
#endif
    }
  }

 decide:
  XDefineCursor(display, root, cursor[SYS]);

  if (!OpaqueResize) {
    XDrawRectangle(display, root, gcXor, rcNew.x, rcNew.y,
		   rcNew.width, rcNew.height);
    XUngrabServer(display);
  }

  if (UseInfoDisplay)
    infoDisp->Unmap();

  if (ClickToFocus)
    SetFocusToActiveWindow();
  if (pointer)
    XUngrabPointer(display, CurrentTime);
  XUngrabKeyboard(display, CurrentTime);
  
  if (!OpaqueResize) {
    ASSERT(paging);
    rcNew.x += paging->origin.x;
    rcNew.y += paging->origin.y;
    ConfigureNewRect(rcNew);
    RedrawWindow();
  }
  else if (!FullOpaque) {
    ConfigureNewRect(rc);
    RedrawWindow();
  }    
  
  if (UseTaskbar && OnTopTaskbar)
    taskBar->RaiseTaskbar();
  if (UsePager && OnTopPager)
    pager->RaisePager();

  ResetStatus(PRESS_FRAME);
}

#ifdef USE_SHAPE
static Bool ResizeWithShape(Display* display, XEvent* ev, char* arg)
{
  switch (ev->type) {
  case KeyPress:
  case ButtonPress:
  case ButtonRelease:
  case MotionNotify:
  case Expose:
    return True;
    
  default:
    if (shapeSupport)
      if (ev->type == event.shapeEventBase + ShapeNotify)
	return True;
    break;
  }
  
  return False;
}
#endif // USE_SHAPE

void Qvwm::FixWindowShape(Rect& rcNew, unsigned int pos, const Point& ptBase,
			const Point& ptNew, const Dim& dNeed)
{
  if (pos & P_LEFT) {
    rcNew.width = RoundOff((ptBase.x - ptNew.x) - dNeed.width -
			   hints.base_width, hints.width_inc) +
			     dNeed.width + hints.base_width;
    rcNew.x = ptBase.x - rcNew.width;
  }
  else if (pos & P_RIGHT)
    rcNew.width = RoundOff((ptNew.x - ptBase.x) - dNeed.width -
			   hints.base_width, hints.width_inc) +
			     dNeed.width + hints.base_width;

  if (pos & P_TOP) {
    rcNew.height = RoundOff((ptBase.y - ptNew.y) - dNeed.height -
			    hints.base_height, hints.height_inc) +
			      dNeed.height + hints.base_height;
    rcNew.y = ptBase.y - rcNew.height;
  }
  else if (pos & P_BOTTOM)
    rcNew.height = RoundOff((ptNew.y - ptBase.y) - dNeed.height -
			    hints.base_height, hints.height_inc) +
			      dNeed.height + hints.base_height;
  
  /*
   * Check minimum limit of size.
   */
  if ((signed)rcNew.width - dNeed.width <= 0) {
    rcNew.width = dNeed.width + Max(1, hints.base_width);
    if (pos & P_LEFT)
      rcNew.x = ptBase.x - rcNew.width;
  }
  else if ((signed)rcNew.width - dNeed.width < hints.min_width) {
    if (pos & P_LEFT)
      rcNew.x = ptBase.x - (hints.min_width + dNeed.width);
    rcNew.width = hints.min_width + dNeed.width;
  }
  if ((signed)rcNew.height - dNeed.height <= 0) {
    rcNew.height = dNeed.height + Max(1, hints.base_height);
    if (pos & P_TOP)
      rcNew.y = ptBase.y - rcNew.height;
  }
  else if ((signed)rcNew.height - dNeed.height < hints.min_height) {
    if (pos & P_TOP)
      rcNew.y = ptBase.y - (hints.min_height + dNeed.height);
    rcNew.height = hints.min_height + dNeed.height;
  }

  /*
   * Check maximum limit of size.
   */
  if (rcNew.width - dNeed.width > hints.max_width) {
    if (pos & P_LEFT)
      rcNew.x = ptBase.x - (hints.max_width + dNeed.width);
    rcNew.width = hints.max_width + dNeed.width;
  }
  if (rcNew.height - dNeed.height > hints.max_height) {
    if (pos & P_TOP)
      rcNew.y = ptBase.y - (hints.max_height + dNeed.height);
    rcNew.height = hints.max_height + dNeed.height;
  }

  /*
   * Check if over taskbar
   */
  if (UseTaskbar && NoResizeOverTaskbar) {
    switch (taskBar->GetPos()) {
    case Taskbar::BOTTOM:
      if ((pos & P_BOTTOM) &&
	  rcNew.y + rcNew.height > taskBar->GetRect().y)
	rcNew.height = taskBar->GetRect().y - rcNew.y;
      break;
    case Taskbar::TOP:
      if ((pos & P_TOP) && rcNew.y < rcScreen.y) {
	rcNew.height -= (rcScreen.y - rcNew.y);
	rcNew.y = rcScreen.y;
      }
      break;
    case Taskbar::LEFT:
      if ((pos & P_LEFT) && rcNew.x < rcScreen.x) {
	rcNew.width -= rcScreen.x - rcNew.x;
	rcNew.x = rcScreen.x;
      }
      break;
    case Taskbar::RIGHT:
      if ((pos & P_RIGHT) && rcNew.x + rcNew.width > taskBar->GetRect().x)
	rcNew.width = taskBar->GetRect().x - rcNew.x;
      break;
    }
    
    if (pos & P_LEFT) {
      rcNew.width = RoundDown(rcNew.width - dNeed.width -
			      hints.base_width, hints.width_inc) +
				dNeed.width + hints.base_width;
      rcNew.x = ptBase.x - rcNew.width;
    }
    else if (pos & P_RIGHT)
      rcNew.width = RoundDown(rcNew.width - dNeed.width -
			      hints.base_width, hints.width_inc) +
				dNeed.width + hints.base_width;
    
    if (pos & P_TOP) {
      rcNew.height = RoundDown(rcNew.height - dNeed.height -
			       hints.base_height, hints.height_inc) +
				 dNeed.height + hints.base_height;
      rcNew.y = ptBase.y - rcNew.height;
    }
    else if (pos & P_BOTTOM)
      rcNew.height = RoundDown(rcNew.height - dNeed.height -
			       hints.base_height, hints.height_inc) +
				 dNeed.height + hints.base_height;
  }
}
      
/*
 * GetFixSize --
 *   Adjust the position and the size of the window according to the window
 *   property.
 */
Rect Qvwm::GetFixSize(const Rect& rc, int max_width, int max_height,
		      int width_inc, int height_inc,
		      int base_width, int base_height)
{
  Rect rect;
  int borderWidth, topBorder, titleHeight, titleEdge;
  int outerw, outerh;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);
  
  outerw = borderWidth * 2;
  outerh = borderWidth + topBorder + titleHeight + titleEdge;

  // clip to window defined maximum
  rect.width = Min(max_width, rc.width - outerw) + outerw;
  rect.height = Min(max_height, rc.height - outerh) + outerh;

  // clip to screen
  rect.width = Min(rect.width, rcScreen.width);
  rect.height = Min(rect.height, rcScreen.height);

  rect.x = rc.x;
  rect.y = rc.y;
  rect.width = RoundDown(rect.width - base_width - outerw, width_inc)
    + base_width + outerw;
  rect.height = RoundDown(rect.height - base_height - outerh, height_inc)
    + base_height + outerh;

  return rect;
}
