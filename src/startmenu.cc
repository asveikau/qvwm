/*
 * startmenu.cc
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
#include "util.h"
#include "menu.h"
#include "startmenu.h"
#include "taskbar.h"
#include "tbutton.h"
#include "qvwmrc.h"
#include "pixmap_image.h"
#include "callback.h"
#include "timer.h"
#include "bitmaps/folder32.xpm"
#include "bitmaps/icon32.xpm"
#include "bitmaps/logo_qvwm11.xpm"

QvImage* StartMenu::imgLogoMark;
QvImage* StartMenu::imgStart[2];

StartMenu::StartMenu(MenuElem* mItem)
: Menu(mItem, fsStartMenu, NULL, rootQvwm,
       5, 2, 2, imgStart[0], imgStart[1])
{

  XSetWindowAttributes attributes;
  unsigned long valueMask;

  if ( imgLogoMark ) {
    leftMargin += imgLogoMark->GetSize().width;

    Dim szLogo = imgLogoMark->GetSize();

    attributes.background_pixel = StartMenuLogoColor.pixel;
    valueMask = CWBackPixel;
    
    logo = XCreateWindow(display, frame,
                         MenuFrameWidth, MenuFrameWidth,
                         szLogo.width, rc.height - MenuFrameWidth * 2,
                         0, CopyFromParent, InputOutput,
                         CopyFromParent, valueMask, &attributes);
    XSaveContext(display, logo, context, (caddr_t)this);

  } else {
    logo = NULL;
  }


  /*
   * Report events even on the logo.
   */
  Rect rect;
  int sep = 0;

  itemFocus = new Window[nitems];

  for (int i = 0; i < nitems; i++) {
    if (func[i] != Q_SEPARATOR) {
      attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
	                      EnterWindowMask |
			      OwnerGrabButtonMask | ExposureMask | 
			      PointerMotionMask;
      rect.y = MenuFrameWidth + (i - sep) * itemHeight + sep * SeparatorHeight;
      rect.height = itemHeight;
    }
    else {
      attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
		     	      OwnerGrabButtonMask | ExposureMask;
      rect.y = MenuFrameWidth + (i - sep) * itemHeight + sep * SeparatorHeight;
      rect.height = SeparatorHeight;
      sep++;
    }

    rect.x = MenuFrameWidth;
    rect.width = rc.width - MenuFrameWidth * 2;

    /*
     * Create transparent wrapper window on logo and menu item.
     */
    itemFocus[i] = XCreateWindow(display, frame,
				 rect.x, rect.y, rect.width, rect.height,
				 0, CopyFromParent, InputOnly, CopyFromParent,
				 CWEventMask, &attributes);
    XSaveContext(display, itemFocus[i], context, (caddr_t)this);
  }

  XResizeWindow(display, frame, rc.width, rc.height);
}

StartMenu::~StartMenu()
{

  if ( logo ) {
    XDeleteContext(display, logo, context);
    XDestroyWindow(display, logo);
  }

  for (int i = 0; i < nitems; i++) {
    XDeleteContext(display, itemFocus[i], context);
    XDestroyWindow(display, itemFocus[i]);
  }
}

/*
 * MapMenu --
 *   Map start menu according to taskbar position.
 */
