/*
 * system_dialog.cc
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
#include "qvwm.h"
#include "system_dialog.h"
#include "radio_button.h"
#include "radio_set.h"
#include "string_button.h"
#include "qvwmrc.h"
#include "image.h"

SystemDialog::SystemDialog(const Rect& rc)
: Dialog(rc)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  
  attributes.background_pixel = DialogColor.pixel;
  attributes.override_redirect = True;
  attributes.event_mask = ExposureMask;
  valueMask = CWBackPixel | CWOverrideRedirect | CWEventMask;

  parent = XCreateWindow(display, root,
			 rc.x - 3, rc.y - 4 - TITLE_HEIGHT / 2,
			 rc.width + 6, rc.height + 7 + TITLE_HEIGHT,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);

  attributes.background_pixel = TitlebarActiveColor.pixel;
  valueMask = CWBackPixel | CWEventMask;

  title = XCreateWindow(display, parent,
			3, 3, rc.width, TITLE_HEIGHT,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  if (GradTitlebar && rc.width > 1) {
    Pixmap pixGrad = CreateGradPixmap(gradActivePattern, rc.width, title);
    XSetWindowBackgroundPixmap(display, title, pixGrad);
    XFreePixmap(display, pixGrad);
  }

  XReparentWindow(display, frame, parent, 3, 4 + TITLE_HEIGHT);

  for (int i = 0; i < 256; i++)
    scKeyTable[i] = NO_ID;

  sbNum = rsNum = stNum = ipNum = 0;
}

SystemDialog::~SystemDialog()
{
  int i;

  for (i = 0; i < sbNum; i++)
    delete sb[i];
  if (sbNum > 0)
    delete [] sb;

  for (i = 0; i < rsNum; i++)
    delete rs[i];
  if (rsNum > 0)
    delete [] rs;

  for (i = 0; i < stNum; i++)
    delete st[i];
  if (stNum > 0)
    delete [] st;

  for (i = 0; i < ipNum; i++)
    delete ip[i];
  if (ipNum > 0)
    delete [] ip;

  XUnmapWindow(display, frame);
  XReparentWindow(display, frame, root, 0, 0);
  XDestroyWindow(display, parent);
}

void SystemDialog::SetRect(const Rect& rect)
{
  Dialog::SetRect(Rect(3, 4 + TITLE_HEIGHT, rect.width, rect.height));

  XMoveResizeWindow(display, parent,
		    rect.x - 3, rect.y - 4 - TITLE_HEIGHT / 2,
		    rect.width + 6, rect.height + 7 + TITLE_HEIGHT);

  XMoveResizeWindow(display, title,
		    3, 3, rect.width, TITLE_HEIGHT);

  if (GradTitlebar) {
    Pixmap pixGrad = CreateGradPixmap(gradActivePattern, rect.width, title);
    XSetWindowBackgroundPixmap(display, title, pixGrad);
    XFreePixmap(display, pixGrad);
  }
}

/*
 * CreateDialogResource --
 *   Create resources for system dialog.
 */
void SystemDialog::CreateDialogResource(DialogRes** dr, int drNum)
{
  RadioButton** rb = NULL;
  int i, nRb = 0, nSb = 0, nRs = 0, nSt = 0, nIp = 0;

  for (i = 0; i < drNum; i++) {
    ASSERT(dr[i]);
    switch (dr[i]->kind) {
    case STRINGBUTTON:
      sbNum++;
      break;

    case RADIOSET:
      rsNum++;
      break;

    case STATICTEXT:
      stNum++;
      break;

    case ICONPIXMAP:
      ipNum++;
      break;

    default:
      break;
    }
  }

  if (sbNum > 0)
    sb = new StringButton*[sbNum];
  if (rsNum > 0)
    rs = new RadioSet*[rsNum];
  if (stNum > 0)
    st = new StaticText*[stNum];
  if (ipNum > 0)
    ip = new IconPixmap*[ipNum];

  if (drNum > 0)
    rb = new RadioButton*[drNum];

  for (i = 0; i < drNum; i++) {
    ASSERT(dr[i]);
    switch (dr[i]->kind) {
    case STRINGBUTTON:
      ASSERT(nSb < sbNum);
      sb[nSb] = new StringButton(frame, dr[i]->rc, dr[i]->str, dr[i]->fs,
				   dr[i]->rid);
      SetShortcutKeyTable(dr[i]->str, dr[i]->rid);

      if (DialogImage) {
	sb[nSb]->SetBgImage(imgDialog, Point(dr[i]->rc.x, dr[i]->rc.y));
	sb[nSb]->SetBgActiveImage(imgDialog,
				  Point(dr[i]->rc.x, dr[i]->rc.y - 1));
      }

      nSb++;
      break;
      
    case RADIOBUTTON:
      ASSERT(nRb < drNum);
      rb[nRb++] = new RadioButton(frame, Point(dr[i]->rc.x, dr[i]->rc.y),
				  dr[i]->str, dr[i]->fs, dr[i]->rid);
      break;

    case RADIOSET:
      ASSERT(nRs < rsNum);
      rs[nRs++] = new RadioSet(rb, nRb, dr[i]->rid, dr[i]->initId);
      nRb = 0;
      break;

    case STATICTEXT:
      ASSERT(nSt < stNum);
      st[nSt++] = new StaticText(Point(dr[i]->rc.x, dr[i]->rc.y), dr[i]->str,
				 dr[i]->fs);
      break;

    case ICONPIXMAP:
      ASSERT(nIp < ipNum);
      ip[nIp++] = new IconPixmap(dr[i]->img,
				 Point(dr[i]->rc.x, dr[i]->rc.y));
      break;
    }
  }
}

