/*
 * menu_func.cc
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
#include "menu.h"
#include "function.h"
#include "util.h"

void Menu::ExecFunction(FuncNumber fn, int i)
{
  QvFunction::execFunction(fn, this, i);
}

void Menu::ExtractChildMenu(int index)
{
  Point ptFrame;
  int gDir;

  if (index < 0 || index >= nitems)
    return;

  // delayed menu creation
  if (next[index] == NULL) {
    ASSERT(childItem[index]);
    next[index] = new Menu(childItem[index], fs, this, qvWm,
			   leftMargin, nameMargin, hiMargin,
			   imgParentFolder, imgParentIcon);
  }

  switch (fDir) {
    /*
     * To right side.
     */
  case RIGHT:
    ptFrame.x = rc.x + rc.width - 7;
    if (ptFrame.x + next[index]->rc.width > rcScreen.width-1) {
      ptFrame.x = rc.x - next[index]->rc.width + MenuFrameWidth;
      fDir = LEFT;
      gDir = GD_LEFT;
    }
    else
      gDir = GD_RIGHT;
    break;

    /*
     * To left side.
     */
  case LEFT:
    ptFrame.x = rc.x - next[index]->rc.width+MenuFrameWidth;
    if (ptFrame.x + next[index]->rc.width < 0) {
      ptFrame.x = rc.x + rc.width - 7;
      fDir = RIGHT;
      gDir = GD_RIGHT;
    }
    else
      gDir = GD_LEFT;
    break;

  default:
    gDir = 0;  // XXX for warning
    ASSERT(False);
  }

  ptFrame.y = rc.y + CalcItemYPos(index) - MenuFrameWidth;
  if (ptFrame.y + next[index]->rc.height > rcScreen.height - 1) {
    ptFrame.y = ptFrame.y + itemHeight - next[index]->rc.height +
      MenuFrameWidth * 2;
    gDir |= GD_UP;
  }
  else
    gDir |= GD_DOWN;

  child = next[index];
  child->SetQvwm(qvWm);
  child->MapMenu(ptFrame.x, ptFrame.y, gDir);
}

void Menu::MoveFocusUp()
{
  int pFocus;

  if (iFocus == -1) {
    iFocus = nitems - 1;
    while (func[iFocus] == Q_SEPARATOR)
      if (--iFocus == -1)
	return;
  }
  else {
    ASSERT(iFocus >= 0 && iFocus < nitems);
    pFocus = iFocus;

    do {
      if (--iFocus == -1)
	iFocus = nitems - 1;
    } while (func[iFocus] == Q_SEPARATOR);

    SetMenuFocus(pFocus);
  }

  ASSERT(iFocus >= 0 && iFocus < nitems);
  SetMenuFocus(iFocus);
}

void Menu::MoveFocusDown()
{
  int pFocus;

  if (iFocus == -1) {
    iFocus = 0;
    while (func[iFocus] == Q_SEPARATOR)
      if (++iFocus == nitems)
	return;
  }
  else {
    ASSERT(iFocus >= 0 && iFocus < nitems);
    pFocus = iFocus;

    do {
      if (++iFocus == nitems)
	iFocus = 0;
    } while (func[iFocus] == Q_SEPARATOR);

    SetMenuFocus(pFocus);
  }

  ASSERT(iFocus >= 0 && iFocus < nitems);
  SetMenuFocus(iFocus);
}

void Menu::MoveFocusRight()
{
  if (fDir == RIGHT) {
    if (iFocus == -1)
      return;
    ASSERT(iFocus >= 0 && iFocus < nitems);
    if (func[iFocus] == Q_DIR) {
      this -> ExecFunction(Q_DIR, iFocus);
      ASSERT(child);
      QvFunction::execFunction(Q_DOWN_FOCUS, child);
    }
  }
  else {
    ASSERT(fDir == LEFT);
    if (parent != NULL)
      UnmapMenu();
  }
}

void Menu::MoveFocusLeft()
{
  if (fDir == LEFT) {
    if (iFocus == -1)
      return;
    ASSERT(iFocus >= 0 && iFocus < nitems);
    if (func[iFocus] == Q_DIR) {
      this -> ExecFunction(Q_DIR, iFocus);
      ASSERT(child);
      QvFunction::execFunction(Q_DOWN_FOCUS, child);
    }
  }
  else {
    ASSERT(fDir == RIGHT);
    if (parent != NULL)
      UnmapMenu();
  }
}

// exec a function of the focused menu item
void Menu::ExecSelectedItem()
{
  int pFocus;

  ASSERT(iFocus >= 0 && iFocus < nitems);

  if (IsSelectable(func[iFocus])) {
    pFocus = iFocus;
    if (func[iFocus] != Q_DIR)
      UnmapAllMenus();
    
    this -> ExecFunction(func[pFocus], pFocus);
  }
}

void Menu::ExecIndexedItem(int index)
{
  ExecCommand(exec[index]);
}
