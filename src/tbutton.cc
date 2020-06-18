/*
 * tbutton.cc
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
#include "main.h"
#include "misc.h"
#include "util.h"
#include "qvwm.h"
#include "tbutton.h"
#include "sbutton.h"
#include "taskbar.h"
#include "startmenu.h"
#include "qvwmrc.h"
#include "paging.h"
#include "desktop.h"
#include "pixmap_image.h"
#include "tooltip.h"
#include "bitmaps/tile0.xpm"

Rect TaskbarButton::rcTButton;
XFontSet* TaskbarButton::fsb;
QvImage* TaskbarButton::imgTile;
int TaskbarButton::BUTTON_HEIGHT;

TaskbarButton::TaskbarButton(Qvwm* qvwm, const Rect& rc, QvImage* image)
: Button(taskBar->w, rc), img(image), focus(False), qvWm(qvwm)
{
  name = shortname = NULL;

  if (qvWm != rootQvwm)
    ChangeName();

  toolTip->SetString(name, &fsTaskbar);
}

TaskbarButton::~TaskbarButton()
{
  delete [] shortname;
  delete [] name;

  QvImage::Destroy(img);
}

// image must be passed as a duplicated object
void TaskbarButton::SetImage(QvImage* image)
{
  if (img)
    QvImage::Destroy(img);

  img = image;
}

void TaskbarButton::ChangeName()
{
  XTextProperty xtp;
  char** cl = 0;
  int n;
  Status got = 0;

  delete [] name;

  if (got = XGetWMName(display, qvWm->GetWin(), &xtp)) { 
  } else {
      got = XGetWMIconName(display, qvWm->GetWin(), &xtp);
  }

  if ( got ) {
      XmbTextPropertyToTextList(display, &xtp, &cl, &n);
  }

  if (cl) {
      name = new char[strlen(cl[0]) + 1];
      strcpy(name, cl[0]);
      XFreeStringList(cl);
  }
  else {
      name = new char[9];
      strcpy(name, "Untitled");
  }

  CalcShortName();

  toolTip->SetString(name, &fsTaskbar);
}

void TaskbarButton::CalcShortName()
{
  delete [] shortname;

  shortname = GetFixName(fsTaskbar, name,
			 rc.width - TB_MARGIN * 2 - SYMBOL_SIZE - 2);
}

/*
 * MapButton --
 *   Map taskbar button.
 */
void TaskbarButton::MapButton()
{
  ASSERT(qvWm);

  if (!qvWm->CheckFlags(TRANSIENT)) {
    taskBar->RedrawAllTaskbarButtons();
    Button::MapButton();
  }
}

/*
 * UnmapButton --
 *   Unmap taskbar button.
 */
void TaskbarButton::UnmapButton()
{
  ASSERT(qvWm);

  if (!qvWm->CheckFlags(TRANSIENT)) {
    Button::UnmapButton();
    taskBar->RedrawAllTaskbarButtons();
  }
}

/*
 * DrawButton --
 *   Draw taskbar button.
 */
void TaskbarButton::DrawButton()
{
  /*
   * No button for the transient window.
   */
  if (qvWm->CheckFlags(TRANSIENT))
    return;

  FillBackground();

  Point offset;

  offset.x = TB_MARGIN;
  offset.y = TB_MARGIN + (BUTTON_HEIGHT - TB_MARGIN * 2 - SYMBOL_SIZE) / 2;

  /*
   * Draw the pixmap of the taskbar button.
   */
  if (CheckState(PUSH) & focus)
    img->Display(frame, Point(offset.x, offset.y + 1));
  else
    img->Display(frame, offset);

  // Draw the title of the taskbar button.
  DrawName();

  Button::DrawButton();
}

/*
 * FillBackground --
 *   Paint the background of taskbar button gray or tile pattern.
 */
void TaskbarButton::FillBackground()
{
  if (CheckState(PUSH) && focus) {
    if (TaskbarImage) {
      imgBack->SetBackground(None);
      imgActiveBack->SetBackground(frame);
      XClearWindow(display, frame);
    }
    else {
      imgTile->SetBackground(frame);
      XClearWindow(display, frame);

      XSetForeground(display, gc, white.pixel);
      XDrawLine(display, frame, gc, 2, 2, rc.width, 2);
    }
  }
  else {
    if (TaskbarImage) {
      imgActiveBack->SetBackground(None);
      imgBack->SetBackground(frame);
    }
    else {
      if (CheckState(PUSH))
	XSetWindowBackground(display, frame, ButtonActiveColor.pixel);
      else
	XSetWindowBackground(display, frame, ButtonColor.pixel);
    }

    XClearWindow(display, frame);
  }
}

/*
 * DrawName --
 *   Draw the text taskbar button.
 */
