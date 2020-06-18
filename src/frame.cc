/*
 * frame.cc
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
#include "tbutton.h"
#include "taskbar.h"
#include "paging.h"
#include "fbutton.h"
#include "qvwmrc.h"
#include "timer.h"
#include "callback.h"
#include "mini.h"
#include "pager.h"

QvImage* Qvwm::imgFrame;
QvImage* Qvwm::imgActiveFrame;

/*
 * CreateFrame --
 *   Create a frame (most lower base window).
 */
void Qvwm::CreateFrame(const Rect& rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = SubstructureRedirectMask | ButtonPressMask |
                          ButtonReleaseMask | EnterWindowMask |
			  LeaveWindowMask | ExposureMask | FocusChangeMask;
  attributes.background_pixel = FrameColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  ASSERT(paging);

  frame = XCreateWindow(display, root,
			rect.x - paging->origin.x, rect.y - paging->origin.y,
			rect.width, rect.height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  XSaveContext(display, frame, context, (caddr_t)this);
}

/*
 * CreateParent --
 *   Create a new direct parent window of original window. This window is
 *   the child of frame window.
 */
void Qvwm::CreateParent(const Rect& rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = SubstructureRedirectMask | ButtonPressMask |
                          ButtonReleaseMask | EnterWindowMask |
			  LeaveWindowMask | ExposureMask;
  attributes.background_pixel = FrameColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  parent = XCreateWindow(display, frame,
			 rect.x, rect.y, rect.width, rect.height,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);
  XSaveContext(display, parent, context, (caddr_t)this);
} 

/*
 * CreateSides --
 *   Create 4 side windows, which are used for resizing the window along
 *   either x or y direction. These windows are the children of frame window.
 */
void Qvwm::CreateSides(const Rect rect[])
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                          EnterWindowMask | LeaveWindowMask |
			  Button1MotionMask | Button2MotionMask;
  attributes.background_pixel = FrameColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  for (int i = 0; i < 4; i++) {
    side[i] = XCreateWindow(display, frame,
			    rect[i].x, rect[i].y,
			    rect[i].width, rect[i].height,
			    0, CopyFromParent, InputOutput, CopyFromParent,
			    valueMask, &attributes);
    XSaveContext(display, side[i], context, (caddr_t)this);

    if (FrameImage) {
      imgSide[i] = imgFrame->Duplicate();
      imgSide[i]->SetBackground(side[i]);
    }
    if (FrameActiveImage)
      imgActiveSide[i] = imgActiveFrame->Duplicate();
  }

  /*
   * Set cursor to each side window.
   */
  XDefineCursor(display, side[0], cursor[X_RESIZE]);
  XDefineCursor(display, side[1], cursor[X_RESIZE]);
  XDefineCursor(display, side[2], cursor[Y_RESIZE]);
  XDefineCursor(display, side[3], cursor[Y_RESIZE]);
}

/*
 * CreateCorners --
 *   Create 4 corner windows, which are used for resizing the window freely.
 *   These windows are the children of frame window.
 */
void Qvwm::CreateCorners(const Rect rect[])
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                          EnterWindowMask | LeaveWindowMask |
			  Button1MotionMask | Button2MotionMask;
  attributes.background_pixel = FrameColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  for (int i = 0; i < 4; i++) {
    corner[i] = XCreateWindow(display, frame,
			      rect[i].x, rect[i].y,
			      rect[i].width, rect[i].height,
			      0, CopyFromParent, InputOutput, CopyFromParent,
			      valueMask, &attributes);
    XSaveContext(display, corner[i], context, (caddr_t)this);

    if (FrameImage) {
      imgCorner[i] = imgFrame->Duplicate();
      imgCorner[i]->SetBackground(corner[i]);
    }
    if (FrameActiveImage)
      imgActiveCorner[i] = imgActiveFrame->Duplicate();
  }

  /*
   * Set cursor to each corner window.
   */
  XDefineCursor(display, corner[0], cursor[RD_RESIZE]);
  XDefineCursor(display, corner[1], cursor[LD_RESIZE]);
  XDefineCursor(display, corner[2], cursor[LD_RESIZE]);
  XDefineCursor(display, corner[3], cursor[RD_RESIZE]);
}