void SystemDialog::MapDialog()
{
  int i;

  XMapRaised(display, parent);
  XMapWindow(display, title);

  Dialog::MapDialog();

  for (i = 0; i < sbNum; i++) {
    ASSERT(sb[i]);
    sb[i]->MapButton();
  }
  for (i = 0; i < rsNum; i++) {
    ASSERT(rs[i]);
    rs[i]->MapRadioButtons();
  }
}

void SystemDialog::UnmapDialog()
{
  Dialog::UnmapDialog();

  XUnmapWindow(display, parent);
}

/*
 * DrawDialog --
 *   Draw the dialog frame.
 */
void SystemDialog::DrawDialog()
{
  XPoint xp[3];
  int width, height;

  width = rc.width + 6;
  height = rc.height + 7 + TITLE_HEIGHT;

  xp[0].x = width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = height - 2;

  XSetForeground(display, ::gc, gray.pixel);
  XDrawLines(display, parent, ::gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = width - 1;
  xp[0].y = 0;
  xp[1].x = width - 1;
  xp[1].y = height - 1;
  xp[2].x = 0;
  xp[2].y = height - 1;

  XSetForeground(display, ::gc, darkGrey.pixel);
  XDrawLines(display, parent, ::gc, xp, 3, CoordModeOrigin);

  xp[0].x = width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = height - 3;

  XSetForeground(display, ::gc, white.pixel);
  XDrawLines(display, parent, ::gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = width - 2;
  xp[0].y = 1;
  xp[1].x = width - 2;
  xp[1].y = height - 2;
  xp[2].x = 1;
  xp[2].y = height - 2;

  XSetForeground(display, ::gc, darkGray.pixel);
  XDrawLines(display, parent, ::gc, xp, 3, CoordModeOrigin);
}  

/*
 * DrawTitle --
 *   Draw the title.
 */
void SystemDialog::DrawTitle()
{
  XRectangle ink, log;
  Point ptTitle;

  XClearWindow(display, title);

  XmbTextExtents(fsTitle, name, strlen(name), &ink, &log);
  ptTitle.x = 4;
  ptTitle.y = (TITLE_HEIGHT-log.height)/2 - log.y;
  
  XSetForeground(display, ::gc, white.pixel);
  XmbDrawString(display, title, fsTitle, ::gc, ptTitle.x, ptTitle.y,
		name, strlen(name));
}  

/*
 * GetSelectableRB --
 *   Get selectable radio set.
 */
ResourceId SystemDialog::GetSelectedRB(ResourceId idRadioSet)
{
  for (int i = 0; i < rsNum; i++) {
    ASSERT(rs[i]);
    if (rs[i]->GetResId() == idRadioSet)
      return rs[i]->GetFocusRB();
  }

  return NO_ID;
}

/*
 * DrawClientWin --
 *   Draw client window.
 */
void SystemDialog::DrawClientWin()
{
  int i;
  XRectangle ink, log;

  /*
   * Draw texts.
   */
  for (i = 0; i < stNum; i++) {
    ASSERT(st[i]);
    XmbTextExtents(st[i]->fs, st[i]->text, strlen(st[i]->text), &ink, &log);
    XSetForeground(display, ::gc, DialogStringColor.pixel);
    XmbDrawString(display, frame, st[i]->fs, ::gc, st[i]->pt.x,
		  st[i]->pt.y-log.y, st[i]->text, strlen(st[i]->text));
  }

  /*
   * Draw icon pixmaps.
   */
  for (i = 0; i < ipNum; i++) {
    ASSERT(ip[i]);
    ip[i]->img->Display(frame, ip[i]->pt);
  }
}

/*
 * Exposure --
 *   Process expose event.
 */
void SystemDialog::Exposure(Window win)
{
  StringButton* sb;
  RadioButton* rb;

  if (XFindContext(display, win, Button::context, (caddr_t*)&sb) == XCSUCCESS)
    sb->DrawButton();
  else if (XFindContext(display, win, RadioButton::context, (caddr_t*)&rb)
                                                                 == XCSUCCESS)
    rb->DrawButton(True);
  else {
    if (win == parent)
      DrawDialog();
    else if (win == title)
      DrawTitle();
    else if (win == frame)
      DrawClientWin();
  }
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
ResourceId SystemDialog::Button1Press(Window win)
{
  StringButton* sb;
  RadioButton* rb;

  if (XFindContext(display, win, Button::context, (caddr_t*)&sb)
                                                               == XCSUCCESS) {
    if (sb->GetParent() == frame) {
      sb->Button1Press();
      if (sb->IsTriggered())
	return sb->GetResId();
    }
  }
  else if (XFindContext(display, win, RadioButton::context, (caddr_t*)&rb)
	                                                        == XCSUCCESS)
    rb->Button1Press();
  else
    XAllowEvents(display, ReplayPointer, CurrentTime);

  return NO_ID;
}

ResourceId SystemDialog::FindShortCutKey(char key)
{
  ResourceId id;

  if (key >= 'a' && key <= 'z')
    key -= 0x20;

  id = scKeyTable[key];

  return id;
}

void SystemDialog::SetShortcutKeyTable(char* str, ResourceId id)
{
  char* keyStr = strstr(str, "\\&");
  char key;

  if (keyStr) {
    key = (unsigned char)*(keyStr + 2);
    if (key >= 'a' && key <= 'z')
      key -= 0x20;
    scKeyTable[key] = id;
  }
}
