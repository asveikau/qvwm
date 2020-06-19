/*
 * tooltip.cc
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
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "main.h"
#include "tooltip.h"
#include "misc.h"
#include "timer.h"
#include "qvwmrc.h"
#include "util.h"

XContext Tooltip::context;

#define XMARGIN 3
#define YMARGIN 1

Tooltip::Tooltip(const char* str, XFontSet* fs)
: m_str(str), m_fs(fs)
{
  m_win = None;
  m_mapped = False;
}

void Tooltip::CreateWindow()
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  XRectangle ink, log;

  if (m_fs == NULL)
    m_fs = &fsTitle;

  XmbTextExtents(*m_fs, m_str, strlen(m_str), &ink, &log);

  m_rc.x = log.x;
  m_rc.y = log.y;
  m_rc.width = log.width + XMARGIN * 2;
  m_rc.height = log.height + YMARGIN * 2;

  attributes.event_mask = ExposureMask;
  attributes.background_pixel = TooltipColor.pixel;
  attributes.border_pixel = TooltipStringColor.pixel;
  attributes.override_redirect = True;
  valueMask = CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWEventMask;

  m_win = XCreateWindow(display, root,
			0, 0, m_rc.width, m_rc.height,
			1, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  XSaveContext(display, m_win, context, (caddr_t)this);

  m_mapped = False;
}

Tooltip::~Tooltip()
{
  Disable();

  if (m_win != None) {
    XDeleteContext(display, m_win, context);
    XDestroyWindow(display, m_win);
  }
}

void Tooltip::SetString(const char* str, XFontSet* fs)
{
  if (str == NULL)
    return;

  m_str = str;
  m_fs = fs;

  m_rc.width = 0;
}

void Tooltip::MapTooltip()
{
  Window junkRoot, junkChild;
  Point ptRoot, ptJunk, pt;
  unsigned int mask;

  if (m_str == NULL)
    return;

  // on-demand creation
  if (m_win == None)
    CreateWindow();

  // on-demand resize
  if (m_rc.width == 0)
    ResizeTooltip();

  XQueryPointer(display, root, &junkRoot, &junkChild, &ptRoot.x, &ptRoot.y,
		&ptJunk.x, &ptJunk.y, &mask);

  pt = AdjustPosition(ptRoot);

  if (ptRoot.y < pt.y)
    XMoveResizeWindow(display, m_win, pt.x, pt.y, m_rc.width, 1);
  else
    XMoveResizeWindow(display, m_win, pt.x, pt.y - m_rc.height, m_rc.width, 1);

  XMapRaised(display, m_win);

  for (int i = 1; i <= TooltipMotionSpeed; i++) {
    int height = m_rc.height * i / TooltipMotionSpeed;
    int y;

    if (height == 0)
      height = 1;

    if (ptRoot.y < pt.y)
      y = pt.y;
    else
      y = pt.y + m_rc.height - height;

    XMoveResizeWindow(display, m_win, pt.x, y, m_rc.width, height);

    XFlush(display);
    usleep(10000);
  }

  m_mapped = True;

  timer->SetTimeout(TooltipDisplayTime,
		    new Callback<Tooltip>(this, &Tooltip::UnmapTooltip));
}

void Tooltip::ResizeTooltip()
{
  XRectangle ink, log;
  
  if (m_fs == NULL)
    m_fs = &fsTitle;
    
  XmbTextExtents(*m_fs, m_str, strlen(m_str), &ink, &log);
  
  m_rc.x = log.x;
  m_rc.y = log.y;
  m_rc.width = log.width + XMARGIN * 2;
  m_rc.height = log.height + YMARGIN * 2;
}

Point Tooltip::AdjustPosition(const Point& pt)
{
  Dim szRoot(DisplayWidth(display, screen), DisplayHeight(display, screen));
  Point ptAdj;

  if (pt.x + m_rc.width >= szRoot.width - 2) {
    ptAdj.x = szRoot.width - 2 - m_rc.width;
    if (ptAdj.x < 2)
      ptAdj.x = 2;
  }
  else
    ptAdj.x = pt.x;

  // adjust vertical position
  if (pt.y + 19 + m_rc.height >= szRoot.height - 1)
    ptAdj.y = pt.y - 4 - m_rc.height;  // above the pointer
  else
    ptAdj.y = pt.y + 19;               // below the pointer

  return ptAdj;
}

void Tooltip::UnmapTooltip()
{
  XUnmapWindow(display, m_win);

  m_mapped = False;

  timer->ResetTimeout(new Callback<Tooltip>(this, &Tooltip::UnmapTooltip));
}

void Tooltip::DrawTooltip()
{
  XSetForeground(display, gc, TooltipStringColor.pixel);
  XmbDrawString(display, m_win, *m_fs, gc,
		XMARGIN - m_rc.x, YMARGIN - m_rc.y, m_str, strlen(m_str));
}

void Tooltip::SetTimer()
{
  timer->SetTimeout(TooltipDelayTime,
		    new Callback<Tooltip>(this, &Tooltip::MapTooltip));
}

void Tooltip::ResetTimer()
{
  Bool hit;

  hit = timer->ResetTimeout(new Callback<Tooltip>(this, &Tooltip::MapTooltip));
  if (!hit)
    return;

  timer->SetTimeout(TooltipDelayTime,
		    new Callback<Tooltip>(this, &Tooltip::MapTooltip));
}

void Tooltip::Disable()
{
  if (m_mapped)
    UnmapTooltip();
  else
    timer->ResetTimeout(new Callback<Tooltip>(this, &Tooltip::MapTooltip));
}

void Tooltip::Initialize()
{
  Tooltip::context = XUniqueContext();
}