void Qvwm::CreateEdges(const Rect rect[])
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ExposureMask;
  attributes.background_pixel = FrameColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  for (int i = 0; i < 4; i++) {
    edge[i] = XCreateWindow(display, frame,
			    rect[i].x, rect[i].y,
			    rect[i].width, rect[i].height,
			    0, CopyFromParent, InputOutput, CopyFromParent,
			    valueMask, &attributes);
    XSaveContext(display, edge[i], context, (caddr_t)this);
  }
}

/*
 * ChangeFrameFocus --
 *   Change the background according to the focus and redraw frame window.
 */
void Qvwm::ChangeFrameFocus()
{
  int i;

  if (CheckFocus()) {
    if (FrameImage) {
      for (i = 0; i < 4; i++) {
	imgSide[i]->SetBackground(None);
	imgCorner[i]->SetBackground(None);
      }
    }

    if (FrameActiveImage) {
      for (i = 0; i < 4; i++) {
	imgActiveSide[i]->SetBackground(side[i]);
	imgActiveCorner[i]->SetBackground(corner[i]);
      }
    }
    else {
      if (FrameActiveColor.pixel != FrameColor.pixel) {
	for (i = 0; i < 4; i++) {
	  XSetWindowBackground(display, side[i], FrameActiveColor.pixel);
	  XClearWindow(display, side[i]);
	  DrawFrame(side[i]);

	  XSetWindowBackground(display, corner[i], FrameActiveColor.pixel);
	  XClearWindow(display, corner[i]);
	  DrawFrame(corner[i]);
	}
      }
    }
  }
  else {
    if (FrameActiveImage) {
      for (i = 0; i < 4; i++) {
	imgActiveSide[i]->SetBackground(None);
	imgActiveCorner[i]->SetBackground(None);
      }
    }
    
    if (FrameImage) {
      for (i = 0; i < 4; i++) {
	imgSide[i]->SetBackground(side[i]);
	imgCorner[i]->SetBackground(corner[i]);
      }
    }
    else {
      if (FrameColor.pixel != FrameActiveColor.pixel) {
	for (i = 0; i < 4; i++) {
	  XSetWindowBackground(display, side[i], FrameColor.pixel);
	  XClearWindow(display, side[i]);
	  DrawFrame(side[i]);
	  
	  XSetWindowBackground(display, corner[i], FrameColor.pixel);
	  XClearWindow(display, corner[i]);
	  DrawFrame(corner[i]);
	}
      }
    }
  }
}

/*
 * DrawFrame --
 *   Draw frame window.
 */
