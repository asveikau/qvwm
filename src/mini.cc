/*
 * mini.cc
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
#include <X11/Xresource.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "mini.h"
#include "pager.h"
#include "taskbar.h"
#include "qvwmrc.h"
#include "paging.h"
#include "event.h"
#include "tooltip.h"

XContext Miniature::context;

/*
 * Miniature class constructor
 */
Miniature::Miniature(Qvwm* qvWm, const Rect& rect)
: qvWm(qvWm), focus(False)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  ASSERT(pager);

  rc = pager->ConvertToPagerSize(rect);

  attributes.border_pixel = darkGray.pixel;
  attributes.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                          Button2MotionMask | EnterWindowMask |
                          LeaveWindowMask | PointerMotionMask;
  valueMask = CWBorderPixel | CWEventMask;

  wMini = XCreateWindow(display, pager->pages, rc.x, rc.y, rc.width, rc.height,
			1, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  XSaveContext(display, wMini, context, (caddr_t)this);

  toolTip = new Tooltip();
}

Miniature::~Miniature()
{
  delete toolTip;

  XDeleteContext(display, wMini, context);

  XDestroyWindow(display, wMini);
}

/*
 * MoveMiniature --
 *   Move miniature window in miniature coordinate.
 */
void Miniature::MoveMiniature(const Point& pt)
{
  rc.x = pt.x;
  rc.y = pt.y;

  XMoveWindow(display, wMini, pt.x, pt.y);
}

/*
 * MoveResizeMiniature --
 *   Move and resize miniature window in miniature coordinate.
 */
void Miniature::MoveResizeMiniature(const Rect& rect)
{
  rc = rect;

  XMoveResizeWindow(display, wMini, rc.x, rc.y, rc.width, rc.height);
}

/*
 * DrawMiniature --
 *   Draw miniature window.
 */
void Miniature::DrawMiniature()
{
  if (focus)
    XSetWindowBackground(display, wMini, MiniatureActiveColor.pixel);
  else
    XSetWindowBackground(display, wMini, MiniatureColor.pixel);
  
  XClearWindow(display, wMini);
}

/*
 * Button1Press --
 *   Process the press of button1(left button).
 *   Switch to the page and give focus to the corresponding window.
 */
void Miniature::Button1Press(const Point& ptRoot)
{
  Point ptWin;
  Window junkChild;

  toolTip->Disable();

  ASSERT(pager);

  XRaiseWindow(display, pager->frame);

  if (UseTaskbar && !taskBar->IsHiding()) {
    ASSERT(paging);
    paging->RaisePagingBelt();
  }

  XTranslateCoordinates(display, root, pager->pages, ptRoot.x, ptRoot.y,
			&ptWin.x, &ptWin.y, &junkChild);

  pager->Button1Press(pager->pages, ptRoot, ptWin);

  if (ClickToFocus) {
    ASSERT(qvWm);
    qvWm->SetFocus();
    qvWm->RaiseWindow(True);
  }
}

/*
 * Button2Press --
 *   Process the press of button2(middle button or left & right botton).
 *   Move miniature window.
 */
void Miniature::Button2Press(const Point& ptRoot)
{
  XEvent ev;
  Point ptOld, ptNew, ptMini, ptReal;
  Rect rcReal;
  Bool pointer = False;
  
  toolTip->Disable();

  ASSERT(pager);
  XRaiseWindow(display, pager->frame);

  if (UseTaskbar && !taskBar->IsHiding()) {
    ASSERT(paging);
    paging->RaisePagingBelt();
  }

  ASSERT(qvWm);

  if (ClickToFocus) {
    if (!qvWm->CheckFocus())
      qvWm->SetFocus();
    qvWm->RaiseWindow(True);
    qvWm->AdjustPage();
  }

  XGrabPointer(display, wMini, True, Button2MotionMask | ButtonReleaseMask,
	       GrabModeAsync, GrabModeAsync, pager->GetPagesWin(), None,
	       CurrentTime);

  ptOld = ptRoot;

  if (!qvWm->CheckStatus(MAXIMIZE_WINDOW)) {
    while (1) {
      XMaskEvent(display,
		 Button2MotionMask | ButtonReleaseMask | ExposureMask |
		 ButtonPressMask | PointerMotionMask,
		 &ev);
      switch (ev.type) {
      case MotionNotify:
	ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
	rc.x += ptNew.x - ptOld.x;
	rc.y += ptNew.y - ptOld.y;
	ptOld = ptNew;
	
	ptMini = Point(rc.x, rc.y);
	MoveMiniature(ptMini);

	if (OpaqueMove) {
	  ptReal = pager->ConvertToRealPos(ptMini);
	  XMoveWindow(display, qvWm->GetFrameWin(),
		      ptReal.x - paging->origin.x,
		      ptReal.y - paging->origin.y);
	  
	  rcReal = qvWm->GetRect();
	  rcReal.x = ptReal.x;
	  rcReal.y = ptReal.y;
	  qvWm->ConfigureNewRect(rcReal);
      }
      break;
      
      case ButtonRelease:
	if (LockDragState) {
	  pointer = True;
	  XChangeActivePointerGrab(display,
				   ButtonPressMask | PointerMotionMask,
				   None, CurrentTime);
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
	break;
      }
    }
  }

decide:
  if (!OpaqueMove) {
    ptReal = pager->ConvertToRealPos(ptMini);
    XMoveWindow(display, qvWm->GetFrameWin(),
		ptReal.x - paging->origin.x, ptReal.y - paging->origin.y);
	  
    rcReal = qvWm->GetRect();
    rcReal.x = ptReal.x;
    rcReal.y = ptReal.y;
    qvWm->ConfigureNewRect(rcReal);
  }
  XUngrabPointer(display, CurrentTime);
}

void Miniature::Enter()
{
  toolTip->SetString(qvWm->GetName(), &fsTitle);
  toolTip->SetTimer();
}

void Miniature::Leave()
{
  toolTip->Disable();
}

void Miniature::PointerMotion()
{
  if (!toolTip->IsMapped())
    toolTip->ResetTimer();
}

/*
 * RestackMiniatures --
 *   Restack miniatures according to minis's order.
 */
void Miniature::RestackMiniatures(Miniature* minis[], int nminis)
{
  Window* wins = new Window[nminis];

  for (int i = 0; i < nminis; i++)
    wins[i] = minis[i]->wMini;

  XRestackWindows(display, wins, nminis);

  delete [] wins;
}

void Miniature::Initialize()
{
  Miniature::context = XUniqueContext();
}
