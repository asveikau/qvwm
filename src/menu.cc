/*
 * menu.cc
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
#include <unistd.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "menu.h"
#include "util.h"
#include "startmenu.h"
#include "qvwmrc.h"
#include "timer.h"
#include "key.h"
#include "icon.h"
#include "callback.h"
#include "taskbar.h"
#include "paging.h"
#include "pixmap_image.h"
#include "bitmaps/folder16.xpm"
#include "bitmaps/icon16.xpm"
#include "bitmaps/next_black.xpm"
#include "bitmaps/next_white.xpm"
#include "bitmaps/check_black.xpm"
#include "bitmaps/check_white.xpm"
#include "bitmaps/selector_black.xpm"
#include "bitmaps/selector_white.xpm"

XContext	Menu::context;
Menu::MenuDir	Menu::fDir = Menu::RIGHT;
QvImage*	Menu::imgMenu[2];
QvImage*	Menu::imgNext[2];
QvImage*	Menu::imgCheck[2];
QvImage*	Menu::imgSelector[2];
QvImage 	*Menu::imgMenuBack, *Menu::imgActiveMenuBack;
Bool JustMapped;

Menu::Menu(MenuElem* mItem, XFontSet& menufs, Menu* par, Qvwm* qvwm,
	   int lMargin, int nMargin, int hMargin,
	   QvImage* imgDefFolder, QvImage* imgDefIcon)
: parent(par), leftMargin(lMargin), nameMargin(nMargin), hiMargin(hMargin),
  fs(menufs), qvWm(qvwm)
{
  int maxWidth = 0, width;
  Dim maxImgSize(0, 0);
  int i, sep = 0;
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  Rect rect;

  QvImage* defaultItemIcon;

  iFocus = -1;            // no focus
  mapped = False;
  child = NULL;

  nitems = GetMenuItemNum(mItem);

  item = new Window[nitems];
  img = new QvImage*[nitems];
  name = new const char*[nitems];
  scKey = new char[nitems];
  func = new FuncNumber[nitems];
  exec = new const char*[nitems];
  next = new Menu*[nitems];
  childItem = new MenuElem*[nitems];

  imgWhiteNext = new QvImage*[nitems];
  imgBlackNext = new QvImage*[nitems];
  imgWhiteCheck = new QvImage*[nitems];
  imgBlackCheck = new QvImage*[nitems];
  imgWhiteSelector = new QvImage*[nitems];
  imgBlackSelector = new QvImage*[nitems];

  if (MenuImage)
    imgBack = new QvImage*[nitems];
  if (MenuActiveImage)
    imgActiveBack = new QvImage*[nitems];

  for (i = 0; i < nitems; i++) {
    ASSERT(mItem);

    func[i] = mItem->func;
    exec[i] = mItem->exec;
    name[i] = mItem->name;
    
    const char* keyStr = strstr(name[i], "\\&");
    if (keyStr) {
      scKey[i] = *(keyStr + 2);
      if (scKey[i] >= 'a' && scKey[i] <= 'z')
	scKey[i] -= 0x20;
    }
    else
      scKey[i] = 0;

    if (mItem->func == Q_DIR) {
      childItem[i] = mItem->child;
      next[i] = NULL;  // menu creation is delayed
    }
    else {
      childItem[i] = NULL;
      next[i] = NULL;
    }

    // set the item's icon
    if (mItem->func == Q_DIR) {
	defaultItemIcon = imgDefFolder;
    } 
    else {
	defaultItemIcon = imgDefIcon;
    }

    if (strcmp(mItem->file, "") != 0) {
      img[i] = CreateImageFromFile(mItem->file, timer);
      if (img[i] == NULL && defaultItemIcon) {
        img[i] = defaultItemIcon->Duplicate();
      }
    }
    else {
      if (defaultItemIcon)
        img[i] = defaultItemIcon->Duplicate();
      else
        img[i] = NULL;
    }
      
    imgWhiteNext[i] = imgNext[0]->Duplicate();
    imgBlackNext[i] = imgNext[1]->Duplicate();
    imgWhiteSelector[i] = imgSelector[0]->Duplicate();
    imgBlackSelector[i] = imgSelector[1]->Duplicate();
    imgWhiteCheck[i] = imgCheck[0]->Duplicate();
    imgBlackCheck[i] = imgCheck[1]->Duplicate();

    width = GetRealWidth(fs, name[i]);
    if (width > maxWidth)
      maxWidth = width;

    if (mItem->func != Q_SEPARATOR && img[i]) {
      Dim size = img[i]->GetSize();
      if (size.width > maxImgSize.width)
	maxImgSize.width = size.width;
      if (size.height > maxImgSize.height)
	maxImgSize.height = size.height;
    }

    mItem = mItem->next;
  }

  // for inheriting default images
  imgParentFolder = imgDefFolder;
  imgParentIcon = imgDefIcon;


  // calculate space for the images (on the left)
  // instance variables imageArea and imageMaxWidth
  if (maxImgSize.width) {
    imageMaxWidth = maxImgSize.width;
  } else { // no images
    imageMaxWidth = 8;
  }      
  imageArea = leftMargin + imageMaxWidth + nameMargin;

  rc.width = imageArea + maxWidth + 24 + MenuFrameWidth * 2;

  // calculate menu item height
  itemHeight = maxImgSize.height + hiMargin;
  {
    XRectangle ink, log;
    for (i = 0; i < nitems; i++) 
      if (func[i] != Q_SEPARATOR) {
        XmbTextExtents(fs, name[i], strlen(name[i]), &ink, &log);
        if ( log.height + hiMargin > itemHeight )
          itemHeight = log.height + hiMargin;
      }

    if (itemHeight < MenuItemMinimalHeight) 
      itemHeight = MenuItemMinimalHeight;
  }


  /*
   * Create frame window.
   */
  attributes.save_under = DoesSaveUnders(ScreenOfDisplay(display, screen));
  attributes.background_pixel = MenuColor.pixel;
  attributes.event_mask = ExposureMask | KeyPressMask | LeaveWindowMask |
                          EnterWindowMask;
  valueMask = CWSaveUnder | CWBackPixel | CWEventMask;
  
  frame = XCreateWindow(display, root,
			0, 0, 1, 1,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  if (MenuImage)
    imgMenuBack->SetBackground(frame);

  valueMask = CWBackPixel | CWEventMask;


  /*
   * Create item windows for menu.
   */
  XRectangle ink, log;
  int last_y = MenuFrameWidth;
  for (i = 0; i < nitems; i++) {
    if (func[i] != Q_SEPARATOR) {
      attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
                              EnterWindowMask |
			      OwnerGrabButtonMask | ExposureMask 
                              | PointerMotionMask
                              ;
      rect.y = last_y;
      rect.height = itemHeight;
    }
    else {
      attributes.event_mask = EnterWindowMask | OwnerGrabButtonMask |
	                      ExposureMask;
      rect.y = last_y;
      rect.height = SeparatorHeight;
      sep++;
    }
    last_y += rect.height;

    rect.x = MenuFrameWidth;
    rect.width = rc.width - MenuFrameWidth * 2;

    item[i] = XCreateWindow(display, frame,
			    rect.x, rect.y, rect.width, rect.height,
			    0, CopyFromParent, InputOutput, CopyFromParent,
			    valueMask, &attributes);

    if (MenuImage) {
      imgBack[i] = imgMenuBack->GetOffsetImage(Point(rect.x, rect.y));

      if (func[i] == Q_SEPARATOR)
	imgBack[i]->SetBackground(item[i]);
    }
    if (MenuActiveImage) {
      if (func[i] == Q_SEPARATOR)
	imgActiveBack[i] = NULL;
      else {
	Point pt(rect.x, rect.y);
	imgActiveBack[i] = imgActiveMenuBack->GetOffsetImage(pt);
      }
    }
  }

  rc.height = (nitems - sep) * itemHeight + sep * SeparatorHeight
    + MenuFrameWidth * 2;

  XResizeWindow(display, frame, rc.width, rc.height);

  XSaveContext(display, frame, context, (caddr_t)this);
  for (i = 0; i < nitems; i++)
    XSaveContext(display, item[i], context, (caddr_t)this);    
}