void Qvwm::DrawFrame(Window win)
{
  XPoint xp[3];

  if (CheckFlags(BORDER)) {
    if (win == side[F_LEFT]) {  // left side
      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, side[F_LEFT], gc, 0, 0, 0, rcSide[F_LEFT].height - 1);
      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, side[F_LEFT], gc, 1, 0, 1, rcSide[F_LEFT].height - 1);
      return;
    }
    else if (win == side[F_RIGHT]) {  // right side
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, side[F_RIGHT], gc,
		rcSide[F_RIGHT].width - 2, 0,
		rcSide[F_RIGHT].width - 2, rcSide[1].height - 1);
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, side[F_RIGHT], gc,
		rcSide[F_RIGHT].width - 1, 0,
		rcSide[F_RIGHT].width - 1, rcSide[F_RIGHT].height - 1);
      return;
    }
    else if (win == side[F_TOP]) {  // top side
      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, side[F_TOP], gc, 0, 0, rcSide[F_TOP].width - 1, 0);
      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, side[F_TOP], gc, 0, 1, rcSide[F_TOP].width - 1, 1);
      return;
    }
    else if (win == side[F_BOTTOM]) {  // bottom side
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, side[F_BOTTOM], gc,
		0, rcSide[F_BOTTOM].height - 2,
		rcSide[F_BOTTOM].width - 1, rcSide[F_BOTTOM].height - 2);
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, side[F_BOTTOM], gc,
		0, rcSide[F_BOTTOM].height - 1,
		rcSide[F_BOTTOM].width - 1, rcSide[F_BOTTOM].height - 1);
      return;
    }
    else if (win == corner[F_TLEFT]) {   // top-left corner
      xp[0].x = rcCorner[F_TLEFT].width - 1;
      xp[0].y = 0;
      xp[1].x = 0;
      xp[1].y = 0;
      xp[2].x = 0;
      xp[2].y = rcCorner[F_TLEFT].height - 1;

      XSetForeground(display, gc, gray.pixel);
      XDrawLines(display, corner[F_TLEFT], gc, xp, 3, CoordModeOrigin);

      xp[0].x = rcCorner[F_TLEFT].width - 1;
      xp[0].y = 1;
      xp[1].x = 1;
      xp[1].y = 1;
      xp[2].x = 1;
      xp[2].y = rcCorner[F_TLEFT].height - 1;
    
      XSetForeground(display, gc, white.pixel);
      XDrawLines(display, corner[F_TLEFT], gc, xp, 3, CoordModeOrigin);
      return;
    }
    else if (win == corner[F_TRIGHT]) {  // top-right corner
      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, corner[F_TRIGHT], gc,
		0, 0, rcCorner[F_TRIGHT].width - 2, 0);
      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, corner[F_TRIGHT], gc,
		0, 1, rcCorner[F_TRIGHT].width - 3, 1);
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, corner[F_TRIGHT], gc,
		rcCorner[F_TRIGHT].width - 2, 1,
		rcCorner[F_TRIGHT].width - 2, rcCorner[F_TRIGHT].height - 1);
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, corner[F_TRIGHT], gc,
		rcCorner[F_TRIGHT].width - 1, 0,
		rcCorner[F_TRIGHT].width - 1, rcCorner[F_TRIGHT].height - 1);
      return;
    }
    else if (win == corner[F_BLEFT]) {  // bottom-left corner
      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, corner[F_BLEFT], gc,
		0, 0, 0, rcCorner[F_BLEFT].height - 2);
      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, corner[F_BLEFT], gc,
		1, 0, 1, rcCorner[F_BLEFT].height - 3);
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, corner[F_BLEFT], gc,
		1, rcCorner[F_BLEFT].width - 2,
		rcCorner[F_BLEFT].width - 1, rcCorner[F_BLEFT].height - 2);
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, corner[F_BLEFT], gc,
		0, rcCorner[F_BLEFT].width - 1,
		rcCorner[F_BLEFT].width - 1, rcCorner[F_BLEFT].height - 1);
      return;
    }
    else if (win == corner[F_BRIGHT]) {  // bottom-right corner
      xp[0].x = rcCorner[F_BRIGHT].width - 2;
      xp[0].y = 0;
      xp[1].x = rcCorner[F_BRIGHT].width - 2;
      xp[1].y = rcCorner[F_BRIGHT].height - 2;
      xp[2].x = 0;
      xp[2].y = rcCorner[F_BRIGHT].height - 2;
      
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLines(display, corner[F_BRIGHT], gc, xp, 3, CoordModeOrigin);

      xp[0].x = rcCorner[F_BRIGHT].width - 1;
      xp[0].y = 0;
      xp[1].x = rcCorner[F_BRIGHT].width - 1;
      xp[1].y = rcCorner[F_BRIGHT].height - 1;
      xp[2].x = 0;
      xp[2].y = rcCorner[F_BRIGHT].height - 1;
      
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLines(display, corner[F_BRIGHT], gc, xp, 3, CoordModeOrigin);
      return;
    }
  }

  if (CheckFlags(BORDER_EDGE)) {
    if (win == edge[F_LEFT]) {
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLine(display, edge[F_LEFT], gc, 0, 0, 0, rcEdge[F_LEFT].height - 1);
      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, edge[F_LEFT], gc, 1, 0, 1, rcEdge[F_LEFT].height - 1);
    }
    else if (win == edge[F_RIGHT]) {
      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, edge[F_RIGHT], gc,
		0, 0, 0, rcEdge[F_RIGHT].height - 1);
      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, edge[F_RIGHT], gc,
		1, 0, 1, rcEdge[F_RIGHT].height - 1);
    }
    else if (win == edge[F_TOP]) {
      int yBase = CheckFlags(TITLE) ? 1 : 0;

      xp[0].x = rcEdge[F_TOP].width - 1;
      xp[0].y = yBase;
      xp[1].x = 0;
      xp[1].y = yBase;
      xp[2].x = 0;
      xp[2].y = yBase + 1;
      XSetForeground(display, gc, darkGray.pixel);
      XDrawLines(display, edge[F_TOP], gc, xp, 3, CoordModeOrigin);

      XSetForeground(display, gc, darkGrey.pixel);
      XDrawLine(display, edge[F_TOP], gc,
		1, yBase + 1, rcEdge[F_TOP].width - 2, yBase + 1);

      XSetForeground(display, gc, white.pixel);
      XDrawPoint(display, edge[F_TOP], gc, rcEdge[F_TOP].width - 1, yBase + 1);
    }
    else if (win == edge[F_BOTTOM]) {
      xp[0].x = 0;
      xp[0].y = 1;
      xp[1].x = rcEdge[F_BOTTOM].width - 1;
      xp[1].y = 1;
      xp[2].x = rcEdge[F_BOTTOM].width - 1;
      xp[2].y = 0;
      XSetForeground(display, gc, white.pixel);
      XDrawLines(display, edge[F_BOTTOM], gc, xp, 3, CoordModeOrigin);

      XSetForeground(display, gc, gray.pixel);
      XDrawLine(display, edge[F_BOTTOM], gc,
		1, 0, rcEdge[F_BOTTOM].width - 2, 0);

      XSetForeground(display, gc, darkGray.pixel);
      XDrawPoint(display, edge[F_BOTTOM], gc, 0, 0);
    }
  }
}

