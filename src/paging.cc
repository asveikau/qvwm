/*
 * paging.cc
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
#include <unistd.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "qvwmrc.h"
#include "paging.h"
#include "pager.h"
#include "mini.h"
#include "event.h"
#include "taskbar.h"
#include "desktop.h"
#include "gnome.h"
#include "util.h"

Paging::Paging(const Point& topLeft, const Dim& size)
: origin(Point(0, 0)), rcVirt(topLeft.x, topLeft.y, size.width, size.height)
{
  if (size.width <= 0) {
    QvwmError("The width of the pager must be positive (%d is an error)",
	      size.width);
    rcVirt.width = 0;
  }
  if (size.height <= 0) {
    QvwmError("The height of the pager must be positive (%d is an error)",
	      size.height);
    rcVirt.width = 0;
  }
  if (topLeft.x > 0 || topLeft.x + size.width - 1 < 0) {
    QvwmError("The pager must include a page (0, 0)");
    rcVirt.x = 0;
  }
  if (topLeft.y > 0 || topLeft.y + size.height - 1 < 0) {
    QvwmError("The pager must include a page (0, 0)");
    rcVirt.y = 0;
  }

  XSetWindowAttributes attributes;
  unsigned long valueMask;
  Cursor cs;
  Rect rcRoot = rootQvwm->GetRect();
  unsigned int fontCursor[] = {
    XC_bottom_side, XC_top_side,
    XC_left_side, XC_right_side,
    XC_bottom_left_corner, XC_bottom_right_corner,
    XC_top_left_corner, XC_top_right_corner
  };
  Rect rcBelt[] = {
    Rect(PagingBeltSize, rcRoot.height - PagingBeltSize,
	 rcRoot.width - PagingBeltSize * 2, PagingBeltSize),
    Rect(PagingBeltSize, 0,
	 rcRoot.width - PagingBeltSize * 2, PagingBeltSize),
    Rect(0, PagingBeltSize,
	 PagingBeltSize, rcRoot.height - PagingBeltSize * 2),
    Rect(rcRoot.width - PagingBeltSize, PagingBeltSize,
	 PagingBeltSize, rcRoot.height - PagingBeltSize * 2),
    Rect(0, rcRoot.height - PagingBeltSize,
	 PagingBeltSize, PagingBeltSize),
    Rect(rcRoot.width - PagingBeltSize, rcRoot.height - PagingBeltSize,
	 PagingBeltSize, PagingBeltSize),
    Rect(0, 0,
	 PagingBeltSize, PagingBeltSize),
    Rect(rcRoot.width - PagingBeltSize, 0,
	 PagingBeltSize, PagingBeltSize)
  };

  /*
   * Create transparent, long and slender windows in the 4 edges of screen
   * for paging.
   */
  attributes.override_redirect = True;
  attributes.event_mask = EnterWindowMask | LeaveWindowMask |
    Button1MotionMask;

  valueMask = CWEventMask | CWOverrideRedirect;

  if (PagingBeltSize > 0) {
    for (int i = 0; i < 8; i++) {
      belt[i] = XCreateWindow(display, root,
			      rcBelt[i].x, rcBelt[i].y,
			      rcBelt[i].width, rcBelt[i].height,
			      0, CopyFromParent, InputOnly, CopyFromParent,
			      valueMask, &attributes);
      
      cs = XCreateFontCursor(display, fontCursor[i]);
      XDefineCursor(display, belt[i], cs);
    }

    MapBelts();
  }
  else {
    for (int i = 0; i < 8; i++)
      belt[i] = None;
  }
}

Paging::~Paging()
{
  if (PagingBeltSize == 0)
    return;

  for (int i = 0; i < 8; i++)
    XDestroyWindow(display, belt[i]);
}

/*
 * IsPagingBelt --
 *   Return True if the window is one of transparent windows for paging.
 */
Bool Paging::IsPagingBelt(Window win)
{
  if (PagingBeltSize == 0)
    return False;

  for (int i = 0; i < 8; i++) {
    if (win == belt[i])
      return True;
  }

  return False;
}

/*
 * HandlePaging --
 *   Do paging if the cursor is in screen edge and if paging resistance
 *   time was expired.
 *   Return True if did paging.
 */
