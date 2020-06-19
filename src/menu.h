/*
 * menu.h
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

#ifndef _MENU_H_
#define _MENU_H_

#include "misc.h"
#include "util.h"

class Qvwm;
class MenuElem;
class QvImage;

#define GD_RIGHT (1 << 0)
#define GD_LEFT  (1 << 1)
#define GD_DOWN  (1 << 2)
#define GD_UP    (1 << 3)

/*
 * Menu class
 */
class Menu {
protected:
  Window frame;
  Rect rc;                   // menu pos & size
  Bool mapped;               // flag if this menu is mapped
  int nitems;                // item number
  int iFocus;                // focus item number
  Menu* parent;              // parent of this menu
  Menu* child;               // menu extracted from this menu

  int imageArea;             // leftMargin + image width + midMargin
  int imageMaxWidth;         // width of the widest item image
  int itemHeight;            // image height + hiMargin * 2
  int leftMargin;            // left margin of image
  int nameMargin;            // margin between image and name
  int hiMargin;              // top and bottom margin of image

  XFontSet& fs;
  Qvwm* qvWm;
  QvImage* imgParentFolder;
  QvImage* imgParentIcon;

  Window* item;              // menu items
  QvImage** img;
  const char **name;
  FuncNumber* func;          // function number
  const char **exec;               // exec command
  char *scKey;               // short cut key for menu item
  Bool *check;               // check for toggle function
  Menu **next;               // child menu
  MenuElem** childItem;      // keep child menu element for delayed creation

  QvImage** imgBack;
  QvImage** imgActiveBack;
  
  QvImage **imgWhiteNext, **imgBlackNext;
  QvImage **imgWhiteSelector, **imgBlackSelector;
  QvImage **imgWhiteCheck, **imgBlackCheck;

  int delayMenuNum;

  static QvImage *imgMenu[2];
  static QvImage *imgNext[2];
  static QvImage *imgSelector[2];
  static QvImage *imgCheck[2];
  static QvImage *imgMenuBack, *imgActiveMenuBack;

  static const int SeparatorHeight = 9;
  static const int MenuFrameWidth = 3;

public:
  // direction of menu extraction
  enum MenuDir { RIGHT, LEFT };

  static XContext context;
  static MenuDir fDir;          // orientation of menu extraction

private:
  void DrawFrame();
  void DrawSeparator(int num);
  void DrawMenuContents(int num);

  void PopupMenu();
  void PopdownMenu();

public:
  Menu(MenuElem* mItem, XFontSet& menufs, Menu* par, Qvwm* qvwm,
       int lMargin = 2, int nMargin = 4, int hMargin = 2,
       QvImage* imgDefFolder = NULL, QvImage* imgDefIcon = NULL);
  virtual ~Menu();
  Window GetFrameWin() const { return frame; }
  Bool CheckMapped() const { return mapped; }
  int GetItemNum() const { return nitems; }
  void SetRect(const Rect& rect) { rc = rect; }
  Rect GetRect() const { return rc; }
  void SetQvwm(Qvwm* qvwm) { qvWm = qvwm; }
  Qvwm* GetQvwm() const { return qvWm; }
  Menu* GetChildMenu() const { return child; }
  
  virtual void MapMenu(int x, int y, int dir = GD_RIGHT | GD_DOWN);
  virtual void UnmapMenu();
  virtual void DrawMenu(Window win);
  virtual void SetMenuFocus(int num);
  virtual int FindItem(Window win);
  int CalcItemYPos(int num);
  Point GetFixedMenuPos(const Point& pt, int& dir);
  virtual void Leave(Window win);
  virtual void Enter(Window win);
  virtual void Button1Press(Window win);
  virtual void Button1Release(Window win);
  virtual void PointerMotion(Window win);

  Bool IsSelectable(FuncNumber fn);
  void ExecShortCutKey(char key);
  Bool IsChecked(FuncNumber fn);
  Bool IsSelected(FuncNumber fn);

  virtual void ExecFunction(FuncNumber fn, int i);
  void ExtractChildMenu(int index);
  void MoveFocusUp();
  void MoveFocusDown();
  void MoveFocusRight();
  void MoveFocusLeft();
  void ExecSelectedItem();
  void ExecIndexedItem(int index);

  static void Initialize();
  static void UnmapAllMenus(Bool hideTaskbar = True);
  static Bool CheckAnyMenusMapped();
  static Window GetMappedMenuFrame();
};

#endif // _MENU_H_