void TaskbarButton::DrawName()
{
  ASSERT(qvWm);

  if (qvWm->CheckFlags(TRANSIENT))
    return;

  XRectangle ink, log;
  int y;

  XmbTextExtents(fsTaskbar, shortname, strlen(shortname), &ink, &log);
  y = (BUTTON_HEIGHT - log.height) / 2 - log.y;

  if (CheckState(PUSH) & focus) {
    if (UseBoldFont) {
      XSetForeground(display, gc, ButtonStringActiveColor.pixel);
      XmbDrawString(display, frame, *fsb, gc,
		    TB_MARGIN + SYMBOL_SIZE + 2, y + 1,
		    shortname, strlen(shortname));
    }
    else {
      XSetForeground(display, gc, ButtonStringActiveColor.pixel);
      XmbDrawString(display, frame, fsTaskbar, gc,
		    TB_MARGIN + SYMBOL_SIZE + 2, y + 1,
		    shortname, strlen(shortname));
      XmbDrawString(display, frame, fsTaskbar, gc,
		    TB_MARGIN + SYMBOL_SIZE + 3, y + 1,
		    shortname, strlen(shortname));
    }
    if (strlen(name) > strlen(shortname)) {
      XRectangle xr[3] =
	{{TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 2, y - 1, 2, 2},
         {TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 5, y - 1, 2, 2},
         {TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 8, y - 1, 2, 2}};

      XFillRectangles(display, frame, gc, xr, 3);
    }
  }
  else {
    XSetForeground(display, gc, ButtonStringColor.pixel);
    XmbDrawString(display, frame, fsTaskbar, gc,
		  TB_MARGIN + SYMBOL_SIZE + 2, y,
		  shortname, strlen(shortname));

    if (strlen(name) > strlen(shortname)) {
      XRectangle xr[3] =
	{{TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 2, y - 2, 1, 2},
	 {TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 5, y - 2, 1, 2},
	 {TB_MARGIN + SYMBOL_SIZE + 2 + log.width + 8, y - 2, 1, 2}};

      XFillRectangles(display, frame, gc, xr, 3);
    }
  }
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
void TaskbarButton::Button1Press()
{
  Menu::UnmapAllMenus(False);
  
  Button::Button1Press();
}

/*
 * ExecButtonFunc --
 *   Give the focus to the taskbar button and the corresponding window.
 */
void TaskbarButton::ExecButtonFunc(ButtonState bs)
{
  if (bs == PUSH) {
    ASSERT(qvWm);

    if (focus) {
      if (RestoreMinimize) {
	qvWm->MinimizeWindow();
	return;
      }
    }
    else {
      if (qvWm->CheckStatus(MINIMIZE_WINDOW))
	qvWm->RestoreWindow();
      else {
	if (!qvWm->CheckFocus() && !qvWm->CheckFlags(NO_FOCUS))
	  qvWm->SetFocus();
	qvWm->RaiseWindow(True);
	qvWm->AdjustPage();
      }
    }

    /*
     * When button is released, state is set to NORMAL in default.
     * So state sets to PUSH here.
     */
    if (qvWm->CheckFlags(NO_FOCUS))
      state = NORMAL;
    else
      state = PUSH;
  }
}
  
/*
 * Button3Release --
 *   Process the release of button3(mouse right button).
 */
void TaskbarButton::Button3Release(const Point& ptRoot)
{
  Point ptNew;
  Bool noFocus = False;
  int dir;

  /*
   * When button3 is released, map menu if start menu isn't mapped.
   */
  if (startMenu == NULL || !startMenu->CheckMapped()) {
    ASSERT(Qvwm::focusQvwm);

    if (Qvwm::focusQvwm->CheckMenuMapped())
      Qvwm::focusQvwm->UnmapMenu();

    ASSERT(qvWm);
    /*
     * Temporarily, give the focus.
     */
    if (qvWm->CheckFlags(NO_FOCUS)) {
      qvWm->ResetFlags(NO_FOCUS);
      noFocus = True;
    }
    
    if (!qvWm->CheckFocus())
      qvWm->SetFocus();
    qvWm->RaiseWindow(True);
    qvWm->AdjustPage();
    
    ASSERT(qvWm->ctrlMenu);

    qvWm->ctrlMenu->SetQvwm(qvWm);
    ptNew = qvWm->ctrlMenu->GetFixedMenuPos(ptRoot, dir);
    qvWm->ctrlMenu->MapMenu(ptNew.x, ptNew.y, dir);

    if (noFocus)
      qvWm->SetFlags(NO_FOCUS);
  }
}

void TaskbarButton::Enter()
{
  // if a start menu or abbreviated by ...
  if (shortname == NULL || strcmp(name, shortname) != 0)
    Button::Enter();
}

void TaskbarButton::MoveResizeButton(const Rect& rect)
{
  rc = rect;

  XMoveResizeWindow(display, frame, rc.x, rc.y, rc.width, rc.height);

  CalcShortName();
}

void TaskbarButton::Initialize()
{
  fsb = UseBoldFont ? &fsBoldTaskbar : &fsTaskbar;

  BUTTON_HEIGHT = TaskbarButtonHeight;
  rcTButton = Rect(0, 0, 160, TaskbarButton::BUTTON_HEIGHT);

  imgTile = new PixmapImage(tile0);
}