Menu::~Menu()
{
  int i;

  if (nitems == 0)
    return;

  XDeleteContext(display, frame, context);
  XDestroyWindow(display, frame);

  for (i = 0; i < nitems; i++) {
    XDeleteContext(display, item[i], context);
    XDestroyWindow(display, item[i]);

    QvImage::Destroy(img[i]);

    if (next[i])
      delete next[i];

    if (MenuImage) {
      if (imgBack[i])
	QvImage::Destroy(imgBack[i]);
    }
    if (MenuActiveImage) {
      if (imgActiveBack[i])
	QvImage::Destroy(imgActiveBack[i]);
    }
  }

  delete [] item;
  delete [] func;
  delete [] img;
  delete [] name;
  delete [] exec;
  delete [] next;

  if (MenuImage) {
    for (i = 0; i < nitems; i++)
      QvImage::Destroy(imgBack[i]);
    delete [] imgBack;
  }
  if (MenuActiveImage) {
    for (i = 0; i < nitems; i++)
      QvImage::Destroy(imgActiveBack[i]);
    delete [] imgActiveBack;
  }

  for (i = 0; i < nitems; i++) {
    QvImage::Destroy(imgWhiteNext[i]);
    QvImage::Destroy(imgBlackNext[i]);
    QvImage::Destroy(imgWhiteSelector[i]);
    QvImage::Destroy(imgBlackSelector[i]);
    QvImage::Destroy(imgWhiteCheck[i]);
    QvImage::Destroy(imgBlackCheck[i]);
  }

  delete [] imgWhiteNext;
  delete [] imgBlackNext;
  delete [] imgWhiteSelector;
  delete [] imgBlackSelector;
  delete [] imgWhiteCheck;
  delete [] imgBlackCheck;
}