/*
 * CalcFramePos --
 *   Calculate the size of parts from frame window size.
 */
void Qvwm::CalcFramePos(const Rect& rcFrame, Rect* rcParent, Rect* rcTitle,
			Rect rcCorner[], Rect rcSide[], Rect rcEdge[],
			Rect* rcCtrl, Rect rcButton[])
{
  int i;
  int borderWidth, topBorder, titleHeight, titleEdge;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

  ASSERT(rcParent);

  /*
   * Parent window pos & size.
   */
  rcParent->x = borderWidth;
  rcParent->y = topBorder + titleHeight + titleEdge;
  rcParent->width = rcFrame.width - borderWidth * 2;
  rcParent->height = rcFrame.height - (borderWidth + topBorder + titleHeight
				       + titleEdge);

  ASSERT(rcTitle);

  /*
   * Title window pos & size.
   */
  rcTitle->x = topBorder;
  rcTitle->y = topBorder;
  rcTitle->width = rcFrame.width - topBorder * 2;
  rcTitle->height = TITLE_HEIGHT;

  ASSERT(rcCorner);

  /*
   * Corner window pos & size.
   */
  for (i = 0; i < 4; i++) {
    rcCorner[i].width = TITLE_HEIGHT + TOP_BORDER;
    rcCorner[i].height = TITLE_HEIGHT + TOP_BORDER;
  }
  rcCorner[F_TLEFT].x = rcCorner[F_BLEFT].x = 0;
  rcCorner[F_TLEFT].y = rcCorner[F_TRIGHT].y = 0;
  rcCorner[F_TRIGHT].x = rcCorner[F_BRIGHT].x =
    rcFrame.width - rcCorner[F_TRIGHT].width;
  rcCorner[F_BLEFT].y = rcCorner[F_BRIGHT].y =
    rcFrame.height - rcCorner[F_BLEFT].height;
    
  ASSERT(rcSide);

  /*
   * side window pos & size.
   */
  rcSide[F_LEFT].x = 0;
  rcSide[F_LEFT].y = rcSide[F_RIGHT].y = TITLE_HEIGHT + TOP_BORDER;
  rcSide[F_RIGHT].x = rcFrame.width - TOP_BORDER;
  rcSide[F_TOP].x = rcSide[F_BOTTOM].x = rcSide[F_LEFT].y;
  rcSide[F_TOP].y = 0;
  rcSide[F_BOTTOM].y = rcFrame.height - TOP_BORDER;
  rcSide[F_LEFT].width = rcSide[F_RIGHT].width = TOP_BORDER;
  rcSide[F_LEFT].height = rcSide[F_RIGHT].height =
    rcFrame.height - rcCorner[F_TLEFT].height * 2;
  if (rcSide[F_LEFT].height == 0)
    rcSide[F_LEFT].height = rcSide[F_RIGHT].height = 1;
  rcSide[F_TOP].width = rcSide[F_BOTTOM].width =
    rcFrame.width - rcCorner[F_TLEFT].width * 2;
  if (rcSide[F_TOP].width == 0)
    rcSide[F_TOP].width = rcSide[F_BOTTOM].width = 1;
  rcSide[F_TOP].height = rcSide[F_BOTTOM].height = TOP_BORDER;

  ASSERT(rcEdge);

  /*
   * border edge window pos & size.
   */
  rcEdge[F_TOP].x = rcEdge[F_BOTTOM].x = topBorder;
  rcEdge[F_TOP].y = topBorder + titleHeight;
  rcEdge[F_BOTTOM].y = rcFrame.height - borderWidth;
  rcEdge[F_TOP].width = rcEdge[F_BOTTOM].width = rcFrame.width - topBorder * 2;
  rcEdge[F_TOP].height = CheckFlags(TITLE) ? 3 : 2;
  rcEdge[F_BOTTOM].height = 2;
  rcEdge[F_LEFT].x = topBorder;
  rcEdge[F_LEFT].y = rcEdge[F_RIGHT].y =
    topBorder + titleHeight + rcEdge[F_TOP].height;
  rcEdge[F_RIGHT].x = rcFrame.width - borderWidth;
  rcEdge[F_LEFT].width = rcEdge[F_RIGHT].width = 2;
  rcEdge[F_LEFT].height = rcEdge[F_RIGHT].height =
    rcFrame.height - titleHeight - rcEdge[F_TOP].height - topBorder * 2 - 2;

  ASSERT(rcCtrl);

  /*
   * Control menu button pos & size.
   */
  rcCtrl->x = 2;
  rcCtrl->y = (rcTitle->height - TaskbarButton::SYMBOL_SIZE) / 2;
  rcCtrl->width = rcCtrl->height = TaskbarButton::SYMBOL_SIZE;

  ASSERT(rcButton);

  /*
   * Frame Button pos & size.
   */
  rcButton[0].x = rcTitle->width - (BUTTON_WIDTH*3 + 4);
  rcButton[1].x = rcTitle->width - (BUTTON_WIDTH*2 + 4);
  rcButton[2].x = rcTitle->width - (BUTTON_WIDTH + 2);
  if (!(CheckFlags(BUTTON3))) {
    rcButton[0].x = rcButton[1].x;
    rcButton[1].x = rcButton[2].x;
  }
  if (!(CheckFlags(BUTTON2)))
    rcButton[0].x = rcButton[1].x;
  for (i = 0; i < 3; i++) {
    rcButton[i].y = (rcTitle->height - BUTTON_HEIGHT) / 2;
    rcButton[i].width = BUTTON_WIDTH;
    rcButton[i].height = BUTTON_HEIGHT;
  }
}    