void StartMenu::MapMenu()
{
  Rect rect;
  int dir;

  // XXX -- also show taskbar if it is disabled
  if (UseTaskbar) 
    if (TaskbarAutoHide && taskBar->IsHiding()) 
      taskBar->ShowTaskbar();

  ASSERT(rootQvwm);
  ASSERT(rootQvwm->tButton);
  
  rootQvwm->tButton->SetState(Button::PUSH);
  rootQvwm->SetFocus();
  rootQvwm->tButton->DrawButton();

  if (UseTaskbar) {
    switch (taskBar->GetPos()) {
    case Taskbar::BOTTOM:
      rect = taskBar->GetRect();
      rc.x = 2;
      rc.y = rect.y - rc.height + 4;
      fDir = RIGHT;
      dir = GD_UP;
      break;
      
    case Taskbar::TOP:
      rect = taskBar->GetRect();
      rc.x = 2;
      rc.y = rect.height-4 - Taskbar::INC_HEIGHT *
	((rect.height - Taskbar::BASE_HEIGHT) / Taskbar::INC_HEIGHT);
      fDir = RIGHT;
      dir = GD_DOWN;
      break;
      
    case Taskbar::LEFT:
      rc.x = 2;
      rc.y = TaskbarButton::BUTTON_HEIGHT + 2;
      fDir = RIGHT;
      dir = GD_RIGHT;
      break;
      
    case Taskbar::RIGHT:
      {
	Rect rcRoot = rootQvwm->GetRect();
	
	rc.x = rcRoot.width - rc.width - 2;
	rc.y = TaskbarButton::BUTTON_HEIGHT + 2;
	fDir = LEFT;
	dir = GD_LEFT;
	break;
      }

    default:
      dir = 0;  // XXX for warning
      ASSERT(False);
    }
  }
  else {
    Rect rcRoot = rootQvwm->GetRect();

    rc.x = 2;
    rc.y = rcRoot.height - rc.height - 2;
    dir = GD_UP;
  }
    
  Menu::MapMenu(rc.x, rc.y, dir);

  if ( logo ) 
    XRaiseWindow(display, logo);

  for (int i = 0; i < nitems; i++)
    XRaiseWindow(display, itemFocus[i]);

  XSelectInput(display, rootQvwm->tButton->GetFrameWin(), 
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       Button1MotionMask);
}

/*
 * UnmapMenu --
 *   Unmap start menu.
 */
void StartMenu::UnmapMenu()
{
  Menu::UnmapMenu();

  ASSERT(rootQvwm);
  ASSERT(rootQvwm->tButton);

  rootQvwm->tButton->SetState(Button::NORMAL);
  rootQvwm->tButton->DrawButton();

  XSelectInput(display, rootQvwm->tButton->GetFrameWin(),
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       Button1MotionMask | OwnerGrabButtonMask);

  if (UseTaskbar && TaskbarAutoHide) {
    Window junkRoot, junkChild;
    Point ptRoot, ptChild;
    unsigned int mask;

    XQueryPointer(display, root, &junkRoot, &junkChild,
		  &ptRoot.x, &ptRoot.y, &ptChild.x, &ptChild.y, &mask);
    if (InRect(ptRoot, taskBar->GetScreenRectOnShowing())) {
      BasicCallback* cb;
      cb = new Callback<Taskbar>(taskBar, &Taskbar::HideTaskbar);
      timer->SetTimeout(TaskbarHideDelay, cb);
    }
  }
}

/*
 * DrawMenu --
 *   Draw start menu.
 */
void StartMenu::DrawMenu(Window win)
{
  Menu::DrawMenu(win);

  if ( logo ) {
    Dim szLogo = imgLogoMark->GetSize();
    Point offset(0, rc.height - szLogo.height - MenuFrameWidth * 2);
  
    imgLogoMark->Display(logo, offset);
  }
}

/*
 * FindItem --
 *   Find the menu item corresponding to win.
 */
int StartMenu::FindItem(Window win)
{
  for (int i = 0; i < nitems; i++)
    if (win == item[i] || win == itemFocus[i])
      return i;

  return -1;
}

void StartMenu::ExecFunction(FuncNumber fn, int num)
{
  if (fn == Q_DIR) {
    ASSERT(num >= 0 && num < nitems);

    // delayed menu creation
    if (next[num] == NULL) {
      ASSERT(childItem[num]);
      next[num] = new Menu(childItem[num], fsCascadeMenu, this, qvWm,
			   5, 3, 6, imgMenu[0], imgMenu[1]);
    }
  }

  Menu::ExecFunction(fn, num);
}

void StartMenu::Initialize()
{

  if ( DefaultFolderIcon ) {
    imgStart[0] = CreateImageFromFile( DefaultFolderIcon, timer);
  } else {
    imgStart[0] = new PixmapImage(folder32);
  }

  if ( DefaultMenuItemIcon ) {
    imgStart[1] = CreateImageFromFile( DefaultMenuItemIcon, timer);
  } else {
    imgStart[1] = NULL;
  }

  if (StartMenuLogoImage) {
    imgLogoMark = CreateImageFromFile(StartMenuLogoImage, timer);
  } else {
    imgLogoMark = NULL;
  }
}
