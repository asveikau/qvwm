/*
 * dialog.cc
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "misc.h"
#include "dialog.h"
#include "qvwmrc.h"
#include "timer.h"
#include "image.h"

QvImage* Dialog::imgDialog;
Timer* Dialog::dlgTimer;

Dialog::Dialog(const Rect& rect)
: rc(rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  
  attributes.background_pixel = DialogColor.pixel;
  attributes.event_mask = ExposureMask | KeyPressMask;
  valueMask = CWBackPixel | CWEventMask;

  frame = XCreateWindow(display, root, rc.x, rc.y, rc.width, rc.height, 0,
			CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  
  if (DialogImage)
    imgDialog->SetBackground(frame);

  XSetTransientForHint(display, frame, root);
}

Dialog::~Dialog()
{
  XDestroyWindow(display, frame);
}

void Dialog::SetRect(const Rect& rect)
{
  rc = rect;

  XMoveResizeWindow(display, frame, rect.x, rect.y, rect.width, rect.height);
}

void Dialog::SetTitle(char* dlgname)
{
  XTextProperty ct;

  name = dlgname;

  XmbTextListToTextProperty(display, &name, 1, XCompoundTextStyle, &ct);
  XSetWMName(display, frame, &ct);
}

/*
 * MapDialog --
 *   Map dialog.
 */
void Dialog::MapDialog()
{
  XMapRaised(display, frame);

  XSetInputFocus(display, frame, RevertToParent, CurrentTime);
}

void Dialog::UnmapDialog()
{
  XUnmapWindow(display, frame);

  Qvwm::SetFocusToActiveWindow();
}

void Dialog::DrawClientWin()
{
}

/*
 * EventLoop --
 *   Process events when dialog is mapped.
 */
ResourceId Dialog::EventLoop()
{
  XEvent ev;
  int fd = ConnectionNumber(display);
  fd_set fds;
  struct timeval tm;
  char key;
  KeySym sym;
  ResourceId id;

  while (1) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (XPending(display) != 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
      case Expose:
	Exposure(ev.xexpose.window);
	break;

      case ButtonPress:
	if (ev.xbutton.button == Button1) {
	  id = Button1Press(ev.xbutton.window);
	  if (id != NO_ID)
	    return id;
	}
	else
	  XAllowEvents(display, ReplayPointer, CurrentTime);
	break;

      case KeyPress:
	if (XLookupString((XKeyEvent *)&ev, &key, 1, &sym, NULL) == 1) {
	  id = FindShortCutKey(key);
	  if (id != NO_ID)
	    return id;
	}
      }

      dlgTimer->CheckTimeout(&tm);
    }

    if (!dlgTimer->CheckTimeout(&tm)) {
      tm.tv_sec = 1;
      tm.tv_usec = 0;
	
      XFlush(display);
    }

#if defined(__hpux) && !defined(_XPG4_EXTENDED)
    if (select(fd + 1, (int *)&fds, 0, 0, &tm) == 0) {    // timeout
#else
    if (select(fd + 1, &fds, 0, 0, &tm) == 0) {    // timeout
#endif
      dlgTimer->CheckTimeout(&tm);
    }
  }
}

void Dialog::Exposure(Window win)
{
}

/*
 * Button1Press --
 *   Process press of button1 (mouse left button)
 */
ResourceId Dialog::Button1Press(Window win)
{
  return NO_ID;
}

ResourceId Dialog::FindShortCutKey(char key)
{
  return NO_ID;
}

void Dialog::Initialize()
{
  dlgTimer = new Timer();

  if (DialogImage) {
    imgDialog = CreateImageFromFile(DialogImage, dlgTimer);
    if (imgDialog == NULL) {
      delete [] DialogImage;
      DialogImage = NULL;
    }
  }
}