Point Paging::HandlePaging(const Point& pt)
{      
  if (PagingBeltSize == 0 || Menu::CheckAnyMenusMapped())
    return Point(0, 0);

  Rect rcRoot = rootQvwm->GetRect();
  Rect rcNoBelt(PagingBeltSize,
		PagingBeltSize,
		rcRoot.width - PagingBeltSize * 2,
		rcRoot.height - PagingBeltSize * 2);

  /*
   * Return unless in the area where paging can be done.
   */
  if (InRect(pt, rcNoBelt))
    return Point(0, 0);

  BeltPos pos = GetBeltPos(pt);

  if (!mapped[pos])
    return Point(0, 0);

  /*
   * Agter wait for PagingResistance ticks without any events, do paging.
   */
  int ticks = 0;
  XEvent ev;
  Point ptNew;

  while (ticks < PagingResistance) {
    ticks += 10;
    usleep(10000);
    
    if (XCheckTypedEvent(display, MotionNotify, &ev)) {
      ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
      if (InRect(ptNew, rcNoBelt))
	return Point(0, 0);
    }
    else if (XCheckTypedWindowEvent(display, belt[pos], LeaveNotify, &ev))
      return Point(0, 0);
  }

  PlaySound(PagingSound);

  ptNew = PagingProc(belt[pos]);

  XSync(display, 0);
  while (XCheckTypedEvent(display, Expose, &ev))
    event.ExposeProc((const XExposeEvent &)ev);

  return ptNew;
}

/*
 * PagingProc --
 *   Do paging to the direction of win.
 */
Point Paging::PagingProc(Window win, Bool rootFocus)
{
  Point ptDelta(0, 0);
  Rect rcRoot = rootQvwm->GetRect();
  Window junkRoot, junkChild;
  Point ptRoot, ptJunk;
  unsigned int mask;

  if (win == belt[LEFT])
    ptDelta.x = -(PagingMovement * rcRoot.width / 100);
  else if (win == belt[RIGHT])
    ptDelta.x = PagingMovement * rcRoot.width / 100;
  else if (win == belt[TOP])
    ptDelta.y = -(PagingMovement * rcRoot.height / 100);
  else if (win == belt[BOTTOM])
    ptDelta.y = PagingMovement * rcRoot.height / 100;
  else if (win == belt[BOTTOM_LEFT]) {
    ptDelta.x = -(PagingMovement * rcRoot.width / 100);
    ptDelta.y = PagingMovement * rcRoot.height / 100;
  }
  else if (win == belt[BOTTOM_RIGHT]) {
    ptDelta.x = PagingMovement * rcRoot.width / 100;
    ptDelta.y = PagingMovement * rcRoot.height / 100;
  }
  else if (win == belt[TOP_LEFT]) {
    ptDelta.x = -(PagingMovement * rcRoot.width / 100);
    ptDelta.y = -(PagingMovement * rcRoot.height / 100);
  }
  else if (win == belt[TOP_RIGHT]) {
    ptDelta.x = PagingMovement * rcRoot.width / 100;
    ptDelta.y = -(PagingMovement * rcRoot.height / 100);
  }
  else
    return Point(0, 0);

  Point oldOrigin = origin;

  origin.x += ptDelta.x;
  origin.y += ptDelta.y;

  PagingAllWindows(oldOrigin, rootFocus);

  XQueryPointer(display, root, &junkRoot, &junkChild, &ptRoot.x, &ptRoot.y,
		&ptJunk.x, &ptJunk.y, &mask);
  Point pt(ptRoot.x - ptDelta.x, ptRoot.y - ptDelta.y);

  if (pt.x < PagingBeltSize)
    pt.x = PagingBeltSize;
  else if (pt.x > rcRoot.width-PagingBeltSize-1)
    pt.x = rcRoot.width - PagingBeltSize - 1;
  if (pt.y < PagingBeltSize)
    pt.y = PagingBeltSize;
  else if (pt.y > rcRoot.height-PagingBeltSize-1)
    pt.y = rcRoot.height - PagingBeltSize - 1;

  XWarpPointer(display, None, root, 0, 0, 0, 0, pt.x, pt.y);

  desktop.ChangeFocusToCursor();

  return ptDelta;
}

/*
 * PagingProc --
 *   Do paging to the position of pt.
 */
void Paging::PagingProc(const Point& pt, Bool rootFocus)
{
  Point oldOrigin = origin;

  origin = pt;
  PagingAllWindows(oldOrigin, rootFocus);
}

/*
 * PagingAllWindows --
 *   Move all windows according to paging movement.
 */