/*
 * MapMenu --
 *   Map menu.
 */
void Menu::MapMenu(int x, int y, int dir)
{
  // this is checked in Enter() and modified in PointerMotion
  // this is the initial state 
  JustMapped = True;
  
  // check the window position and adjust if necessary
  int display_width  = DisplayWidth(display, screen);
  int display_height = DisplayHeight(display, screen);
  if ( y + rc.height > display_height ) { y = display_height - rc.height; }
  if ( y < 0 ) { y = 0; }
  if ( x + rc.width > display_width ) { x = display_width - rc.width; }
  if ( x < 0 ) { x = 0; }


  PlaySound(MenuPopupSound);

  if (GradMenuMap && mapped)
    XUnmapWindow(display, frame);

  XMapSubwindows(display, frame);
  XMoveWindow(display, frame, x, y);
  if (GradMenuMap)
    XResizeWindow(display, frame, 1, 1);
  XMapRaised(display, frame);

  // make this menu show gradually
  if (GradMenuMap) {
    // child menus expand only horizontaly -- ? did I mean vertically?
    if (parent)
      dir &= ~(GD_DOWN | GD_UP);

    int divide = ((rc.width + rc.height) / 2) * GradMenuMapSpeed / 100;
    if (divide == 0)
      divide = 1;

    for (int i = 1; i <= divide; i++) {
      Dim size(rc.width * i / divide, rc.height * i / divide);
      if (size.width == 0)
	size.width = 1;
      if (size.height == 0)
	size.height = 1;

      if (dir & GD_RIGHT) {
	if (dir & GD_DOWN) {
	  // right & down
	  XResizeWindow(display, frame, size.width, size.height);
	}
	else if (dir & GD_UP) {
	  // right & up
	  XMoveResizeWindow(display, frame,
			    x, y + rc.height - size.height,
			    size.width, size.height);
	}
	else {
	  // right
	  XResizeWindow(display, frame, size.width, rc.height);
	}
      }
      else if (dir & GD_LEFT) {
	if (dir & GD_DOWN) {
	  // left & down
	  XMoveResizeWindow(display, frame,
			    x + rc.width - size.width, y,
			    size.width, size.height);
	}
	else if (dir & GD_UP) {
	  // left & up
	  XMoveResizeWindow(display, frame,
			    x + rc.width - size.width,
			    y + rc.height - size.height,
			    size.width, size.height);
	}
	else {
	  // left
	  XMoveResizeWindow(display, frame,
			    x + rc.width - size.width, y,
			    size.width, rc.height);
	}
      }
      else if (dir & GD_DOWN) {
	// down
	XResizeWindow(display, frame, rc.width, size.height);
      }
      else {
	// up
	XMoveResizeWindow(display, frame,
			  x, y + rc.height - size.height,
			  rc.width, size.height);
      }

      XFlush(display);
      usleep(10000);
    }
  }

  mapped = True;
  fDir = RIGHT;
  rc.x = x;
  rc.y = y;

  ASSERT(qvWm);

  /*
   * Take off the focus from the window when any menus are shown.
   */
  if (qvWm->CheckMapped()) {
    if (qvWm != rootQvwm)
      for (int i = 1; i < 4; i++)
	XGrabButton(display, i, 0, qvWm->GetWin(), True, ButtonPressMask,
		    GrabModeAsync, GrabModeAsync, None, cursor[0]);
  }

  XSetInputFocus(display, frame, RevertToParent, CurrentTime);

  menuKey->GrabKeys(frame);
}

