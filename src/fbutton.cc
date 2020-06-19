/*
 * fbutton.cc
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
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "fbutton.h"
#include "qvwmrc.h"
#include "pixmap_image.h"
#include "tooltip.h"
#include "bitmaps/button_min.xpm"
#include "bitmaps/button_max.xpm"
#include "bitmaps/button_close.xpm"
#include "bitmaps/button_restore.xpm"

const char* FrameButton::desc[4];

QvImage* FrameButton::imgButton[4];

FrameButton::FrameButton(Qvwm* qvwm, Window parent, const Rect& rc)
: Button(parent, rc), img(NULL), qvWm(qvwm)
{
  if (TitlebarImage) {
    SetBgImage(Qvwm::imgTitlebar, Point(rc.x, rc.y));
    SetBgActiveImage(Qvwm::imgActiveTitlebar, Point(rc.x, rc.y));
  }
}

FrameButton::~FrameButton()
{
  QvImage::Destroy(img);
}

void FrameButton::ChangeImage(ButtonName bn)
{
  QvImage::Destroy(img);
  img = imgButton[bn]->Duplicate();
  toolTip->SetString(desc[bn], &fsTitle);
}

/*
 * DrawButton --
 *   Draw frame button.
 */
void FrameButton::DrawButton()
{
  if (qvWm->CheckFocus()) {
    if (TitlebarImage)
      imgBack->SetBackground(None);

    if (TitlebarActiveImage)
      imgActiveBack->SetBackground(frame);
    else
      XSetWindowBackground(display, frame, ButtonActiveColor.pixel);
  }
  else {
    if (TitlebarActiveImage)
      imgActiveBack->SetBackground(None);

    if (TitlebarImage)
      imgBack->SetBackground(frame);
    else
      XSetWindowBackground(display, frame, ButtonColor.pixel);
  }

  XClearWindow(display, frame);

  Button::DrawButton();

  if (img) {
    if (state == NORMAL)
      img->Display(frame, Point(2, 2));
    else
      img->Display(frame, Point(3, 3));
  }
}

/*
 * Button1Press --
 *   Process press of button1 (mouse left button)
 */
void FrameButton::Button1Press()
{
  ASSERT(qvWm);

  if (!qvWm->CheckFocus())
    qvWm->SetFocus();

  qvWm->RaiseWindow(True);

  Button::Button1Press();
}

/*
 * ExecButtonFunc --
 *   Execute the function of frame button 1 (minimize).
 */
void FrameButton1::ExecButtonFunc(ButtonState bs)
{
  ASSERT(qvWm);
  
  if (bs == PUSH)
    qvWm->MinimizeWindow();
}

/*
 * ExecButtonFunc --
 *   Execute the function of frame button 2 (maximize or restore).
 */
void FrameButton2::ExecButtonFunc(ButtonState bs)
{
  ASSERT(qvWm);

  if (bs == PUSH) {
    if (qvWm->CheckStatus(MAXIMIZE_WINDOW)) {
      ChangeImage(MAXIMIZE);
      qvWm->RestoreWindow();
    }
    else {
      ChangeImage(RESTORE);
      qvWm->MaximizeWindow();
    }
  }
}

/*
 * ExecButtonFunc --
 *   Execute the function of frame button 3 (close window).
 */
void FrameButton3::ExecButtonFunc(ButtonState bs)
{
  ASSERT(qvWm);

  if (bs == PUSH)
    QvFunction::execFunction(Q_CLOSE, qvWm);
}

void FrameButton::Initialize()
{
  imgButton[MINIMIZE] = new PixmapImage(button_min);
  imgButton[MAXIMIZE] = new PixmapImage(button_max);
  imgButton[CLOSE] = new PixmapImage(button_close);
  imgButton[RESTORE] = new PixmapImage(button_restore);

  desc[MINIMIZE] = MinimizeButtonMessage;
  desc[MAXIMIZE] = MaximizeButtonMessage;
  desc[CLOSE] = CloseButtonMessage;
  desc[RESTORE] = RestoreButtonMessage;
}
