/*
 * accessary.cc
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
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include "main.h"
#include "accessory.h"
#include "util.h"
#include "callback.h"
#include "timer.h"
#include "extra_image.h"

Accessory::Accessory(char* filename, char* pos, char* mode)
: m_filename(filename), m_pos(pos)
{
  if (strcmp(mode, "Background") == 0)
    m_mode = ACC_BACK;
  else if (strcmp(mode, "OnTop") == 0)
    m_mode = ACC_ONTOP;
  else if (strcmp(mode, "Application") == 0)
    m_mode = ACC_APP;
  else {
    QvwmError("invalid accessory mode: %s", mode);
    m_mode = ACC_BACK;
  }

  m_pid = 0;
}

Accessory::~Accessory()
{
  if (m_pid)
    kill(m_pid, SIGTERM);
}

void Accessory::CreateAccessory()
{
  // make a child process process accessories in order to separate from qvwm
  // heavy task to create (animation) image and animate it if necessary
  if ((m_pid = fork()) != 0)
    return;

  // signal initialization (all ignore)
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  signal(SIGHUP, SIG_DFL);

  // error handler initialization
  XSetErrorHandler((XErrorHandler)NULL);
  XSetIOErrorHandler((XIOErrorHandler)NULL);

  // X initialization
  display = XOpenDisplay(displayName);
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

#ifdef USE_IMLIB
  ExtraImage::Initialize();
#endif

  // timer initialization
  timer = new Timer();

  m_img = CreateImageFromFile(m_filename, timer);
  if (m_img == NULL)
    exit(1);

  XSetWindowAttributes attributes;
  unsigned long valueMask;
  int gravity;
  Point pt = CalcPosition(m_pos, &gravity);
  Dim szImg = m_img->GetSize();

  if (m_mode == ACC_APP)
    attributes.override_redirect = False;
  else
    attributes.override_redirect = True;
  valueMask = CWOverrideRedirect;

  m_win = XCreateWindow(display, root,
			pt.x, pt.y, szImg.width, szImg.height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  if (m_mode == ACC_APP) {
    XSizeHints hints;
    hints.flags = USPosition | PWinGravity;
    hints.win_gravity = gravity;
    XSetWMNormalHints(display, m_win, &hints);

    char name[256];
    sprintf(name, "Accessory: %s", m_filename);
    XStoreName(display, m_win, name);
  }

#ifdef USE_SHAPE
  if (m_mode != ACC_APP) {
    m_shapeMask = XCreatePixmap(display, m_win, szImg.width, szImg.height, 1);
    m_gcShape = XCreateGC(display, m_shapeMask, 0, 0);
  }
#endif // USE_SHAPE

  SetBgImage();
  
  m_img->SetDisplayCallback(new Callback<Accessory>(this,
						    &Accessory::SetBgImage));

  MapAccessory();
  EventLoop();

  XDestroyWindow(display, m_win);
  QvImage::Destroy(m_img);
#ifdef USE_SHAPE
  if (m_mode != ACC_APP) {
    XFreePixmap(display, m_shapeMask);
    XFreeGC(display, m_gcShape);
  }
#endif // USE_SHAPE

  XCloseDisplay(display);

  exit(0);
}

void Accessory::CreateShapedWindow()
{
#ifdef USE_SHAPE
  Dim szImg = m_img->GetSize();

  // make the whole transparent (pixel value 0 is transparent).
  XSetForeground(display, m_gcShape, 0);
  XFillRectangle(display, m_shapeMask, m_gcShape,
		 0, 0, szImg.width, szImg.height);
  
  // make opaque
  XSetForeground(display, m_gcShape, 1);
  XSetBackground(display, m_gcShape, 0);

  if (m_img->GetMask())
    XCopyPlane(display, m_img->GetMask(), m_shapeMask, m_gcShape,
	       0, 0, szImg.width, szImg.height, 0, 0, 1);
  else
    XFillRectangle(display, m_shapeMask, m_gcShape,
		   0, 0, szImg.width, szImg.height);

  XShapeCombineMask(display, m_win, ShapeBounding, 0, 0, m_shapeMask,
		    ShapeSet);
#endif // USE_SHAPE
}

Point Accessory::CalcPosition(char* pos, int* gravity)
{
  Dim szImg = m_img->GetSize();
  Dim szRoot(DisplayWidth(display, screen), DisplayHeight(display, screen));
  Point pt;
  
  if (strcmp(pos, "Center") == 0) {
    pt.x = (szRoot.width - szImg.width) / 2;
    pt.y = (szRoot.height - szImg.height) / 2;
    *gravity = CenterGravity;
  }
  else if (strcmp(pos, "Bottom") == 0) {
    pt.x = (szRoot.width - szImg.width) / 2;
    pt.y = szRoot.height - szImg.height;
    *gravity = SouthGravity;
  }
  else if (strcmp(pos, "Top") == 0) {
    pt.x = (szRoot.width - szImg.width) / 2;
    pt.y = 0;
    *gravity = NorthGravity;
  }
  else if (strcmp(pos, "Left") == 0) {
    pt.x = 0;
    pt.y = (szRoot.height - szImg.height) / 2;
    *gravity = WestGravity;
  }
  else if (strcmp(pos, "Right") == 0) {
    pt.x = szRoot.width - szImg.width;
    pt.y = (szRoot.height - szImg.height) / 2;
    *gravity = EastGravity;
  }
  else if (strcmp(pos, "TopLeft") == 0) {
    pt.x = 0;
    pt.y = 0;
    *gravity = NorthWestGravity;
  }
  else if (strcmp(pos, "TopRight") == 0) {
    pt.x = szRoot.width - szImg.width;
    pt.y = 0;
    *gravity = NorthEastGravity;
  }
  else if (strcmp(pos, "BottomLeft") == 0) {
    pt.x = 0;
    pt.y = szRoot.height - szImg.height;
    *gravity = SouthWestGravity;
  }
  else if (strcmp(pos, "BottomRight") == 0) {
    pt.x = szRoot.width - szImg.width;
    pt.y = szRoot.height - szImg.height;
    *gravity = SouthEastGravity;
  }
  else {
    unsigned int width, height;
    int bitmask;

    bitmask = XParseGeometry(pos, &pt.x, &pt.y, &width, &height);
    if ((bitmask & (XValue|YValue)) == (XValue|YValue)) {
      if (bitmask & XNegative)
	pt.x = szRoot.width + pt.x - szImg.width;
      if (bitmask & YNegative)
	pt.y = szRoot.height + pt.y - szImg.height;

      if ((bitmask & (XNegative | YNegative)) == 0)
	*gravity = NorthWestGravity;
      else if ((bitmask & (XNegative | YNegative)) == XNegative)
	*gravity = NorthEastGravity;
      else if ((bitmask & (XNegative | YNegative)) == YNegative)
	*gravity = SouthWestGravity;
      else if ((bitmask & (XNegative | YNegative)) == (XNegative | YNegative))
	*gravity = SouthEastGravity;
    }    
    else {
      pt.x = 0;
      pt.y = 0;
      *gravity = CenterGravity;
    }
  }

  return pt;
}

void Accessory::SetBgImage()
{
  XSetWindowBackgroundPixmap(display, m_win, m_img->GetPixmap());

  if (m_mode != ACC_APP)
    CreateShapedWindow();

  XClearWindow(display, m_win);
}

void Accessory::MapAccessory()
{
  XMapWindow(display, m_win);
  if (m_mode == ACC_BACK)
    XLowerWindow(display, m_win);
  else if (m_mode == ACC_ONTOP)
    XRaiseWindow(display, m_win);
}

void Accessory::EventLoop()
{
  XEvent ev;
  int fd = ConnectionNumber(display);
  fd_set fds;
  struct timeval tm, *tmp;

  XSynchronize(display, True);

  while (1) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (XPending(display) != 0) {
      XNextEvent(display, &ev);

      timer->CheckTimeout(&tm);
    }

    if (timer->CheckTimeout(&tm))
      tmp = &tm;
    else
      tmp = NULL;

    XFlush(display);

#if defined(__hpux) && !defined(_XPG4_EXTENDED)
    if (select(fd + 1, (int *)&fds, 0, 0, tmp) == 0) {    // timeout
#else
    if (select(fd + 1, &fds, 0, 0, tmp) == 0) {    // timeout
#endif
      timer->CheckTimeout(&tm);
    }
  }
}