/*
 * UnmapMenu --
 *   Unmap menu.
 */
void Menu::UnmapMenu()
{
  int i;

  timer->ResetTimeout(new Callback<Menu>(this, &Menu::PopupMenu));
  JustMapped = False;
  mapped = False;
  iFocus = -1;

  if (child != NULL)
    child->UnmapMenu();

  menuKey->UngrabKeys(frame);

  ASSERT(qvWm);

  /*
   * Yield the focus.
   */
  if (qvWm->CheckMapped()) {
    if (qvWm != rootQvwm)
      for (i = 1; i < 4; i++)
	XUngrabButton(display, i, 0, qvWm->GetWin());

    /*
     * Yield the focus to the parent, if any.
     */
    if (parent != NULL)
      XSetInputFocus(display, parent->frame, RevertToParent, CurrentTime);
    else
      Qvwm::SetFocusToActiveWindow();
  }

  XUnmapWindow(display, frame);

  if (parent != NULL)
    parent->child = NULL;
}

/*
 * DrawMenu --
 *   Draw menu.
 */
void Menu::DrawMenu(Window win)
{
  int num;

  if (win == frame)
    DrawFrame();
  else {
    if ((num = FindItem(win)) != -1) {
      if (func[num] == Q_SEPARATOR)
	  DrawSeparator(num);
	else
	  SetMenuFocus(num);
    }
  }
}

/*
 * DrawFrame --
 *   Draw frame.
 */
