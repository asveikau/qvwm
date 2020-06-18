/*
 * button.cc
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
#include <X11/Xresource.h>
#include "main.h"
#include "misc.h"
#include "button.h"
#include "event.h"
#include "qvwmrc.h"
#include "image.h"
#include "tooltip.h"

XContext Button::context;

Button::Button(Window pWin, const Rect& rect)
: parent(pWin), rc(rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  // initialize state
  state = NORMAL;

  attributes.background_pixel = ButtonColor.pixel;
  attributes.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                          EnterWindowMask | LeaveWindowMask |
                          PointerMotionMask ;
  valueMask = CWBackPixel | CWEventMask;

  frame = XCreateWindow(display, parent,
			rc.x, rc.y, rc.width, rc.height, 0,
			CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  // save frame window id to context
  XSaveContext(display, frame, Button::context, (caddr_t)this);

  toolTip = new Tooltip();

  imgBack = NULL;
  imgActiveBack = NULL;
}

Button::~Button()
{
  XDestroyWindow(display, frame);

  // delete frame window id to context
  XDeleteContext(display, frame, Button::context);

  QvImage::Destroy(imgBack);
  QvImage::Destroy(imgActiveBack);

  delete toolTip;
}  

/*
 * MapButton --
 *   Map button frame.
 */
void Button::MapButton()
{
  XMapWindow(display, frame);
}

/*
 * UnmapButton --
 *   Unmap button frame.
 */
void Button::UnmapButton()
{
  XUnmapWindow(display, frame);
}

/*
 * DrawButton --
 *   Draw button according to state.
 */
void Button::DrawButton()
{
  XPoint xp[3];

  xp[0].x = rc.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = rc.height - 2;

  if (state == PUSH)
    XSetForeground(display, gc, darkGrey.pixel);
  else
    XSetForeground(display, gc, white.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 1;
  xp[0].y = 0;
  xp[1].x = rc.width - 1;
  xp[1].y = rc.height - 1;
  xp[2].x = 0;
  xp[2].y = rc.height - 1;

  if (state == PUSH)
    XSetForeground(display, gc, white.pixel);
  else
    XSetForeground(display, gc, darkGrey.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = rc.height - 3;

  if (state == PUSH)
    XSetForeground(display, gc, darkGray.pixel);
  else
    XSetForeground(display, gc, gray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 2;
  xp[0].y = 1;
  xp[1].x = rc.width - 2;
  xp[1].y = rc.height - 2;
  xp[2].x = 1;
  xp[2].y = rc.height - 2;
  
  if (state == PUSH)
    XSetForeground(display, gc, gray.pixel);
  else
    XSetForeground(display, gc, darkGray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
}

/*
 * MoveResizeButton --
 *   Move and resize button.
 */
void Button::MoveResizeButton(const Rect& rect)
{
  rc = rect;

  XMoveResizeWindow(display, frame, rc.x, rc.y, rc.width, rc.height);
}

// img must be passed as only a pointer
void Button::SetBgImage(QvImage* img, const Point& off)
{
  if (img) {
    QvImage::Destroy(imgBack);
    imgBack = img->GetOffsetImage(off);
  }
}  

// image must be passed as only a pointer
void Button::SetBgActiveImage(QvImage* img, const Point& off)
{
  if (img) {
    QvImage::Destroy(imgActiveBack);
    imgActiveBack = img->GetOffsetImage(off);
  }
}  

/*
 * Button1Press --
 *   Process press of button1 (mouse left button)
 */
void Button::Button1Press()
{
  XEvent ev;
  ButtonState bs;

  toolTip->Disable();

  state = PUSH;
  DrawButton();

  while (1) {
    XMaskEvent(display, EnterWindowMask | LeaveWindowMask | ButtonReleaseMask,
	       &ev);
    switch (ev.type) {
    case EnterNotify:
      if (ev.xcrossing.window == frame) {
	state = PUSH;
	DrawButton();
      }
      break;

    case LeaveNotify:
      if (ev.xcrossing.window == frame) {
	state = NORMAL;
	DrawButton();
      }
      break;

    case ButtonRelease:
      if (ev.xbutton.state & Button1Mask) {
	bs = state;
	state = NORMAL;

	ExecButtonFunc(bs);
	DrawButton();
	return;
      }
    }
  }
}

void Button::Button3Release(const Point& ptRoot)
{
}

void Button::Enter()
{
  toolTip->SetTimer();
}

void Button::Leave()
{
  toolTip->Disable();
}

void Button::PointerMotion()
{
  if (!toolTip->IsMapped())
    toolTip->ResetTimer();
}

void Button::Initialize()
{
  Button::context = XUniqueContext();
}
