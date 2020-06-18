/*
 * function.cc
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
#ifdef __EMX__
#include <process.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "misc.h"
#include "function.h"
#include "qvwm.h"
#include "menu.h"
#include "startmenu.h"
#include "fbutton.h"
#include "parse.h"
#include "event.h"
#include "switcher.h"
#include "qvwmrc.h"
#include "exit_dialog.h"
#include "confirm_dialog.h"
#include "paging.h"
#include "pager.h"
#include "icon.h"
#include "focus_mgr.h"
#include "desktop.h"
#include "gnome.h"
#include "hash.h"

/* XXX */
KeyCode swCode;
unsigned int swMod;

Bool QvFunction::execFunction(FuncNumber fn, Menu* menu, int index)
{
  if (index < 0 || index >= menu->GetItemNum()) {
    QvwmError("invalid menu item index: %d", index);
    return True;
  }

  switch (fn) {
  /*
   * Extract the menu.
   */
  case Q_DIR:
    menu->ExtractChildMenu(index);
    break;

  /*
   * Execute the external command.
   */
  case Q_EXEC:
    PlaySound(MenuCommandSound, True);
    PlaySound(OpenSound);
    menu->ExecIndexedItem(index);
    break;

  default:
    PlaySound(MenuCommandSound);
    return execFunction(fn, menu);
  }

  return True;
}

// functions for a menu or the selected menu item
Bool QvFunction::execFunction(FuncNumber fn, Menu* menu)
{
  Window junkRoot, junkChild;
  Point ptRoot, ptJunk;
  unsigned int mask;
  int borderWidth, topBorder, titleHeight, titleEdge;
  Rect rect;

  ASSERT(menu);

  switch (fn) {
  /*
   * Popup the menu.
   */
  case Q_POPUP_MENU:
    if (!menu->CheckMapped()) {
      Qvwm* qvWm = menu->GetQvwm();

      Menu::UnmapAllMenus();

      if (qvWm == rootQvwm) {
	XQueryPointer(display, root, &junkRoot, &junkChild,
		      &ptRoot.x, &ptRoot.y, &ptJunk.x, &ptJunk.y, &mask);
	menu->MapMenu(ptRoot.x, ptRoot.y);
      }
      else {
	qvWm->GetBorderAndTitle(borderWidth, topBorder, titleHeight,
				titleEdge);
	rect = qvWm->GetRect();
 	menu->MapMenu(rect.x + topBorder, rect.y + topBorder + titleHeight);
      }
    }
    break;

  /*
   * Popdown one menu.
   */
  case Q_POPDOWN_MENU:
    // Erase only one menu.
    menu->UnmapMenu();
    break;

  /*
   * Move the focus of menu.
   */
  case Q_UP_FOCUS:
    menu->MoveFocusUp();
    break;
    
  case Q_DOWN_FOCUS:
    menu->MoveFocusDown();
    break;

  case Q_RIGHT_FOCUS:
    menu->MoveFocusRight();
    break;

  case Q_LEFT_FOCUS:
    menu->MoveFocusLeft();
    break;
    
  /*
   * Execute the function of the menu item.
   */
  case Q_ACTION:
    menu->ExecSelectedItem();
    break;

  default:
    return execFunction(fn, menu->GetQvwm());
  }

  return True;
}