/*
 * CalcWindowPos --  rcOrig -> rc
 *   Calculate the correct window position.
 */
void Qvwm::CalcWindowPos(const Rect& base)
{
  Point gravity;
  Rect rect;
  int borderWidth, topBorder, titleHeight, titleEdge;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);
  gravity = GetGravityOffset();

  rc.x = base.x + rcScreen.x + (borderWidth - bwOrig) * gravity.x;
  rc.y = base.y + rcScreen.y;
  if (CheckFlags(STICKY)) {
    rc.x += paging->origin.x;
    rc.y += paging->origin.y;
  }
  if (gravity.y == CENTER)
    rc.y -= topBorder + titleHeight + titleEdge - bwOrig;
  else if (gravity.y == SOUTH)
    rc.y -= borderWidth + topBorder + titleHeight + titleEdge - bwOrig*2;

  if (UseTaskbar) {
    switch (taskBar->GetPos()) {
    case Taskbar::BOTTOM:
    case Taskbar::TOP:
      // South??Gravity
      if (gravity.y == SOUTH) {
	rect = taskBar->GetRect();
	rc.y -= rect.height;
      }
      break;
    case Taskbar::LEFT:
    case Taskbar::RIGHT:
      // ??EastGravity
      if (gravity.x == EAST) {
	rect = taskBar->GetRect();
	rc.x -= rect.width;
      }
      break;
    }
  }

  rc.width = base.width + borderWidth * 2;
  rc.height = base.height + borderWidth + topBorder + titleHeight + titleEdge;
}

