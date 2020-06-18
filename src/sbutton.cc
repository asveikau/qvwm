/*
 * sbutton.cc
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
#include "qvwm.h"
#include "sbutton.h"
#include "startmenu.h"
#include "qvwmrc.h"
#include "image.h"
#include "callback.h"
#include "tooltip.h"

StartButton::StartButton(Qvwm* qvWm, int some_width)
: TaskbarButton(qvWm, Rect(rcTButton.x, rcTButton.y,
			   some_width, TaskbarButton::BUTTON_HEIGHT), imgLogo)

{
  name = new char[strlen(StartButtonTitle) + 1];
  strcpy(name, StartButtonTitle);

  buttonWidth = some_width;

  XSelectInput(display, frame,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       Button1MotionMask | PointerMotionMask | EnterWindowMask |
	       LeaveWindowMask | OwnerGrabButtonMask);

  imgLogo->SetPreCallback(new Callback<StartButton>(this,
						    &StartButton::DrawActive));

  toolTip->SetString(StartButtonMessage, &fsTaskbar);
}

void StartButton::DrawButton()
{
  TaskbarButton::DrawButton();

  DrawActive();
}

void StartButton::DrawActive()
{
  if (CheckState(PUSH)) {
    XSetForeground(display, gcDash, black.pixel);
    XDrawRectangle(display, frame, gcDash, 3, 3, rc.width - 7, rc.height - 7);
  }
}

void StartButton::FillBackground()
{
  if (CheckState(PUSH)) {
    if (TaskbarImage) {
      imgBack->SetBackground(None);
      imgActiveBack->SetBackground(frame);
    }
  }
  else {
    if (TaskbarImage) {
      imgActiveBack->SetBackground(None);
      imgBack->SetBackground(frame);
    }
  }

  XClearWindow(display, frame);
}

/*
 * DrawName --
 *   Don't adjust the text in start button even if it is too long.
 */
void StartButton::DrawName()
{
  int y;
  XRectangle ink, log;

  XmbTextExtents(fsTaskbar, name, strlen(name), &ink, &log);
  y = (TaskbarButton::BUTTON_HEIGHT - log.height) / 2 - log.y;

  if (CheckState(PUSH)) {
    if (UseBoldFont) {
      XSetForeground(display, ::gc, ButtonStringActiveColor.pixel);
      XmbDrawString(display, frame, *fsb, ::gc, 22, y+1, name, strlen(name));
    }
    else {
      XSetForeground(display, ::gc, ButtonStringActiveColor.pixel);
      XmbDrawString(display, frame, fsTaskbar, ::gc, 22, y+1,
		    name, strlen(name));
      XmbDrawString(display, frame, fsTaskbar, ::gc, 23, y+1,
		    name, strlen(name));
    }
  }
  else {
    if (UseBoldFont) {
      XSetForeground(display, ::gc, ButtonStringColor.pixel);
      XmbDrawString(display, frame, *fsb, ::gc, 22, y, name, strlen(name));
    }
    else {
      XSetForeground(display, ::gc, ButtonStringColor.pixel);
      XmbDrawString(display, frame, fsTaskbar, ::gc, 22, y,
		    name, strlen(name));
      XmbDrawString(display, frame, fsTaskbar, ::gc, 23, y,
		    name, strlen(name));
    }
  }
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
void StartButton::Button1Press()
{
  toolTip->Disable();

  /*
   * Create start button if it isn't still created. The creation of start
   * button is delayed until it is needed.
   */
  if (startMenu == NULL)
    startMenu = new StartMenu(StartMenuItem);

  if (startMenu->CheckMapped())
    startMenu->UnmapMenu();
  else {
    Menu::UnmapAllMenus(False);
    startMenu->MapMenu();
  }
}

void StartButton::Enter()
{
  // XXX turn around TaskbarButton::Enter
  Button::Enter();
}