Bool QvFunction::execFunction(FuncNumber fn, Qvwm* qvWm)
{
  Point pt, ptJunk;
  Rect rect;
  Window junkRoot, junkChild;
  unsigned int mask;

  ASSERT(qvWm);

  Menu::UnmapAllMenus();

/* XXX some, most or all of these functions cannot be performed on the
 * root window, but we do not always check that. */
  if(qvWm==rootQvwm) {
    QvwmError( "attempt function %d on the root window", fn );
  }

  switch (fn) {
  /*
   * Restore the window pos & size.
   */
  case Q_RESTORE:
    if (qvWm->CheckStatus(MAXIMIZE_WINDOW) &&
	!qvWm->CheckStatus(MINIMIZE_WINDOW)) {
      ASSERT(qvWm->fButton[1]);
      qvWm->fButton[1]->ChangeImage(FrameButton::MAXIMIZE);
    }
    qvWm->RestoreWindow();

    /*
     * After mapped, give the focus to qvWm.
     */
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;


  // focus  
  case Q_FOCUS:
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  case Q_RAISE_FOCUS:
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    if (qvWm->CheckMapped())
      qvWm->RaiseWindow(True);
    else
      qvWm->RestoreWindow();

    qvWm->AdjustPage();
    break;


  /*
   * Minimize the window.
   */
  case Q_MINIMIZE:
    if( qvWm == rootQvwm ) { break; }
    qvWm->MinimizeWindow();
    break;

  /*
   * Maximize or restore the window, depending on its current state
   */
  case Q_MAXIMIZE:
    if( qvWm == rootQvwm ) { break; }
    ASSERT(qvWm->fButton[1]);
    
    if ( qvWm->CheckStatus(MAXIMIZE_WINDOW) ) {
      qvWm->fButton[1]->ChangeImage(FrameButton::MAXIMIZE);
      qvWm->RestoreWindow();
    } else {
      qvWm->fButton[1]->ChangeImage(FrameButton::RESTORE);
      qvWm->MaximizeWindow();
    }	
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  /*
   * Maximize the window, unconditional
   */
  case Q_MAXIMIZE_ONEWAY:
    if( qvWm == rootQvwm ) { break; }
    ASSERT(qvWm->fButton[1]);
    qvWm->fButton[1]->ChangeImage(FrameButton::RESTORE);
    qvWm->MaximizeWindow();
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;


  /*
   * Move the window.
   */
  case Q_MOVE:
    rect = qvWm->GetRect();
    XDefineCursor(display, root, cursor[MOVE]);
    XQueryPointer(display, root, &junkRoot, &junkChild, &pt.x, &pt.y,
		  &ptJunk.x, &ptJunk.y, &mask);
    qvWm->SetStatus(PRESS_FRAME);
    qvWm->MoveWindow(pt, False);
    XDefineCursor(display, root, cursor[SYS]);
    break;

  /*
   * Resize the window.
   */
  case Q_RESIZE:
    rect = qvWm->GetRect();
    XDefineCursor(display, root, cursor[MOVE]);
    XQueryPointer(display, root, &junkRoot, &junkChild, &pt.x, &pt.y,
		  &ptJunk.x, &ptJunk.y, &mask);
    qvWm->SetStatus(PRESS_FRAME);
    qvWm->ResizeWindow(0, False);
    XDefineCursor(display, root, cursor[SYS]);
    break;


  /*
   * Expand the window "in place" without obscuring other windows
   */
  case Q_EXPAND:
    qvWm->ExpandWindow(EXPAND_LEFT | EXPAND_RIGHT | EXPAND_UP | EXPAND_DOWN);
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  case Q_EXPAND_LEFT:
    qvWm->ExpandWindow(EXPAND_LEFT);
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  case Q_EXPAND_RIGHT:
    qvWm->ExpandWindow(EXPAND_RIGHT);
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  case Q_EXPAND_UP:
    qvWm->ExpandWindow(EXPAND_UP);
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  case Q_EXPAND_DOWN:
    qvWm->ExpandWindow(EXPAND_DOWN);
    rootQvwm->SetFocus();
    qvWm->SetFocus();
    break;

  /*
   * Close the window.
   */
  case Q_CLOSE:
    if (qvWm == rootQvwm) {
      QvFunction::execFunction(Q_EXIT);
    }
    else {
      qvWm->FetchWMProtocols();
      if (qvWm->CheckFlags(WM_DELETE_WINDOW)) {
        PlaySound(CloseSound);
	qvWm->CloseWindow();
      }
      else
	qvWm->KillClient();
    }
    break;

  /*
   * Kill the application.
   */
  case Q_KILL:
    if (qvWm != rootQvwm)
      qvWm->KillClient();
    break;

  /*
   * Raise a window
   */
  case Q_RAISE:
    qvWm->RaiseWindow(True);
    break;
    
  /*
   * Lower a window
   */
  case Q_LOWER:
    qvWm->LowerWindow();
    break;

  /*
   * Toggle switch of an ONTOP attribute
   */
  case Q_TOGGLE_ONTOP:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(ONTOP)) {
	qvWm->ResetFlags(ONTOP);
	desktop.GetOnTopList().Remove(qvWm);
	Gnome::ChangeLayer(qvWm, WIN_LAYER_NORMAL);
      }
      else {
	qvWm->SetFlags(ONTOP);
	desktop.GetOnTopList().InsertTail(qvWm);
	Gnome::ChangeLayer(qvWm, WIN_LAYER_ONTOP);
      }
    }
    break;

  case Q_TOGGLE_STICKY:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(STICKY)) {
	qvWm->ResetFlags(STICKY);
	qvWm->ConfigureNewRect(qvWm->GetRect());
	Gnome::ResetState(qvWm, WIN_STATE_STICKY);
      }
      else {
	qvWm->SetFlags(STICKY);
	qvWm->ConfigureNewRect(qvWm->GetRect());
	Gnome::SetState(qvWm, WIN_STATE_STICKY);
      }
    }
    break;

  case Q_TOGGLE_BORDER:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(BORDER)) {
	qvWm->ResetFlags(BORDER);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(BORDER);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_BORDER_EDGE:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(BORDER_EDGE)) {
	qvWm->ResetFlags(BORDER_EDGE);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(BORDER_EDGE);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_BUTTON1:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(BUTTON1)) {
	qvWm->ResetFlags(BUTTON1);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(BUTTON1);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_BUTTON2:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(BUTTON2)) {
	qvWm->ResetFlags(BUTTON2);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(BUTTON2);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_BUTTON3:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(BUTTON3)) {
	qvWm->ResetFlags(BUTTON3);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(BUTTON3);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_CTRLBTN:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(CTRL_MENU)) {
	qvWm->ResetFlags(CTRL_MENU);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(CTRL_MENU);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_FOCUS:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(NO_FOCUS)) {
	qvWm->ResetFlags(NO_FOCUS);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(NO_FOCUS);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_TBUTTON:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(NO_TBUTTON)) {
	qvWm->ResetFlags(NO_TBUTTON);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(NO_TBUTTON);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  case Q_TOGGLE_TITLE:
    if (qvWm != rootQvwm) {
      if (qvWm->CheckFlags(TITLE)) {
	qvWm->ResetFlags(TITLE);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
      else {
	qvWm->SetFlags(TITLE);
	qvWm->ConfigureNewRect(qvWm->GetRect());
        qvWm->MinimizeWindow();
	qvWm->RestoreWindow ();
      }
    }
    break;

  default:
    return execFunction(fn);
  }

  return True;
}

