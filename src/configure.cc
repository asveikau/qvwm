/*
 * configure.cc
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
#include "misc.h"
#include "qvwm.h"
#include "fbutton.h"
#include "paging.h"
#include "taskbar.h"
#include "timer.h"
#include "callback.h"
#include "gnome.h"
#include "qvwmrc.h"

/*
 * Configure --
 *   Change window position and size according to cre.
 *   rcOrig = cre - rcScreen + paging
 *   rc = cre + gravity + paging
 */
void Qvwm::Configure(const XConfigureRequestEvent* cre)
{
  Qvwm* tmpQvwm;
  XWindowChanges xwc;
  int borderWidth, topBorder, titleHeight, titleEdge;

  ASSERT(cre);

  if (cre->value_mask & CWStackMode) {
    // XXX this processing is quick hack
    switch (cre->detail) {
    case Above:
      RaiseWindow(True);
      break;

    case Below:
      LowerWindow();
      break;

    default:
      xwc.stack_mode = cre->detail;

      if (cre->value_mask & CWSibling) {
	if (XFindContext(display, cre->above, Qvwm::context,
			 (caddr_t *)&tmpQvwm) == XCSUCCESS)
	  xwc.sibling = tmpQvwm->frame;
	else
	  xwc.sibling = cre->above;
	
	XConfigureWindow(display, frame,
			 cre->value_mask & (CWSibling | CWStackMode), &xwc);
      }
      else
	XConfigureWindow(display, frame, cre->value_mask & CWStackMode, &xwc);
    }
  }

  if (cre->value_mask & CWBorderWidth)
    bwOrig = cre->border_width;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

  ASSERT(paging);

  if (cre->value_mask & CWX) {
    rcOrig.x = cre->x - rcScreen.x;
    if (!CheckFlags(STICKY))
      rcOrig.x += paging->origin.x;
    rc.x = cre->x + paging->origin.x - borderWidth;
  }
  if (cre->value_mask & CWY) {
    rcOrig.y = cre->y - rcScreen.y;
    if (!CheckFlags(STICKY))
      rcOrig.y += paging->origin.y;
    rc.y = cre->y + paging->origin.y;
    rc.y -= topBorder + titleHeight + titleEdge;
  }
  if (cre->value_mask & CWWidth) {
    rcOrig.width = cre->width;
    rc.width = cre->width + borderWidth * 2;
  }
  if (cre->value_mask & CWHeight) {
    rcOrig.height = cre->height;
    rc.height = cre->height + borderWidth + topBorder + titleHeight
      + titleEdge;
  }

  if (cre->value_mask & (CWX | CWY | CWWidth | CWHeight)) {
    if (CheckStatus(MAXIMIZE_WINDOW) && !CheckStatus(MINIMIZE_WINDOW)) {
      ResetStatus(MAXIMIZE_WINDOW);
      flags = flagsBak;

      ASSERT(fButton[FrameButton::MAXIMIZE]);
      
      fButton[FrameButton::MAXIMIZE]->SetState(Button::NORMAL);
      fButton[FrameButton::MAXIMIZE]->ChangeImage(FrameButton::MAXIMIZE);
      fButton[FrameButton::MAXIMIZE]->DrawButton();

      for (int i = 0; i < 4; i++) {
	XMapWindow(display, side[i]);
	XMapWindow(display, corner[i]);
      }

      if (CheckFlags(THIN_BORDER))
	XSetWindowBorderWidth(display, frame, 1);

      Gnome::ResetState(this,
			WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ);

    }

    SendConfigureEvent();
    RedrawWindow();
  }
}

void Qvwm::Circulate(int place)
{
  switch (place) {
  case PlaceOnTop:
    RaiseWindow(True);
    break;

  case PlaceOnBottom:
    LowerWindow();
    break;
  }
}

/*
 * ConfigureNewRc --
 *   Change original window according to rcNew (frame rc).
 *   rcOrig = rc - rcScreen - gravity
 */