/*
 * RedrawWindow --
 *   Redraw the window decoration.
 */
void Qvwm::RedrawWindow(Bool delay)
{
  Rect oldTitle = rcTitle;
  Rect rcCtrl, rcButton[3];

  CalcFramePos(rc, &rcParent, &rcTitle, rcCorner, rcSide, rcEdge, &rcCtrl,
	       rcButton);

  ASSERT(paging);

  XMoveResizeWindow(display, frame,
		    rc.x - paging->origin.x, rc.y - paging->origin.y,
		    rc.width, rc.height);
  XMoveResizeWindow(display, parent, rcParent.x, rcParent.y,
		    rcParent.width, rcParent.height);

  origWinSize = Dim(rcParent.width, rcParent.height);

  if (!delay)
    XResizeWindow(display, wOrig, origWinSize.width, origWinSize.height);

  if (CheckFlags(BORDER)) {
    for (int i = 0; i < 4; i++) {
      XMoveResizeWindow(display, side[i], rcSide[i].x, rcSide[i].y,
			rcSide[i].width, rcSide[i].height);
      XMoveWindow(display, corner[i], rcCorner[i].x, rcCorner[i].y);
    }
  }
  if (CheckFlags(BORDER_EDGE)) {
    for (int i = 0; i < 4; i++)
      XMoveResizeWindow(display, edge[i], rcEdge[i].x, rcEdge[i].y,
			rcEdge[i].width, rcEdge[i].height);
  }
  if (CheckFlags(TITLE)) {
    XMoveWindow(display, ctrl, rcCtrl.x, rcCtrl.y);
    for (int i = 0; i < 3; i++) {
      ASSERT(fButton[i]);
      fButton[i]->MoveResizeButton(rcButton[i]);

      if (TitlebarImage) {
	fButton[i]->SetBgImage(imgTitlebar,
			       Point(rcButton[i].x, rcButton[i].y));
	fButton[i]->SetBgActiveImage(imgTitlebar,
				     Point(rcButton[i].x, rcButton[i].y));
	fButton[i]->DrawButton();
      }
    }
    XMoveResizeWindow(display, title, rcTitle.x, rcTitle.y,
		      rcTitle.width, rcTitle.height);

    if (rcTitle.width != oldTitle.width) {
      if (GradTitlebar) {
	if (pixGrad != None)
	  XFreePixmap(display, pixGrad);
	pixGrad = None;

	if (pixActiveGrad != None)
	  XFreePixmap(display, pixActiveGrad);
	pixActiveGrad = None;
      }

      CalcShortName();
      DrawTitle(True);
    }
  }

  if (CheckFlags(SHAPED))
    SetShape();

  if (UsePager) {
    ASSERT(pager);
    ASSERT(mini);
    mini->MoveResizeMiniature(pager->ConvertToPagerSize(rc));
  }
}

void Qvwm::RecalcWindow()
{
  ASSERT(paging);

  if (CheckStatus(MAXIMIZE_WINDOW)) {
    rc = GetFixSize(rcScreen, hints.max_width, hints.max_height,
		    hints.width_inc, hints.height_inc,
		    hints.base_width, hints.base_height);
    ConfigureNewRect(Rect(rc.x + paging->origin.x, rc.y + paging->origin.y,
			  rc.width, rc.height));
    RedrawWindow();
  }
  else {
    CalcWindowPos(rcOrig);
    SendConfigureEvent();
    XMoveWindow(display, frame,
		rc.x - paging->origin.x, rc.y - paging->origin.y);
  }
}

void Qvwm::CreateFramePixmap()
{
  if (FrameImage) {
    imgFrame = CreateImageFromFile(FrameImage, timer);
    if (imgFrame == NULL) {
      delete [] FrameImage;
      FrameImage = NULL;
    }
  }
  if (FrameActiveImage) {
    imgActiveFrame = CreateImageFromFile(FrameActiveImage, timer);
    if (imgActiveFrame == NULL) {
      delete [] FrameActiveImage;
      FrameActiveImage = NULL;
    }
  }
}