Bool QvFunction::execFunction(FuncNumber fn)
{
  Rect rect;

  Menu::UnmapAllMenus(False);

  switch (fn) {
  /*
   * Do nothing.
   */
  case Q_NONE:
    break;

  /*
   * Exit qvwm.
   */
  case Q_EXIT:
    if (!UseExitDialog) {
      if (UseConfirmDialog) {
	if (confirmDlg == NULL)
	  confirmDlg = new ConfirmDialog();
	confirmDlg->MapDialog();
	confirmDlg->ProcessDialog();
      }
      else
	FinishQvwm();
    }
    else {
      if (exitDlg == NULL)
	exitDlg = new ExitDialog();
      exitDlg->MapDialog();
      exitDlg->ProcessDialog();
    }
    break;

  /*
   * Restart qvwm.
   */
  case Q_RESTART:
    RestartQvwm();
    break;

  /*
   * Switch the task to another task with the task switcher.
   */
  case Q_SWITCH_TASK:
    if (taskSwitcher == NULL)
      taskSwitcher = new TaskSwitcher();
    taskSwitcher->SwitchTask(True, swCode, swMod);
    break;

  /*
   * Switch back the task to another task with the task switcher.
   */
  case Q_SWITCH_TASK_BACK:
    if (taskSwitcher == NULL)
      taskSwitcher = new TaskSwitcher();
    taskSwitcher->SwitchTask(False, swCode, swMod);
    break;

  /*
   * Switch the window to next window in focus stack.
   */
  case Q_CHANGE_WINDOW:
    focusMgr.RollFocus(True);
    break;

  case Q_CHANGE_WINDOW_BACK:
    focusMgr.RollFocus(False);
    break;

  case Q_CHANGE_WINDOW_INSCR:
    focusMgr.RollFocusWithinScreen(True);
    break;

  case Q_CHANGE_WINDOW_BACK_INSCR:
    focusMgr.RollFocusWithinScreen(False);
    break;

  case Q_DESKTOP_FOCUS:
    rootQvwm->SetFocus();
    break;

  /*
   * Popup the start menu.
   */
  case Q_POPUP_START_MENU:
    if (startMenu == NULL)
      startMenu = new StartMenu(StartMenuItem);

    if (!startMenu->CheckMapped()) {
      rootQvwm->SetFocus();
      startMenu->MapMenu();
    }

    break;

  /*
   * Popup the desktop menu.
   */
  case Q_POPUP_DESKTOP_MENU:
    rootQvwm->SetFocus();
    QvFunction::execFunction(Q_POPUP_MENU, rootQvwm->ctrlMenu);
    break;

  /*
   * Popdown all menu.
   */
  case Q_POPDOWN_ALL_MENU:
    Menu::UnmapAllMenus();
    break;

  /*
   * Switch to left virtual page.
   */
  case Q_LEFT_PAGING:
    paging->SwitchPageLeft();
    break;

  case Q_RIGHT_PAGING:
    paging->SwitchPageRight();
    break;

  case Q_UP_PAGING:
    paging->SwitchPageUp();
    break;

  case Q_DOWN_PAGING:
    paging->SwitchPageDown();
    break;

    /*
     * Rearrange windows
     */
  case Q_OVERLAP:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.Overlap(True, rect);
    break;

  case Q_OVERLAP_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.Overlap(False, rect);
    break;

  case Q_TILE_HORZ:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.TileHorz(True, rect);
    break;

  case Q_TILE_HORZ_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.TileHorz(False, rect);
    break;

  case Q_TILE_VERT:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.TileVert(True, rect);
    break;

  case Q_TILE_VERT_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.TileVert(False, rect);
    break;

  case Q_MINIMIZE_ALL:
    desktop.MinimizeAll(True);
    break;

  case Q_MINIMIZE_ALL_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.MinimizeAll(False, rect);
    break;

  case Q_RESTORE_ALL:
    desktop.RestoreAll(True);
    break;

  case Q_RESTORE_ALL_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.RestoreAll(False, rect);
    break;

  case Q_CLOSE_ALL:
    desktop.CloseAll(True);
    break;

  case Q_CLOSE_ALL_INSCR:
    rect = Rect(paging->origin.x, paging->origin.y,
		rcScreen.width, rcScreen.height);
    desktop.CloseAll(False, rect);
    break;

  /*
   * Move taskbar.
   */
  case Q_BOTTOM:
    if (UseTaskbar && !DisableDesktopChange)
      taskBar->MoveTaskbar(Taskbar::BOTTOM);
    break;

  case Q_TOP:
    if (UseTaskbar && !DisableDesktopChange)
      taskBar->MoveTaskbar(Taskbar::TOP);
    break;

  case Q_LEFT:
    if (UseTaskbar && !DisableDesktopChange)
      taskBar->MoveTaskbar(Taskbar::LEFT);
    break;

  case Q_RIGHT:
    if (UseTaskbar && !DisableDesktopChange)
      taskBar->MoveTaskbar(Taskbar::RIGHT);
    break;

  case Q_TOGGLE_TASKBAR:
    if (UseTaskbar && !DisableDesktopChange) {
      if (enableTaskbar)
	execFunction(Q_DISABLE_TASKBAR);
      else
	execFunction(Q_ENABLE_TASKBAR);
    }
    break;

  case Q_ENABLE_TASKBAR:
    if (UseTaskbar && !DisableDesktopChange) {
      if (!enableTaskbar) {
	enableTaskbar = True;
	taskBar->EnableTaskbar();
      }
    }
    break;

  case Q_DISABLE_TASKBAR:
    if (UseTaskbar && !DisableDesktopChange) {
      if (enableTaskbar) {
	enableTaskbar = False;
	taskBar->DisableTaskbar();
        // YYY -- give focus to the last used window 
      }
    }
    break;

  case Q_TOGGLE_AUTOHIDE:
    if (UseTaskbar && !DisableDesktopChange) {
      if (TaskbarAutoHide)
	execFunction(Q_DISABLE_AUTOHIDE);
      else
	execFunction(Q_ENABLE_AUTOHIDE);
    }
    break;

  case Q_ENABLE_AUTOHIDE:
    if (UseTaskbar && !DisableDesktopChange) {
      if (!TaskbarAutoHide) {
	TaskbarAutoHide = True;
	rcScreen = taskBar->GetScreenRectOnHiding();
	taskBar->HideTaskbar();
	if (UsePager) {
	  ASSERT(pager);
	  pager->RecalcPager();
	}
      }
    }
    break;

  case Q_DISABLE_AUTOHIDE:
    if (UseTaskbar && !DisableDesktopChange) {
      if (TaskbarAutoHide) {
	TaskbarAutoHide = False;
	if (taskBar->IsHiding())
	  taskBar->ShowTaskbar();
	rcScreen = taskBar->GetScreenRectOnShowing();
	if (UsePager) {
	  ASSERT(pager);
	  pager->RecalcPager();
	}
      }
    }
    break;

  case Q_SHOW_TASKBAR:
    if (UseTaskbar && !DisableDesktopChange)
      if (TaskbarAutoHide && taskBar->IsHiding())
	taskBar->ShowTaskbar();
    break;

  case Q_HIDE_TASKBAR:
    if (UseTaskbar && !DisableDesktopChange)
      if (TaskbarAutoHide && !taskBar->IsHiding())
	taskBar->HideTaskbar();
    break;

  case Q_TOGGLE_PAGER:
    if (UsePager && !DisableDesktopChange) {
      if (enablePager)
	execFunction(Q_DISABLE_PAGER);
      else
	execFunction(Q_ENABLE_PAGER);
    }
    break;
        
  case Q_ENABLE_PAGER:
    if (UsePager && !DisableDesktopChange) {
      if (!enablePager) {
	enablePager = True;
	pager->MapPager();
      }
    }
    break;

  case Q_DISABLE_PAGER:
    if (UsePager && !DisableDesktopChange) {
      if (enablePager) {
	enablePager = False;
	pager->UnmapPager();
      }
    }
    break;

  /*
   * Line up desktop icons
   */ 
  case Q_LINEUP_ICON:
    desktop.LineUpAllIcons();
    break;

  default:
    return False;
  }

  return True;
}

