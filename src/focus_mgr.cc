/*
 * focus_mgr.cc
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
#include "util.h"
#include "qvwm.h"
#include "focus_mgr.h"
#include "qvwmrc.h"
#include "key.h"
#include "pager.h"
#include "mini.h"
#include "paging.h"
#include "fbutton.h"
#include "desktop.h"

void FocusMgr::SetFocus(Qvwm* qvWm)
{
  ASSERT(qvWm);

  Qvwm::focusQvwm = qvWm;

  if (qvWm == rootQvwm) {
    desktop.SetFocus();
    scKey->GrabKeys(root);
    qvWm->InstallWindowColormaps();

//    DumpStack();

    return;
  }

  if (qvWm->CheckMapped()) {
    /*
     * Ungrab buttons so that the window without focus can get focus by
     * ButtonPress.
     */
    if (AutoRaise && qvWm == Qvwm::activeQvwm)
      for (int i = 1; i < 4; i++)
	XUngrabButton(display, i, 0, qvWm->GetWin());
  
    /*
     * Set the input focus to this window.
     */
    XSetInputFocus(display, qvWm->GetWin(), RevertToParent, CurrentTime);

    qvWm->InstallWindowColormaps();

    /*
     * Redraw according to focus change
     */
    qvWm->DrawTitle(True);
    qvWm->ChangeFrameFocus();
    for (int i = 0; i < 3; i++)
      qvWm->fButton[i]->DrawButton();
  }

  ASSERT(qvWm->tButton);
  qvWm->tButton->SetFocus();
  qvWm->tButton->SetState(Button::PUSH);
  qvWm->tButton->DrawButton();

  if (UsePager) {
    ASSERT(qvWm->mini);
    qvWm->mini->SetFocus();
  }

  // move qvWm to the top of the focus stack
  // XXX even if a transient window, give it a focus in the above
  if (!qvWm->CheckFlags(TRANSIENT)) {
    Remove(qvWm);
    Push(qvWm);
  }

  if (qvWm->CheckFlags(WM_TAKE_FOCUS))
    qvWm->SendMessage(_XA_WM_TAKE_FOCUS);

//  DumpStack();
}

void FocusMgr::ResetFocus(Qvwm* qvWm)
{
  ASSERT(qvWm);

  Qvwm::focusQvwm = NULL;

  if (qvWm == rootQvwm) {
    scKey->UngrabKeys(root);
    return;
  }

  if (qvWm->CheckMapped()) {
    /*
     * Grab buttons so that the window without focus can get focus by
     * ButtonPress.
     */
    for (int i = 1; i < 4; i++)
      XGrabButton(display, i, 0, qvWm->GetWin(), True, ButtonPressMask,
		  GrabModeSync, GrabModeAsync, None, cursor[SYS]);
  }

  /*
   * Redraw according to focus change
   */
  qvWm->DrawTitle(True);
  qvWm->ChangeFrameFocus();
  for (int i = 0; i < 3; i++)
    qvWm->fButton[i]->DrawButton();

  ASSERT(qvWm->tButton);
  qvWm->tButton->ResetFocus();
  qvWm->tButton->SetState(Button::NORMAL);
  qvWm->tButton->DrawButton();

  if (UsePager) {
    ASSERT(qvWm->mini);
    qvWm->mini->ResetFocus();
  }
}

/*
 * RollFocus --
 *   Roll focus stack: that is, the top becomes the bottom, and the second
 *   is the top.
 */
void FocusMgr::RollFocus(Bool forward)
{
  Qvwm* qvWm;

  if (forward) {
    // move this window to the last of the focus list
    qvWm = focusMgr.Pop();
    focusMgr.InsertBottom(qvWm);
    qvWm = focusMgr.Top();
  }
  else {
    // move the last of the focus list to the top of the stack
    qvWm = focusMgr.RemoveBottom();
    focusMgr.Push(qvWm);
  }

  ASSERT(qvWm->CheckMapped());

  qvWm->SetFocus();
  qvWm->RaiseWindow(True);
  qvWm->AdjustPage();
}

/*
 * Roll focus stack within the same screen, not including windows with
 * NOFOCUS or STICKY.
 */
void FocusMgr::RollFocusWithinScreen(Bool forward)
{
  Qvwm* qvWm;
  List<Qvwm>::Iterator i(&focusMgr.GetMapList());
  Rect vt(paging->origin.x, paging->origin.y, rcScreen.width, rcScreen.height);

  if (forward) {
    // move the top window to the bottom if it has a focus
    if (Qvwm::focusQvwm != rootQvwm && !Qvwm::focusQvwm->CheckFlags(STICKY) &&
	Intersect(Qvwm::focusQvwm->GetRect(), vt)) {
      qvWm = focusMgr.Pop();
      focusMgr.InsertBottom(qvWm);
    }
      
    // search the next window within the same screen
    for (qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
      if (!qvWm->CheckFlags(STICKY) && Intersect(qvWm->GetRect(), vt)) {
	i.Remove();
	focusMgr.Push(qvWm);
	break;
      }
    }
  }
  else {
    // search the next window within the same screen
    for (qvWm = i.GetTail(); qvWm; qvWm = i.GetPrev()) {
      if (!qvWm->CheckFlags(STICKY) && Intersect(qvWm->GetRect(), vt)) {
	i.Remove();
	focusMgr.Push(qvWm);
	break;
      }
    }
  }

  if (qvWm == NULL)
    qvWm = rootQvwm;

  ASSERT(qvWm->CheckMapped());

  qvWm->SetFocus();
  qvWm->RaiseWindow(True);
}

//////////////////////////////////////////////////////////////////////
// debug function
void FocusMgr::DumpStack()
{
  List<Qvwm>::Iterator i(&mapList);

  printf("FOCUS: %s\n", Qvwm::focusQvwm->GetName());

  printf("MAP LIST: ");
  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext())
    printf("%s -> ", qvWm->GetName());
  printf("\n");

  i = List<Qvwm>::Iterator(&unmapList);

  printf("UNMAP LIST: ");
  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext())
    printf("%s -> ", qvWm->GetName());
  printf("\n");

  printf("\n");
}

Bool FocusMgr::Check(Qvwm* qvWm)
{
  List<Qvwm>::Iterator i(&mapList);

  for (Qvwm* tmpQvwm = i.GetHead(); tmpQvwm; tmpQvwm = i.GetNext())
    if (tmpQvwm == qvWm)
      return False;
  
  return True;
}