void Qvwm::ConfigureNewRect(const Rect& rcNew, Bool delay)
{
  int borderWidth, topBorder, titleHeight, titleEdge;
  Point gravity;
  Rect rect;

  rc = rcNew;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

  gravity = GetGravityOffset();

  rcOrig.x = rcNew.x - rcScreen.x - (borderWidth - bwOrig) * gravity.x;
  rcOrig.y = rcNew.y - rcScreen.y;
  if (CheckFlags(STICKY)) {
    rcOrig.x -= paging->origin.x;
    rcOrig.y -= paging->origin.y;
  }
  if (gravity.y == CENTER)
    rcOrig.y += topBorder + titleHeight + titleEdge - bwOrig;
  else if (gravity.y == SOUTH)
    rcOrig.y += borderWidth + topBorder + titleHeight + titleEdge - bwOrig * 2;
  
  rcOrig.width = rcNew.width - borderWidth * 2;
  rcOrig.height = rcNew.height - (titleHeight + borderWidth + topBorder
				  + titleEdge);

  /*
   * Adjust taskbar position.
   */
  if (UseTaskbar) {
    switch (taskBar->GetPos()) {
    case Taskbar::BOTTOM:
    case Taskbar::TOP:
      // South??Gravity
      if (gravity.y == SOUTH) {
	rect = taskBar->GetRect();
	rcOrig.y += rect.height;
      }
      break;
      
    case Taskbar::LEFT:
    case Taskbar::RIGHT:
      // ??EastGravity
      if (gravity.x == EAST) {
	rect = taskBar->GetRect();
	rcOrig.x += rect.width;
      }
      break;
    }
  }

  if (!delay)
    SendConfigureEvent();
}

/*
 * SendConfigureEvent --
 *   Send event to the window whose position and size changed.
 */
void Qvwm::SendConfigureEvent()
{
  XEvent clientEvent;
  int borderWidth, topBorder, titleHeight, titleEdge;
  
  clientEvent.type = ConfigureNotify;
  clientEvent.xconfigure.display = display;
  clientEvent.xconfigure.event = wOrig;
  clientEvent.xconfigure.window = wOrig;
      
  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

  ASSERT(paging);

  clientEvent.xconfigure.x = rc.x - paging->origin.x + borderWidth;
  clientEvent.xconfigure.y = rc.y - paging->origin.y + topBorder
    + titleHeight + titleEdge;
  clientEvent.xconfigure.width = rc.width - borderWidth * 2;
  clientEvent.xconfigure.height = rc.height - (borderWidth + topBorder
					       + titleHeight + titleEdge);

  clientEvent.xconfigure.border_width = 0;
  clientEvent.xconfigure.above = frame;
  clientEvent.xconfigure.override_redirect = False;
  
  XSendEvent(display, wOrig, False, StructureNotifyMask, &clientEvent);
}

/*
 * GetBorderAndTitle --
 *   Get border width, top border width, title height, title edge height
 *   according to the flags.
 *
 *  top border width
 *   /\
 *   +------------------------------+  \ top border width
 *   | +--------------------------+ |  /
 *   | | TITLE                    | |  - title height 
 *   + +--------------------------+ |  \
 *   |  +------------------------+  |  / title edge
 *   |  |                        |  |
 *   |  |                        |  |
 *   \ /
 *  border width
 */
void Qvwm::GetBorderAndTitle(int& borderWidth, int& topBorder,
			     int& titleHeight, int& titleEdge)
{
  if (CheckFlags(BORDER)) {
    borderWidth = BORDER_WIDTH;
    topBorder = TOP_BORDER;
  }
  else {
    borderWidth = 2;
    topBorder = 0;
  }

  if (!CheckFlags(BORDER_EDGE)) {
    borderWidth -= 2;
    if (CheckFlags(TITLE))
      titleEdge = 1;  // a line between title and border edge
    else
      titleEdge = 0;
  }
  else {
    if (CheckFlags(TITLE))
      titleEdge = TITLE_EDGE;
    else
      titleEdge = TITLE_EDGE - 1;
  }

  titleHeight = CheckFlags(TITLE) ? TITLE_HEIGHT : 0;
}