void Menu::DrawFrame()
{
  XPoint xp[3];

  xp[0].x = rc.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = rc.height - 2;

  XSetForeground(display, ::gc, gray.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 1;
  xp[0].y = 0;
  xp[1].x = rc.width - 1;
  xp[1].y = rc.height - 1;
  xp[2].x = 0;
  xp[2].y = rc.height - 1;

  XSetForeground(display, ::gc, darkGrey.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);

  xp[0].x = rc.width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = rc.height - 3;

  XSetForeground(display, ::gc, white.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 2;
  xp[0].y = 1;
  xp[1].x = rc.width - 2;
  xp[1].y = rc.height - 2;
  xp[2].x = 1;
  xp[2].y = rc.height - 2;

  XSetForeground(display, ::gc, darkGray.pixel);
  XDrawLines(display, frame, ::gc, xp, 3, CoordModeOrigin);
}

/*
 * DrawSeparator --
 *   Draw the separator.
 */
void Menu::DrawSeparator(int num)
{
  ASSERT(num >= 0 && num < nitems);

  XSetForeground(display, ::gc, darkGray.pixel);
  XDrawLine(display, item[num], ::gc, 0, 2, rc.width-1, 2);

  XSetForeground(display, ::gc, white.pixel);
  XDrawLine(display, item[num], ::gc, 0, 3, rc.width-1, 3);
}

/*
 * DrawMenuContents --
 *   Draw the contents of the menu.
 */
void Menu::DrawMenuContents(int num)
{
  XRectangle ink, log;
  int y;
  int middley;

  ASSERT(num >= 0 && num < nitems);

  /*
   * Draw the menu text.
   */
  middley = itemHeight / 2;
  XmbTextExtents(fs, name[num], strlen(name[num]), &ink, &log);
  y = (itemHeight - log.height) / 2 - log.y;

  if (num == iFocus) {
    if (IsSelectable(func[num])) {
      XSetForeground(display, gc, MenuStringActiveColor.pixel);
      DrawRealString(item[num], fs, gc, imageArea, y, name[num]);
    }
    else {
      XSetForeground(display, gc, darkGray.pixel);
      DrawRealString(item[num], fs, gc, imageArea, y, name[num]);
    }

    if (IsChecked(func[num]) && img[num] == NULL)
      imgWhiteCheck[num]->Display(item[num], Point(5, middley-3));

    if (IsSelected(func[num]) && img[num] == NULL)
      imgWhiteSelector[num]->Display(item[num], Point(4, middley-3));
    
    if (func[num] == Q_DIR)
      imgWhiteNext[num]->Display(item[num],
				 Point(rc.width - 14, middley - 4));
  }
  else {
    if (IsSelectable(func[num])) {
      XSetForeground(display, gc, MenuStringColor.pixel);
      DrawRealString(item[num], fs, gc, imageArea, y, name[num]);
    }
    else {
      XSetForeground(display, gc, white.pixel);
      DrawRealString(item[num], fs, gc, imageArea + 1, y + 1, name[num]);
      XSetForeground(display, gc, darkGray.pixel);
      DrawRealString(item[num], fs, gc, imageArea, y, name[num]);
    }

    if (IsChecked(func[num]) && img[num] == NULL)
      imgBlackCheck[num]->Display(item[num], Point(5, middley-3));

    if (IsSelected(func[num]) && img[num] == NULL)
      imgBlackSelector[num]->Display(item[num], Point(4, middley-3));
    
    if (func[num] == Q_DIR)
      imgBlackNext[num]->Display(item[num],
				 Point(rc.width - 14, middley - 4));
  }

  /*
   * Draw the menu pixmap, if neccesary.
   */
  if (img[num] && func[num] != Q_NONE) {
    Dim size = img[num]->GetSize();
    int y = (itemHeight - size.height) / 2;
    int x = (imageMaxWidth - size.width) / 2;

    img[num]->Display(item[num], Point(leftMargin+x, y));
  }
}

/*
 * SetMenuFocus --
 *   Give the focus to the menu item, and redraw.
 */
void Menu::SetMenuFocus(int num)
{
  ASSERT(num >= 0 && num < nitems);
  
  if (num == iFocus) {
    if (MenuImage)
      imgBack[num]->SetBackground(None);

    if (MenuActiveImage)
      imgActiveBack[num]->SetBackground(item[num]);
    else
      XSetWindowBackground(display, item[num], MenuActiveColor.pixel);
  }
  else {
    if (MenuActiveImage)
      imgActiveBack[num]->SetBackground(None);

    if (MenuImage)
      imgBack[num]->SetBackground(item[num]);
    else
      XSetWindowBackground(display, item[num], MenuColor.pixel);
  }

  XClearWindow(display, item[num]);

  DrawMenuContents(num);
}

/*
 * FindItem --
 *   Find the menu item corresponding to given window.
 */
int Menu::FindItem(Window win)
{
  for (int i = 0; i < nitems; i++)
    if (win == item[i])
      return i;
  
  return -1;
}

/*
 * CalcItemYPos --
 *   Calculate y position of the menu item.
 */
int Menu::CalcItemYPos(int num)
{
  int y = MenuFrameWidth;
  
  ASSERT(num < nitems);

  for (int i = 0; i < num; i++)
    if (func[i] != Q_SEPARATOR)
      y += itemHeight;
    else
      y += SeparatorHeight;

  return y;
}

/*
 * GetFixedMenuPos --
 *   Adjust the menu position.
 */
Point Menu::GetFixedMenuPos(const Point& pt, int& dir)
{
  Point ptNew;
  Rect rcRoot = rootQvwm->GetRect();

  if (pt.x + rc.width > rcRoot.width - 1) {
    ptNew.x = pt.x - rc.width;
    dir = GD_LEFT;
  }
  else {
    ptNew.x = pt.x;
    dir = GD_RIGHT;
  }
  if (pt.y + rc.height > rcRoot.height - 1) {
    ptNew.y = rcRoot.height - rc.height;
    dir |= GD_UP;
  }
  else {
    ptNew.y = pt.y;
    dir |= GD_DOWN;
  }

  return ptNew;
}

/*
 * PopupMenu --
 *   Pop down previous child menu, if any, and pop up new child menu.
 */
void Menu::PopupMenu()
{
  Menu* child = GetChildMenu();

  if (child)
    child->UnmapMenu();

  this -> ExecFunction(Q_DIR, delayMenuNum);
}

/*
 * PopdownMenu --
 *   Pop down previous child menu.
 */
void Menu::PopdownMenu()
{
  UnmapMenu();
}  

void Menu::Leave(Window win)
{
  XEvent ev;

  if (child) {
    if (XCheckTypedWindowEvent(display, child->frame, EnterNotify, &ev)) {
      child->Enter(child->frame);
      return;
    }
    else
      child->UnmapMenu();
  }
  else
    timer->ResetTimeout(new Callback<Menu>(this, &Menu::PopupMenu));

  int pFocus = iFocus;

  if (iFocus != -1) {
    iFocus = -1;
    SetMenuFocus(pFocus);
  }
}

/*
 * Enter --
 *   Process enter event.
 */
void Menu::Enter(Window win)
{
  if (!CheckMapped() || win == frame)
    return;

  // If this menu has just been mapped and the mouse pointer is above
  // the window, this is not yet a reason to give focus to a menu item.
  // Because the EnterNotify event would be generated even when user
  // didn't touch the mouse (when the newly-mapped window is positioned
  // right below pointer.  So we ignore EnterNotify events until user
  // moves the mouse or clicks a button.
  if (JustMapped) { return; }

  int num, pFocus;
  num = FindItem(win);

  if (iFocus != num) {
    if (iFocus != -1) {
      pFocus = iFocus;
      iFocus = -1;
      SetMenuFocus(pFocus);
    }
    if (func[num] != Q_SEPARATOR) {
      iFocus = num;
      SetMenuFocus(num);
    }

    timer->ResetTimeout(new Callback<Menu>(this, &Menu::PopupMenu));

    /*
     * Delay that child menu popups.
     */
    if (func[num] == Q_DIR) {
      delayMenuNum = num;
      if (child)
	timer->SetTimeout(MenuDelayTime2,
			  new Callback<Menu>(this, &Menu::PopupMenu));
      else
	timer->SetTimeout(MenuDelayTime,
			  new Callback<Menu>(this, &Menu::PopupMenu));
    }
    else {
      if (child)
	timer->SetTimeout(MenuDelayTime,
			  new Callback<Menu>(child, &Menu::PopdownMenu));
    }
  }
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
void Menu::Button1Press(Window win) 
{
  int num;

  num = FindItem(win);
  ASSERT(num >= 0 && num < nitems);

  if (func[num] == Q_DIR) {
    timer->ForceTimeout(new Callback<Menu>(this, &Menu::PopupMenu));

    /*
     * When you operate a menu by both key and mouse, and when child menu
     * is not still be popuped. ???
     */
    if (child != next[num]) {
      Enter(win);
      timer->ForceTimeout(new Callback<Menu>(this, &Menu::PopupMenu));
    }
  }
}

/*
 * Button1Release --
 *   Process the release of button1(mouse left button).
 */
void Menu::Button1Release(Window win)
{
  int num;

  num = FindItem(win);
  ASSERT(num >= 0 && num < nitems);

  if (IsSelectable(func[num])) {
    ASSERT(qvWm);
    /*
     * Erase all menus.
     */
    if (func[num] != Q_DIR) {
      UnmapAllMenus();
      this -> ExecFunction(func[num], num);
    }
  }
}

/*
 * PointerMotion --
 *   Process the motion of mouse.
 */
void Menu::PointerMotion(Window win)
{
  int num;
  // if this menu was just just mapped, the EnterNotify event hasn't
  // been handled yet.  But now it is the right time to do that:
  if(JustMapped) { 
    JustMapped = False;
    Enter(win);
  }
  
  // the mouse pointer is outside of this menu
  if (iFocus == -1)
    return;

  num = FindItem(win);
  if (num != iFocus && func[num] != Q_SEPARATOR)
    Enter(win);
}

void Menu::ExecShortCutKey(char key)
{
  for (int i = 0; i < nitems; i++) {
    if (key >= 'a' && key <= 'z')
      key -= 0x20;
    if (scKey[i] == key)
      if (IsSelectable(func[i])) {
	if (func[i] != Q_DIR)
	  UnmapAllMenus();
	else {
	  if (iFocus != i) {
	    if (iFocus != -1) {
	      int pFocus = iFocus;
	      iFocus = -1;
	      SetMenuFocus(pFocus);
	    }
	    iFocus = i;
	    SetMenuFocus(i);
	  }
	}
        this -> ExecFunction( func[i], i );
      }
  }
}

Bool Menu::IsChecked(FuncNumber fn)
{
  switch (fn) {
  case Q_TOGGLE_ONTOP:
    if (qvWm->CheckFlags(ONTOP))
      return True;
    break;
    
  case Q_TOGGLE_STICKY:
    if (qvWm->CheckFlags(STICKY))
      return True;
    break;

  case Q_TOGGLE_AUTOHIDE:
    if (TaskbarAutoHide)
      return True;
    break;
    
  case Q_TOGGLE_TASKBAR:
    if (UseTaskbar && enableTaskbar)
      return True;
    break;

  case Q_TOGGLE_PAGER:
    if (UsePager && enablePager)
      return True;
    break;

  default:
    break;
  }

  return False;
}

Bool Menu::IsSelected(FuncNumber fn)
{
  switch (fn) {
  case Q_BOTTOM:
    if (UseTaskbar)
      if (taskBar->GetPos() == Taskbar::BOTTOM)
	return True;
    break;

  case Q_TOP:
    if (UseTaskbar)
      if (taskBar->GetPos() == Taskbar::TOP)
	return True;
    break;

  case Q_LEFT:
    if (UseTaskbar)
      if (taskBar->GetPos() == Taskbar::LEFT)
	return True;
    break;

  case Q_RIGHT:
    if (UseTaskbar)
      if (taskBar->GetPos() == Taskbar::RIGHT)
	return True;
    break;

  default:
    break;
  }

  return False;
}

/*
 * IsSelectable --
 *   Return True if the menu item is selectable.
 */
Bool Menu::IsSelectable(FuncNumber fn)
{
  ASSERT(qvWm);

  switch (fn) {
  case Q_NONE:
    return False;
    
  case Q_RESTORE:
    if (qvWm->CheckStatus(MINIMIZE_WINDOW | MAXIMIZE_WINDOW))
      return True;
    return False;

  case Q_MOVE:
  case Q_RESIZE:
    if (qvWm->CheckStatus(MINIMIZE_WINDOW | MAXIMIZE_WINDOW))
      return False;
    return True;

  case Q_MINIMIZE:
    if (qvWm->CheckStatus(MINIMIZE_WINDOW) || qvWm->CheckFlags(NO_TBUTTON))
      return False;
    return True;

  case Q_MAXIMIZE:
    if (qvWm->CheckStatus(MAXIMIZE_WINDOW) &&
	!qvWm->CheckStatus(MINIMIZE_WINDOW))
      return False;
    return True;

  case Q_SEPARATOR:
    return False;

  case Q_BOTTOM:
  case Q_TOP:
  case Q_LEFT:
  case Q_RIGHT:
    if (UseTaskbar && !DisableDesktopChange)
      return True;
    else
      return False;

  case Q_LEFT_PAGING:
    {
      Rect rcRoot = rootQvwm->GetRect();
      Rect rcVirt = paging->GetVirtRect();
      if (paging->origin.x > rcVirt.x * rcRoot.width)
	return True;
      else
	return False;
    }

  case Q_RIGHT_PAGING:
    {
      Rect rcRoot = rootQvwm->GetRect();
      Rect rcVirt = paging->GetVirtRect();
      if (paging->origin.x < (rcVirt.x + rcVirt.width - 1) * rcRoot.width)
	return True;
      else
	return False;
    }

  case Q_UP_PAGING:
    {
      Rect rcRoot = rootQvwm->GetRect();
      Rect rcVirt = paging->GetVirtRect();
      if (paging->origin.y > rcVirt.y * rcRoot.height)
	return True;
      else
	return False;
    }
    
  case Q_DOWN_PAGING:
    {
      Rect rcRoot = rootQvwm->GetRect();
      Rect rcVirt = paging->GetVirtRect();
      if (paging->origin.y < (rcVirt.y + rcVirt.height - 1) * rcRoot.height)
	return True;
      else
	return False;
    }

  default:
    return True;
  }
}

void Menu::Initialize()
{
  Menu::context = XUniqueContext();

  /*
   * create images for default folder
   */
  if ( DefaultFolderIcon ) {
    imgMenu[0] = CreateImageFromFile( DefaultFolderIcon, timer);
  } else {
    imgMenu[0] = new PixmapImage(folder16);
  }

  if ( DefaultMenuItemIcon ) {
    imgMenu[1] = CreateImageFromFile( DefaultMenuItemIcon, timer);
  } else {
    imgMenu[1] = new PixmapImage(icon16);
  }
  

  /*
   * create pixmap for next mark
   */
  imgNext[0] = new PixmapImage(next_white);
  imgNext[1] = new PixmapImage(next_black);

  /*
   * create pixmap for check mark
   */
  imgCheck[0] = new PixmapImage(check_white);
  imgCheck[1] = new PixmapImage(check_black);

  /*
   * create pixmap for selector
   */
  imgSelector[0] = new PixmapImage(selector_white);
  imgSelector[1] = new PixmapImage(selector_black);

  if (MenuImage) {
    imgMenuBack = CreateImageFromFile(MenuImage, timer);
    if (imgMenuBack == NULL) {
      delete [] MenuImage;
      MenuImage = NULL;
    }
  }
  if (MenuActiveImage) {
    imgActiveMenuBack = CreateImageFromFile(MenuActiveImage, timer);
    if (imgActiveMenuBack == NULL) {
      delete [] MenuActiveImage;
      MenuActiveImage = NULL;
    }
  }
}

/*
 * UnmapAllMenus --
 *   Unmap all menus (qvwm ctrl menu, startmenu, icon menu and taskbar menu).
 *   Only a menu in these menus is mapped.
 */
void Menu::UnmapAllMenus(Bool hideTaskbar)
{
  if (Qvwm::focusQvwm && Qvwm::focusQvwm->CheckMenuMapped())
    Qvwm::focusQvwm->UnmapMenu();
  else if (::ctrlMenu->CheckMapped())
    ::ctrlMenu->UnmapMenu(); // for NOFOCUS window
  else if (startMenu && startMenu->CheckMapped())
    startMenu->UnmapMenu();
  else if (Icon::focusIcon && Icon::focusIcon->CheckMenuMapped())
    Icon::focusIcon->UnmapMenu();
  else if (UseTaskbar && taskBar->CheckMenuMapped())
    taskBar->UnmapMenu();

  if (UseTaskbar)
    if (hideTaskbar && TaskbarAutoHide && !taskBar->IsHiding())
      taskBar->HideTaskbar();
}

/*
 * CheckAnyMenusMapped --
 *   check whether any menus are mapped.
 */
Bool Menu::CheckAnyMenusMapped()
{
  if ((Qvwm::focusQvwm && Qvwm::focusQvwm->CheckMenuMapped()) ||
      ::ctrlMenu->CheckMapped() ||
      (startMenu && startMenu->CheckMapped()) ||
      (Icon::focusIcon && Icon::focusIcon->CheckMenuMapped()) ||
      (UseTaskbar && taskBar->CheckMenuMapped()))
    return True;
  
  return False;
}

/*
 * GetMappedMenuFrame --
 *   get the frame of a mapped menu.
 */
Window Menu::GetMappedMenuFrame()
{
  if (Qvwm::focusQvwm && Qvwm::focusQvwm->CheckMenuMapped())
    return Qvwm::focusQvwm->ctrlMenu->GetFrameWin();
  else if (::ctrlMenu->CheckMapped())
    return ::ctrlMenu->GetFrameWin();
  else if (startMenu && startMenu->CheckMapped())
    return startMenu->GetFrameWin();
  else if (Icon::focusIcon && Icon::focusIcon->CheckMenuMapped())
    return Icon::ctrlMenu->GetFrameWin();
  else if (UseTaskbar && taskBar->CheckMenuMapped())
    return taskBar->GetMenu()->GetFrameWin();

  return None;
}
