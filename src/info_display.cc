/*
 * info_display.cc
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
#include "main.h"
#include "info_display.h"

#define X_MARGIN 10
#define Y_MARGIN 6

InfoDisplay::InfoDisplay()
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ExposureMask;
  attributes.background_pixel = gray.pixel;
  valueMask = CWBackPixel | CWEventMask;

  frame = XCreateWindow(display, root,
			0, 0, 1, 1,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  strInfo[0] = '\0';
}

InfoDisplay::~InfoDisplay()
{
  XDestroyWindow(display, frame);
}

void InfoDisplay::Map()
{
  XMapRaised(display, frame);
}

void InfoDisplay::Unmap()
{
  XUnmapWindow(display, frame);

  strInfo[0] = '\0';
}

void InfoDisplay::Draw()
{
  int len = strlen(strInfo);
  XRectangle ink, log;
  Point pt;

  XmbTextExtents(fsTitle, strInfo, len, &ink, &log);

  rc.width = log.width + X_MARGIN * 2;
  rc.height = log.height + Y_MARGIN * 2;
  rc.x = (DisplayWidth(display, screen) - rc.width) / 2;
  rc.y = (DisplayHeight(display, screen) - rc.height) / 2;

  XMoveResizeWindow(display, frame, rc.x, rc.y, rc.width, rc.height);
  XClearWindow(display, frame);

  pt.x = X_MARGIN - log.x;
  pt.y = Y_MARGIN - log.y;

  XSetForeground(display, gc, black.pixel);
  XmbDrawString(display, frame, fsTitle, gc, pt.x, pt.y, strInfo, len);

  XPoint xp[3];

  xp[0].x = rc.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = rc.height - 2;

  XSetForeground(display, gc, white.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 1;
  xp[0].y = 0;
  xp[1].x = rc.width - 1;
  xp[1].y = rc.height - 1;
  xp[2].x = 0;
  xp[2].y = rc.height - 1;

  XSetForeground(display, gc, darkGrey.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = rc.height - 3;

  XSetForeground(display, gc, gray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 2;
  xp[0].y = 1;
  xp[1].x = rc.width - 2;
  xp[1].y = rc.height - 2;
  xp[2].x = 1;
  xp[2].y = rc.height - 2;
  
  XSetForeground(display, gc, darkGray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  // internal box
  xp[0].x = rc.width - 5;
  xp[0].y = 3;
  xp[1].x = 3;
  xp[1].y = 3;
  xp[2].x = 3;
  xp[2].y = rc.height - 5;

  XSetForeground(display, gc, darkGray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 4;
  xp[0].y = 2;
  xp[1].x = rc.width - 4;
  xp[1].y = rc.height - 4;
  xp[2].x = 2;
  xp[2].y = rc.height - 4;

  XSetForeground(display, gc, white.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
}  

void InfoDisplay::NotifyPosition(const Point& pt)
{
  sprintf(strInfo, "%d, %d", pt.x, pt.y);
  Draw();
}

void InfoDisplay::NotifyShape(const Rect& rect)
{
  sprintf(strInfo, "%d, %d, %d, %d", rect.x, rect.y, rect.width, rect.height);
  Draw();
}