/*
 * Table for used variables
 */
static FuncTable funcTable[] =
  {{"QVWM_NONE",			Q_NONE},
   {"QVWM_SEPARATOR",			Q_SEPARATOR},
   // qvwm
   {"QVWM_EXIT",			Q_EXIT},
   {"QVWM_RESTART",			Q_RESTART},
   // window
   {"QVWM_MOVE",			Q_MOVE},
   {"QVWM_RESIZE",			Q_RESIZE},
   {"QVWM_MINIMIZE",			Q_MINIMIZE},
   {"QVWM_MAXIMIZE",			Q_MAXIMIZE},
   {"QVWM_MAXIMIZE_ONEWAY",		Q_MAXIMIZE_ONEWAY},
   {"QVWM_RESTORE",			Q_RESTORE},
   {"QVWM_EXPAND",			Q_EXPAND},
   {"QVWM_EXPAND_LEFT",			Q_EXPAND_LEFT},
   {"QVWM_EXPAND_RIGHT",		Q_EXPAND_RIGHT},
   {"QVWM_EXPAND_UP",			Q_EXPAND_UP},
   {"QVWM_EXPAND_DOWN",			Q_EXPAND_DOWN},
   {"QVWM_RAISE",			Q_RAISE},
   {"QVWM_LOWER",			Q_LOWER},
   {"QVWM_CLOSE",			Q_CLOSE},
   {"QVWM_KILL",			Q_KILL},
   {"QVWM_TOGGLE_ONTOP",		Q_TOGGLE_ONTOP},
   {"QVWM_TOGGLE_STICKY",		Q_TOGGLE_STICKY},
   {"QVWM_TOGGLE_FOCUS",		Q_TOGGLE_FOCUS},
   {"QVWM_TOGGLE_BORDER",		Q_TOGGLE_BORDER},
   {"QVWM_TOGGLE_BORDER_EDGE",		Q_TOGGLE_BORDER_EDGE},
   {"QVWM_TOGGLE_BUTTON1",		Q_TOGGLE_BUTTON1},
   {"QVWM_TOGGLE_BUTTON2",		Q_TOGGLE_BUTTON2},
   {"QVWM_TOGGLE_BUTTON3",		Q_TOGGLE_BUTTON3},
   {"QVWM_TOGGLE_CTRLBTN",		Q_TOGGLE_CTRLBTN},
   {"QVWM_TOGGLE_TBUTTON",		Q_TOGGLE_TBUTTON},
   {"QVWM_TOGGLE_TITLE",		Q_TOGGLE_TITLE},
   // window focus
   {"QVWM_SWITCH_TASK",			Q_SWITCH_TASK},
   {"QVWM_SWITCH_TASK_BACK",		Q_SWITCH_TASK_BACK},
   {"QVWM_CHANGE_WIN",			Q_CHANGE_WINDOW},
   {"QVWM_CHANGE_WIN_BACK",		Q_CHANGE_WINDOW_BACK},
   {"QVWM_CHANGE_WIN_INSCR",		Q_CHANGE_WINDOW_INSCR},
   {"QVWM_CHANGE_WIN_BACK_INSCR",	Q_CHANGE_WINDOW_BACK_INSCR},
   {"QVWM_DESKTOP_FOCUS",		Q_DESKTOP_FOCUS},
   {"QVWM_FOCUS",                       Q_FOCUS},
   {"QVWM_RAISE_FOCUS",                 Q_RAISE_FOCUS},
   // window rearrangement
   {"QVWM_OVERLAP",			Q_OVERLAP},
   {"QVWM_OVERLAP_INSCR",		Q_OVERLAP_INSCR},
   {"QVWM_TILE_HORZ",			Q_TILE_HORZ},
   {"QVWM_TILE_HORZ_INSCR",		Q_TILE_HORZ_INSCR},
   {"QVWM_TILE_VERT",			Q_TILE_VERT},
   {"QVWM_TILE_VERT_INSCR",		Q_TILE_VERT_INSCR},
   {"QVWM_MINIMIZE_ALL",		Q_MINIMIZE_ALL},
   {"QVWM_MINIMIZE_ALL_INSCR",		Q_MINIMIZE_ALL_INSCR},
   {"QVWM_RESTORE_ALL",			Q_RESTORE_ALL},
   {"QVWM_RESTORE_ALL_INSCR",		Q_RESTORE_ALL_INSCR},
   {"QVWM_CLOSE_ALL",			Q_CLOSE_ALL},
   {"QVWM_CLOSE_ALL_INSCR",		Q_CLOSE_ALL_INSCR},
   // menu
   {"QVWM_POPUP_START_MENU",		Q_POPUP_START_MENU},
   {"QVWM_POPUP_DESKTOP_MENU",		Q_POPUP_DESKTOP_MENU},
   {"QVWM_POPUP_MENU",			Q_POPUP_MENU},
   {"QVWM_POPDOWN_MENU",		Q_POPDOWN_MENU},
   {"QVWM_POPDOWN_ALL_MENU",		Q_POPDOWN_ALL_MENU},
   // paging
   {"QVWM_LEFT_PAGING",			Q_LEFT_PAGING},
   {"QVWM_RIGHT_PAGING",		Q_RIGHT_PAGING},
   {"QVWM_UP_PAGING",			Q_UP_PAGING},
   {"QVWM_DOWN_PAGING",			Q_DOWN_PAGING},
   // taskbar
   {"QVWM_BOTTOM",			Q_BOTTOM},
   {"QVWM_TOP",				Q_TOP},
   {"QVWM_LEFT",			Q_LEFT},
   {"QVWM_RIGHT",			Q_RIGHT},
   {"QVWM_TOGGLE_AUTOHIDE",		Q_TOGGLE_AUTOHIDE},
   {"QVWM_ENABLE_AUTOHIDE",		Q_ENABLE_AUTOHIDE},
   {"QVWM_DISABLE_AUTOHIDE",		Q_DISABLE_AUTOHIDE},
   {"QVWM_TOGGLE_TASKBAR",              Q_TOGGLE_TASKBAR},
   {"QVWM_ENABLE_TASKBAR",              Q_ENABLE_TASKBAR},
   {"QVWM_DISABLE_TASKBAR",             Q_DISABLE_TASKBAR},
   {"QVWM_SHOW_TASKBAR",		Q_SHOW_TASKBAR},
   {"QVWM_HIDE_TASKBAR",		Q_HIDE_TASKBAR},
   // pager
   {"QVWM_TOGGLE_PAGER",                Q_TOGGLE_PAGER},
   {"QVWM_ENABLE_PAGER",                Q_ENABLE_PAGER},
   {"QVWM_DISABLE_PAGER",               Q_DISABLE_PAGER},
   // icon
   {"QVWM_LINEUP_ICON",			Q_LINEUP_ICON},
   {"QVWM_EXEC_ICON",			Q_EXEC_ICON},
   {"QVWM_DELETE_ICON",			Q_DELETE_ICON},
   /* for backward compatibility */
   {"QVWM_CHANGEWINDOW",		Q_CHANGE_WINDOW},
   {"QVWM_SWITCHTASK",			Q_SWITCH_TASK},
   {"QVWM_POPUPSTARTMENU",		Q_POPUP_START_MENU},
   {"QVWM_POPUPMENU",			Q_POPUP_MENU},
   {"QVWM_LEFTPAGING",			Q_LEFT_PAGING},
   {"QVWM_RIGHTPAGING",			Q_RIGHT_PAGING},
   {"QVWM_UPPAGING",			Q_UP_PAGING},
   {"QVWM_DOWNPAGING",			Q_DOWN_PAGING},
   {"QVWM_LINEUP",			Q_LINEUP_ICON},
   {"QVWM_TILEHORZ",			Q_TILE_HORZ},
   {"QVWM_TILEVERT",			Q_TILE_VERT},
   {"QVWM_MINALL",			Q_MINIMIZE_ALL},
   {"QVWM_EXECICON",			Q_EXEC_ICON},
   {"QVWM_DELICON",			Q_DELETE_ICON},
   {"QVWM_TOGGLEONTOP",			Q_TOGGLE_ONTOP},
   {"QVWM_TOGGLESTICKY",		Q_TOGGLE_STICKY},
   {"QVWM_TOGGLEAUTOHIDE",		Q_TOGGLE_AUTOHIDE}
  };

Hash<FuncNumber>* QvFunction::funcHashTable;
const int FuncTableSize = sizeof(funcTable) / sizeof(FuncTable);

void QvFunction::initialize()
{
  int i;

  funcHashTable = new Hash<FuncNumber>(HashTableSize);

  for (i = 0; i < FuncTableSize; i++)
    funcHashTable->SetHashItem(funcTable[i].name, &funcTable[i].num);
}
