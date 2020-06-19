/*
 * radio_button.cc
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "main.h"
#include "misc.h"
#include "util.h"
#include "qvwm.h"
#include "radio_button.h"
#include "radio_set.h"
#include "qvwmrc.h"
#include "dialog.h"
#include "pixmap_image.h"
#include "bitmaps/radiobtn_normal.xpm"
#include "bitmaps/radiobtn_push.xpm"
#include "bitmaps/radiobtn_select.xpm"

XContext RadioButton::context;
QvImage* RadioButton::imgRadio[3];

RadioButton::RadioButton(Window parent, const Point& point, const char* btnname,
			 XFontSet& btnfs, ResourceId res_id)
: pt(point), name(btnname), fs(btnfs), rid(res_id)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  XRectangle ink, log;
  int width, height;

  status = 0;

  XmbTextExtents(fs, name, strlen(name), &ink, &log);
  width = BUTTON_SIZE + 5 + log.width + 1;
  height = Max(BUTTON_SIZE, log.height+2);

  rbY = (height - BUTTON_SIZE) / 2;
  rnY = (height - log.height) / 2 - log.y;

  rc = Rect(BUTTON_SIZE + 4 - log.x, (height - log.height) / 2,
	    log.width + 1, log.height);

  attributes.background_pixel = DialogColor.pixel;
  attributes.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                          EnterWindowMask | LeaveWindowMask;
  valueMask = CWBackPixel | CWEventMask;
  
  frame = XCreateWindow(display, parent, pt.x, pt.y, width, height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  XSaveContext(display, frame, context, (caddr_t)this);

  if (DialogImage) {
    imgRadioBack = Dialog::imgDialog->GetOffsetImage(Point(pt.x, pt.y));
    imgRadioBack->SetBackground(frame);
  }
}

/*
 * MapButton --
 *   Map radio button frame.
 */
RadioButton::~RadioButton()
{
  XDestroyWindow(display, frame);
}

/*
 * DrawButton --
 *   Draw radio button. If all is True, draw text too.
 */
void RadioButton::MapButton()
{
  XMapWindow(display, frame);
}

void RadioButton::DrawButton(Bool all)
{
  if (status & RADIO_PUSH)
    imgRadio[1]->Display(frame, Point(0, rbY));
  else
    imgRadio[0]->Display(frame, Point(0, rbY));

  if (status & RADIO_CHECK) {
    imgRadio[2]->Display(frame, Point(4, rbY+4));
    XSetForeground(display, gcDash, ButtonStringColor.pixel);
    XDrawRectangle(display, frame, gcDash, rc.x, rc.y, rc.width, rc.height); 
  }
  else {
    if (DialogImage) {
      XClearArea(display, frame, rc.x, rc.y, rc.width, 1, False);
      XClearArea(display, frame, rc.x, rc.y, 1, rc.height, False);
      XClearArea(display, frame, rc.x + rc.width, rc.y, 1, rc.height, False);
      XClearArea(display, frame, rc.x, rc.y + rc.height, rc.width, 1, False);
    }
    else {
      XSetForeground(display, gcDash, DialogColor.pixel);
      XDrawRectangle(display, frame, gcDash, rc.x, rc.y, rc.width, rc.height);
    }
  }

  /*
   * Draw text.
   */
  if (all) {
    XSetForeground(display, ::gc, DialogStringColor.pixel);
    XmbDrawString(display, frame, fs, ::gc, BUTTON_SIZE + 5, rnY,
		  name, strlen(name));
  }
}

/*
 * Button1Press --
 *   Process press of button1 (mouse left button).
 */
void RadioButton::Button1Press()
{
  XEvent ev;

  status |= RADIO_PUSH;
  DrawButton(False);
  
  while (1) {
    XMaskEvent(display, EnterWindowMask | LeaveWindowMask | ButtonReleaseMask,
	       &ev);
    switch (ev.type) {
    case EnterNotify:
      status |= RADIO_PUSH;
      DrawButton(False);
      break;

    case LeaveNotify:
      status &= ~RADIO_PUSH;
      DrawButton(False);
      break;

    case ButtonRelease:
      if (ev.xbutton.state & Button1Mask) {
	if (status & RADIO_PUSH) {
	  ASSERT(rs);
	  
	  if (rs->rbFocus != this) {
	    ASSERT(rs->rbFocus)
	    rs->rbFocus->status = 0;
	    rs->rbFocus->DrawButton(False);
	    rs->rbFocus = this;
	  }
	  status = RADIO_CHECK;
	  DrawButton(False);
	}
	return;
      }
      break;
    }
  }
}

void RadioButton::Initialize()
{
  context = XUniqueContext();

  imgRadio[0] = new PixmapImage(radiobtn_normal);
  imgRadio[1] = new PixmapImage(radiobtn_push);
  imgRadio[2] = new PixmapImage(radiobtn_select);
}
