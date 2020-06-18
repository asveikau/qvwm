/*
 * pager.cc
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
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "event.h"
#include "qvwmrc.h"
#include "paging.h"
#include "pager.h"
#include "util.h"
#include "taskbar.h"
#include "callback.h"

Pager::Pager(InternGeom geom)
: rcOrig(geom.rc), gravity(geom.gravity)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  /*
   * If width or height is 0, set default value.
   */
  if (rcOrig.width == 0)
    rcOrig.width = 48;
  if (rcOrig.height == 0)
    rcOrig.height = 48;

  /*
   * Adjust pager size and position.
   */
  rcOrig.width = RoundDown(rcOrig.width, paging->rcVirt.width);
  rcOrig.height = RoundDown(rcOrig.height, paging->rcVirt.height);
  if (gravity.x == EAST)
    rcOrig.x = DisplayWidth(display, screen) + rcOrig.x - rcOrig.width;
  if (gravity.y == SOUTH)
    rcOrig.y = DisplayHeight(display, screen) + rcOrig.y - rcOrig.height;

  CalcPagerPos();

  /*
   * Create the frame window for pager.
   */
  attributes.background_pixel = PagerColor.pixel;
  attributes.override_redirect = True;
  attributes.event_mask = ExposureMask;
  valueMask = CWBackPixel | CWEventMask | CWOverrideRedirect;
  
  frame = XCreateWindow(display, root,
			rc.x, rc.y, rc.width, rc.height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  
  /*
   * Create the title window.
   */
  attributes.background_pixel = TitlebarActiveColor.pixel;
  attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
                          Button1MotionMask;
  valueMask = CWBackPixel | CWEventMask;
  
  title = XCreateWindow(display, frame,
			BORDER_WIDTH, BORDER_WIDTH, rcOrig.width, TITLE_HEIGHT,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  
  if (GradTitlebar) {
    Pixmap pixGrad = CreateGradPixmap(gradActivePattern, rcOrig.width, title);
    XSetWindowBackgroundPixmap(display, title, pixGrad);
  }

  /*
   * Create the window for the miniature of whole virtual screen.
   */
  attributes.background_pixel = PagerColor.pixel;
  attributes.event_mask = ButtonPressMask | ExposureMask;
  valueMask = CWBackPixel | CWEventMask;
  
  pages = XCreateWindow(display, frame,
			BORDER_WIDTH, TITLE_HEIGHT + BORDER_WIDTH + 1,
			rcOrig.width, rcOrig.height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  
  if (PagerImage) {
    imgPager = CreateImageFromFile(PagerImage, timer);
    if (imgPager)
      imgPager->SetBackground(pages);
  }

  ASSERT(paging);

  pageSize = Dim(rcOrig.width / paging->rcVirt.width,
		 rcOrig.height / paging->rcVirt.height);
  
  /*
   * Create the window standing for current page.
   */
  attributes.background_pixel = PagerActiveColor.pixel;
  attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
                          Button3MotionMask;
  valueMask = CWBackPixel | CWEventMask;
  
  visual = XCreateWindow(display, pages,
			 0, 0, pageSize.width - 2, pageSize.height - 2,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);
  
  XMapWindow(display, title);
  XMapWindow(display, pages);
  XMapWindow(display, visual);
}

Pager::~Pager()
{
  XDestroyWindow(display, frame);

  if (PagerImage)
    QvImage::Destroy(imgPager);
}

void Pager::MapPager()
{
  XMapWindow(display, frame);
  DrawVisualPage();

  RaisePager();
}

void Pager::UnmapPager()
{
  XUnmapWindow(display, frame);
}

/*
 * IsPagerWindows --
 *   Return True if the window is pager window.
 */
Bool Pager::IsPagerWindows(Window win)
{
  if (win == pages || win == visual || win == frame || win == title)
    return True;
  else
    return False;
}

void Pager::CalcPagerPos()
{
  Rect rect;

  if (UseTaskbar) {
    if (TaskbarAutoHide)
      rect = taskBar->GetScreenRectOnHiding();
    else
      rect = taskBar->GetScreenRectOnShowing();
  }

  rc.x = rcOrig.x + rect.x + BORDER_WIDTH * gravity.x;
  rc.y = rcOrig.y + rect.y;
  if (gravity.y == CENTER)
    rc.y -= BORDER_WIDTH + TITLE_HEIGHT + 1;
  else if (gravity.y == SOUTH)
    rc.y -= BORDER_WIDTH * 2 + TITLE_HEIGHT + 1;

  Rect rcRoot = rootQvwm->GetRect();

  if (UseTaskbar) {
    switch (taskBar->GetPos()) {
    case Taskbar::BOTTOM:
    case Taskbar::TOP:
      // South??Gravity
      if (gravity.y == SOUTH)
	rc.y -= rcRoot.height - rect.height;
      break;
      
    case Taskbar::LEFT:
    case Taskbar::RIGHT:
      // ??EastGravity
      if (gravity.x == EAST) {
	rc.x -= rcRoot.width - rect.width;
      }
      break;
    }
  }

  rc.width = rcOrig.width + BORDER_WIDTH * 2;
  rc.height = rcOrig.height + BORDER_WIDTH * 2 + TITLE_HEIGHT + 1;
}

/*
 * RecalcPager --
 *   Recalc the position of pager according to rcScreen and move the pager.
 */
void Pager::RecalcPager()
{
  CalcPagerPos();
  XMoveWindow(display, frame, rc.x, rc.y);
}

/*
 * ConvertToPagerPos
 *   Convert real position to position in pager.
 */
Point Pager::ConvertToPagerPos(const Point& pt)
{
  Rect rcRoot = rootQvwm->GetRect();
  Point ptMini;

  ASSERT(paging);

  ptMini.x = pt.x * pageSize.width / rcRoot.width -
    paging->rcVirt.x * pageSize.width;
  ptMini.y = pt.y * pageSize.height / rcRoot.height -
    paging->rcVirt.y * pageSize.height;

  return (ptMini);
}

/*
 * ConvertToPagerSize
 *   Convert real size to size in pager.
 */
Rect Pager::ConvertToPagerSize(const Rect& rect)
{
  Rect rcRoot = rootQvwm->GetRect();
  Rect rcMini;

  ASSERT(paging);

  rcMini.x = rect.x * pageSize.width / rcRoot.width -
    paging->rcVirt.x * pageSize.width;
  rcMini.y = rect.y * pageSize.height / rcRoot.height -
    paging->rcVirt.y * pageSize.height;
  rcMini.width = rect.width * pageSize.width / rcRoot.width;
  if (rcMini.width == 0)
    rcMini.width = 1;
  rcMini.height = rect.height * pageSize.height / rcRoot.height;
  if (rcMini.height == 0)
    rcMini.height = 1;

  return (rcMini);
}

/*
 * ConvertToRealPos
 *   Convert position in pager to real position.
 */
Point Pager::ConvertToRealPos(const Point& pt)
{
  Rect rcRoot = rootQvwm->GetRect();
  Point ptReal;

  ASSERT(paging);

  ptReal.x = pt.x * rcRoot.width / pageSize.width +
    paging->rcVirt.x * rcRoot.width;
  ptReal.y = pt.y * rcRoot.height / pageSize.height +
    paging->rcVirt.y * rcRoot.height;

  return (ptReal);
}

/*
 * DrawPages --
 *   Draw pages.
 */
void Pager::DrawPages()
{
  int i;

  ASSERT(paging);

  for (i = 1; i < paging->rcVirt.width; i++) {
    XSetForeground(display, gc, darkGray.pixel);
    XDrawLine(display, pages, gc,
	      pageSize.width*i - 1, 1,
	      pageSize.width * i - 1, rcOrig.height - 2);
    XSetForeground(display, gc, white.pixel);
    XDrawLine(display, pages, gc,
	      pageSize.width * i, 1,
	      pageSize.width * i, rcOrig.height - 2);
  }
  for (i = 1; i < paging->rcVirt.height; i++) {
    XSetForeground(display, gc, darkGray.pixel);
    XDrawLine(display, pages, gc,
	      1, pageSize.height * i - 1,
	      rcOrig.width - 2, pageSize.height * i - 1);
    XSetForeground(display, gc, white.pixel);
    XDrawLine(display, pages, gc,
	      1, pageSize.height * i,
	      rcOrig.width - 2, pageSize.height * i);
  }
}

/*
 * DrawVisualPage --
 *   Draw area standing for visual page.
 */
void Pager::DrawVisualPage()
{
  Rect rcRoot = rootQvwm->GetRect();
  Point ptVisual;

  ASSERT(paging);

  ptVisual.x = (paging->origin.x - paging->rcVirt.x * rcRoot.width)
    * rcOrig.width / (rcRoot.width * paging->rcVirt.width) + 1;
  ptVisual.y = (paging->origin.y - paging->rcVirt.y * rcRoot.height)
    * rcOrig.height / (rcRoot.height * paging->rcVirt.height) + 1;
  
  XMoveWindow(display, visual, ptVisual.x, ptVisual.y);
}

/*
 * DrawFrame --
 *   Draw pager frame.
 */
void Pager::DrawFrame()
{
  XPoint xp[3];

  xp[0].x = rc.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = rc.height - 2;
  
  XSetForeground(display, gc, gray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 1;
  xp[0].y = 0;
  xp[1].x = rc.width - 1;
  xp[1].y = rc.height - 1;
  xp[2].x = 0;
  xp[2].y = rc.height - 1;

  XSetForeground(display, ::gc, darkGrey.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = rc.height - 3;

  XSetForeground(display, ::gc, white.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 2;
  xp[0].y = 1;
  xp[1].x = rc.width - 2;
  xp[1].y = rc.height - 2;
  xp[2].x = 1;
  xp[2].y = rc.height - 2;

  XSetForeground(display, ::gc, darkGray.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
void Pager::Button1Press(Window win, const Point& ptRoot, const Point& ptWin)
{
  RaisePager();

  if (UseTaskbar && OnTopTaskbar && !OnTopPager)
    taskBar->RaiseTaskbar();

  if (UseTaskbar && !taskBar->IsHiding()) {
    ASSERT(paging);
    paging->RaisePagingBelt();
  }

  if (win == pages) {
    /*
     * Move the current page to the clicked area.
     */
    Point ptPage;
    Rect rcRoot = rootQvwm->GetRect();

    ptPage.x = ptWin.x * paging->rcVirt.width / rcOrig.width;
    ptPage.y = ptWin.y * paging->rcVirt.height / rcOrig.height;
    
    Point oldOrigin = paging->origin;
    paging->origin.x = (ptPage.x + paging->rcVirt.x) * rcRoot.width;
    paging->origin.y = (ptPage.y + paging->rcVirt.y) * rcRoot.height;

    if (paging->origin.x != oldOrigin.x || paging->origin.y != oldOrigin.y)
      PlaySound(PagerSound);

    paging->PagingAllWindows(oldOrigin);
  }
  else if (win == title) {
    if (DisableDesktopChange)
      return;

    /*
     * Move the pager.
     */
    XEvent ev;
    Point ptOld(ptRoot), ptNew, ptMove;
    Bool pointer = False;

    while (1) {
      XMaskEvent(display,
		 Button1MotionMask | ButtonReleaseMask | ExposureMask |
		 ButtonPressMask | PointerMotionMask,
		 &ev);
      switch (ev.type) {
      case MotionNotify:
	ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
	ptMove = ptNew - ptOld;
	rc.x += ptMove.x;
	rc.y += ptMove.y;
	rcOrig.x += ptMove.x;
	rcOrig.y += ptMove.y;
	ptOld = ptNew;
	XMoveWindow(display, frame, rc.x, rc.y);
	break;

      case ButtonRelease:
	if (LockDragState) {
	  pointer = True;
	  XGrabPointer(display, root, True,
		       ButtonPressMask | PointerMotionMask,
		       GrabModeAsync, GrabModeAsync, root, None, CurrentTime);
	  break;
	}
	else
	  return;

      case ButtonPress:
	if (pointer) {
	  XUngrabPointer(display, CurrentTime);
	  return;
	}
	break;

      case Expose:
	event.ExposeProc((const XExposeEvent &)ev);
	break;
      }
    }
  }
}

/*
 * Button3Press --
 *   Process the press of button3(mouse right button).
 *   You can move the visual page, or the current screen freely.
 */
void Pager::Button3Press(const Point& ptRoot)
{
  XEvent ev;
  Point ptOld, ptNew;
  Rect rcRoot = rootQvwm->GetRect();
  Bool pointer = False;
  
  RaisePager();

  if (UseTaskbar && !taskBar->IsHiding()) {
    ASSERT(paging);
    paging->RaisePagingBelt();
  }

  XGrabPointer(display, visual, True, Button3MotionMask | ButtonReleaseMask,
	       GrabModeAsync, GrabModeAsync, pages, None, CurrentTime);

  ptOld = ptRoot;

  while (1) {
    XMaskEvent(display,
	       Button3MotionMask | ButtonReleaseMask | ExposureMask |
	       ButtonPressMask | PointerMotionMask,
	       &ev);
    switch (ev.type) {
    case MotionNotify:
      {
	Point oldOrigin = paging->origin;

	ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
	paging->origin.x += (ptNew.x - ptOld.x) *
	  (rcRoot.width * paging->rcVirt.width / rcOrig.width);
	paging->origin.y += (ptNew.y - ptOld.y) *
	  (rcRoot.height * paging->rcVirt.height / rcOrig.height);
	ptOld = ptNew;
	paging->PagingAllWindows(oldOrigin);
	break;
      }

    case ButtonRelease:
      if (LockDragState) {
	pointer = True;
	XChangeActivePointerGrab(display, ButtonPressMask | PointerMotionMask,
				 None, CurrentTime);
	break;
      }
      else {
	XUngrabPointer(display, CurrentTime);
	return;
      }

    case ButtonPress:
      if (pointer) {
	XUngrabPointer(display, CurrentTime);
	return;
      }
      break;

    case Expose:
      event.ExposeProc((const XExposeEvent &)ev);
      break;
    }
  }
}

/*
 * Exposure --
 *   Process exposure event.
 */
void Pager::Exposure(Window win)
{
  if (win == frame)
    DrawFrame();
  else if (win == pages)
    DrawPages();
}

void Pager::RaisePager()
{
  if (OnTopPager)
    XRaiseWindow(display, frame);
  else {
    Window win[2];

    win[0] = desktop.GetTopWindow();
    win[1] = frame;
    XRestackWindows(display, win, 2);
  }
}
