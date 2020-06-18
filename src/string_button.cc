/*
 * string_button.cc
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
#include "resource.h"
#include "string_button.h"
#include "qvwmrc.h"
#include "util.h"

StringButton::StringButton(Window parent, const Rect& rc, char* label,
			   XFontSet& labelfs, ResourceId res_id)
: Button(parent, rc), str(label), fs(labelfs), rid(res_id)
{
  triggered = False;
}

/*
 * DrawButton --
 *   Draw button according to state.
 */
void StringButton::DrawButton()
{
  Point pt;
  XRectangle ink, log;
  int width;

  XmbTextExtents(fs, str, strlen(str), &ink, &log);
  width = GetRealWidth(fs, str);
  pt.x = (rc.width - width) / 2 - log.x;
  pt.y = (rc.height - log.height) / 2 - log.y;

  if (CheckState(PUSH)) {
    if (imgBack)
      imgBack->SetBackground(None);

    if (imgActiveBack)
      imgActiveBack->SetBackground(frame);
    else
      XSetBackground(display, ::gc, ButtonColor.pixel);
  }
  else {
    if (imgActiveBack)
      imgActiveBack->SetBackground(None);

    if (imgBack)
      imgBack->SetBackground(frame);
    else
      XSetBackground(display, ::gc, ButtonColor.pixel);
  }

  XClearWindow(display, frame);

  XSetForeground(display, ::gc, ButtonStringColor.pixel);

  if (state == PUSH)
    DrawRealString(frame, fs, ::gc, pt.x, pt.y + 1, str);
  else
    DrawRealString(frame, fs, ::gc, pt.x, pt.y, str);

  Button::DrawButton();
}

/*
 * ExecButtonFunc --
 *   Execute button function.
 */
void StringButton::ExecButtonFunc(ButtonState bs)
{
  if (bs == PUSH)
    triggered = True;
  else
    triggered = False;
}