void Paging::PagingAllWindows(const Point& oldOrigin, Bool rootFocus)
{
  MapBelts();

  if (oldOrigin.x == origin.x && oldOrigin.y == origin.y)
    return;

  List<Qvwm>::Iterator i(&desktop.GetQvwmList());
  Point newOrigin = origin;

  for (int j = 0; j < PagingSpeed; j++) {
    origin.x = oldOrigin.x + (newOrigin.x - oldOrigin.x) * (j + 1)
      / PagingSpeed;
    origin.y = oldOrigin.y + (newOrigin.y - oldOrigin.y) * (j + 1)
      / PagingSpeed;

    for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
      Rect rect = qvWm->GetRect();
      
      if (qvWm->CheckFlags(STICKY)) {
	Point pt(rect.x - oldOrigin.x + origin.x,
		 rect.y - oldOrigin.y + origin.y);

	/*
	 * rc of STICKY window changes on paging.
	 * only last loop
	 */
	if (j == PagingSpeed - 1) {
	  rect.x = pt.x;
	  rect.y = pt.y;
	  qvWm->SetRect(rect);
	}

	if (UsePager) {
	  ASSERT(qvWm->mini);
	  qvWm->mini->MoveMiniature(pager->ConvertToPagerPos(pt));
	}
	continue;
      }
      
      XMoveWindow(display, qvWm->GetFrameWin(),
		  rect.x - origin.x, rect.y - origin.y);

      // only last loop
      if (j == PagingSpeed - 1) {
	qvWm->SendConfigureEvent();
	if (Intersect(rect, rootQvwm->GetRect()))
	  Gnome::ResetState(qvWm, WIN_STATE_HID_WORKSPACE);
	else
	  Gnome::SetState(qvWm, WIN_STATE_HID_WORKSPACE);
      }
    }

    XFlush(display);
    usleep(10000);
  }

  if (UsePager) {
    ASSERT(pager);
    pager->DrawVisualPage();
  }

  Gnome::SetActiveDesktop();

  if (!ClickToFocus && NoDesktopFocus && rootFocus) {
    Point ptJunk;
    Window w;

    // set focus root only if pointer is on root.
    XTranslateCoordinates(display, root, root, ptJunk.x, ptJunk.y,
			  &ptJunk.x, &ptJunk.y, &w);
    if (w == root)
      rootQvwm->SetFocus();
  }

  if (rootFocus && UseTaskbar && TaskbarAutoHide)
    taskBar->HideTaskbar();

  if (TaskbarButtonInScr) {
    if (taskBar)
      taskBar->RedrawAllTaskbarButtons();
  }
}

void Paging::MapBelts()
{
  Rect rcRoot = rootQvwm->GetRect();

  // check left-most edge
  if (origin.x > rcVirt.x * rcRoot.width) {
    if (PagingBeltSize > 0) {
      XMapWindow(display, belt[LEFT]);
      mapped[LEFT] = True;
    }
  }
  else {
    origin.x = rcVirt.x * rcRoot.width;
    if (PagingBeltSize > 0) {
      XUnmapWindow(display, belt[LEFT]);
      mapped[LEFT] = False;
    }
  }

  // check right-most edge
  if (origin.x < (rcVirt.x + rcVirt.width - 1) * rcRoot.width) {
    if (PagingBeltSize > 0) {
      XMapWindow(display, belt[RIGHT]);
      mapped[RIGHT] = True;
    }
  }
  else {
    origin.x = (rcVirt.x + rcVirt.width - 1) * rcRoot.width;
    if (PagingBeltSize > 0) {
      XUnmapWindow(display, belt[RIGHT]);
      mapped[RIGHT] = False;
    }
  }

  // check top-most edge
  if (origin.y > rcVirt.y * rcRoot.height) {
    if (PagingBeltSize > 0) {
      XMapWindow(display, belt[TOP]);
      mapped[TOP] = True;
    }
  }
  else {
    origin.y = rcVirt.y * rcRoot.height;
    if (PagingBeltSize > 0) {
      XUnmapWindow(display, belt[TOP]);
      mapped[TOP] = False;
    }
  }

  // check bottom-most side
  if (origin.y < (rcVirt.y + rcVirt.height - 1) * rcRoot.height) {
    if (PagingBeltSize > 0) {
      XMapWindow(display, belt[BOTTOM]);
      mapped[BOTTOM] = True;
    }
  }
  else {
    origin.y = (rcVirt.y + rcVirt.height - 1) * rcRoot.height;
    if (PagingBeltSize > 0) {
      XUnmapWindow(display, belt[BOTTOM]);
      mapped[BOTTOM] = False;
    }
  }

  if (PagingBeltSize > 0) {
    if (mapped[BOTTOM] && mapped[LEFT]) {
      XMapWindow(display, belt[BOTTOM_LEFT]);
      mapped[BOTTOM_LEFT] = True;
    }
    else {
      XUnmapWindow(display, belt[BOTTOM_LEFT]);
      mapped[BOTTOM_LEFT] = False;
    }

    if (mapped[BOTTOM] && mapped[RIGHT]) {
      XMapWindow(display, belt[BOTTOM_RIGHT]);
      mapped[BOTTOM_RIGHT] = True;
    }
    else {
      XUnmapWindow(display, belt[BOTTOM_RIGHT]);
      mapped[BOTTOM_RIGHT] = False;
    }
    
    if (mapped[TOP] && mapped[LEFT]) {
      XMapWindow(display, belt[TOP_LEFT]);
      mapped[TOP_LEFT] = True;
    }
    else {
      XUnmapWindow(display, belt[TOP_LEFT]);
      mapped[TOP_LEFT] = False;
    }

    if (mapped[TOP] && mapped[RIGHT]) {
      XMapWindow(display, belt[TOP_RIGHT]);
      mapped[TOP_RIGHT] = True;
    }
    else {
      XUnmapWindow(display, belt[TOP_RIGHT]);
      mapped[TOP_RIGHT] = False;
    }
  }
}

