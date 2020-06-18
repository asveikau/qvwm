/*
 * focus.cc
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
#include "misc.h"
#include "qvwm.h"
#include "icon.h"
#include "util.h"
#include "qvwmrc.h"
#include "focus_mgr.h"
#include "pager.h"

/*
 * SetFocus --
 *   Take off the focus from current focus window, and set the focus to
 *   this window.
 */
void Qvwm::SetFocus()
{
  /*
   * Take off the focus of short cut icon, if any.
   */
  if (this != rootQvwm && Icon::focusIcon != NULL)
    Icon::focusIcon->ResetFocus();

  /*
   * If this window has already had the focus, return.
   */
  if (this == focusQvwm)
    return;

  // XXX not enter the focus stack if a NO_FOCUS window and
  // not give it a focus
  if (!CheckFlags(NO_FOCUS)) {
    focusMgr.ResetFocus(focusQvwm);
    focusMgr.SetFocus(this);
  }

  if (UseTaskbar && OnTopTaskbar)
    taskBar->RaiseTaskbar();
  if (UsePager && OnTopPager)
    pager->RaisePager();
}

/*
 * YieldFocus --
 *   Give a focus to another window and move this window to the last of
 *   focus list.
 */
void Qvwm::YieldFocus()
{
  // move this window to the bottom of the focus stack
  // do nothing if this window is not in the focus stack
  // (a NO_FUCUS or transient window)
  if (focusMgr.Remove(this))
    focusMgr.InsertBottom(this);

  if (this == focusQvwm) {
    Qvwm* tmpQvwm = focusMgr.Top();

    if (tmpQvwm == NULL || tmpQvwm == this)
      tmpQvwm = rootQvwm;
    
    if (ClickToFocus) {
      if (tmpQvwm->CheckMapped()) {
	tmpQvwm->SetFocus();
	tmpQvwm->RaiseWindow(True);
      }
      else
	tmpQvwm->RestoreWindow();
    }
    else
      rootQvwm->SetFocus();
  }
}

void Qvwm::SetFocusToActiveWindow()
{
  if (Qvwm::focusQvwm == rootQvwm)
    desktop.SetFocus();
  else
    XSetInputFocus(display, Qvwm::focusQvwm->GetWin(), RevertToParent,
		   CurrentTime);
}