/*
 * RaisePagingBelt --
 *   Raise transparent windows for paging.
 */
void Paging::RaisePagingBelt()
{
  if (PagingBeltSize == 0)
    return;

  for (int i = 0; i < 8; i++) {
    if (mapped[i])
      XRaiseWindow(display, belt[i]);
  }
}

Paging::BeltPos Paging::GetBeltPos(const Point& pt)
{
  Rect rcRoot = rootQvwm->GetRect();
  Rect rcBelt[] = {
    Rect(PagingBeltSize, rcRoot.height - PagingBeltSize,
	 rcRoot.width - PagingBeltSize * 2, PagingBeltSize),
    Rect(PagingBeltSize, 0,
	 rcRoot.width - PagingBeltSize * 2, PagingBeltSize),
    Rect(0, PagingBeltSize,
	 PagingBeltSize, rcRoot.height - PagingBeltSize * 2),
    Rect(rcRoot.width - PagingBeltSize, PagingBeltSize,
	 PagingBeltSize, rcRoot.height - PagingBeltSize * 2),
    Rect(0, rcRoot.height - PagingBeltSize,
	 PagingBeltSize, PagingBeltSize),
    Rect(rcRoot.width - PagingBeltSize, rcRoot.height - PagingBeltSize,
	 PagingBeltSize, PagingBeltSize),
    Rect(0, 0,
	 PagingBeltSize, PagingBeltSize),
    Rect(rcRoot.width - PagingBeltSize, 0,
	 PagingBeltSize, PagingBeltSize)
  };

  for (int i = 0; i < 8; i++) {
    if (InRect(pt, rcBelt[i]))
      return (BeltPos)i;
  }
    
  ASSERT(FALSE);

  return BOTTOM;  // XXX
}

void Paging::SwitchPageLeft()
{
  if (Menu::CheckAnyMenusMapped())
    return;

  Rect rcRoot = rootQvwm->GetRect();
  Point oldOrigin = origin;

  origin.x -= rcRoot.width;
  PagingAllWindows(oldOrigin);

  desktop.ChangeFocusToCursor();
}

void Paging::SwitchPageRight()
{
  if (Menu::CheckAnyMenusMapped())
    return;

  Rect rcRoot = rootQvwm->GetRect();
  Point oldOrigin = origin;

  origin.x += rcRoot.width;
  PagingAllWindows(oldOrigin);

  desktop.ChangeFocusToCursor();
}

void Paging::SwitchPageUp()
{
  if (Menu::CheckAnyMenusMapped())
    return;

  Rect rcRoot = rootQvwm->GetRect();
  Point oldOrigin = origin;

  origin.y -= rcRoot.height;
  PagingAllWindows(oldOrigin);

  desktop.ChangeFocusToCursor();
}

void Paging::SwitchPageDown()
{
  if (Menu::CheckAnyMenusMapped())
    return;

  Rect rcRoot = rootQvwm->GetRect();
  Point oldOrigin = origin;

  origin.y += rcRoot.height;
  PagingAllWindows(oldOrigin);

  desktop.ChangeFocusToCursor();
}
