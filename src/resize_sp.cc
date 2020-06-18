/*
 * resize_sp.cc
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
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "event.h"
#include "taskbar.h"
#include "fbutton.h"
#include "tbutton.h"
#include "qvwmrc.h"
#include "paging.h"
#include "pager.h"
#include "mini.h"
#include "desktop.h"
#include "gnome.h"

/*
 * MaximizeWindow --
 *   Maximize the window.
 */
void Qvwm::MaximizeWindow(Bool motion, Bool mapTransient)
{
  Window junkRoot;
  unsigned int junkBW, junkDepth;
  Rect rcSrc, rcDest, rect;

  if (this == rootQvwm)
    return;
  if (CheckStatus(MAXIMIZE_WINDOW))
    return;

  XGrabServer(display);

  XGetGeometry(display, frame, &junkRoot, &rcSrc.x, &rcSrc.y,
	       (unsigned int *)&rcSrc.width, (unsigned int *)&rcSrc.height,
	       &junkBW, &junkDepth);

  ASSERT(paging);

  if (CheckStatus(MINIMIZE_WINDOW)) {
    /*
     * min -> max
     */
    SetStateProperty(NormalState);

    if (UseTaskbar) {
      ASSERT(tButton);
      rcSrc = tButton->GetRect();
      rect = taskBar->GetRect();
      rcSrc.x += rect.x;
      rcSrc.y += rect.y;
    }
  }
  else {
    /*
     * normal -> max
     */
    rcSrc = rc;
    rcSrc.x -= paging->origin.x;
    rcSrc.y -= paging->origin.y;
    flagsBak = flags;

    // XXX
    for (int i = 0; i < 4; i++) {
      XUnmapWindow(display, side[i]);
      XUnmapWindow(display, corner[i]);
    }
  }

  ResetFlags(BORDER);

  rcDest = GetFixSize(rcScreen, hints.max_width, hints.max_height,
		      hints.width_inc, hints.height_inc,
		      hints.base_width, hints.base_height);
  rc = rcDest;
  rc.x += paging->origin.x;
  rc.y += paging->origin.y;

  rcOrigBak = rcOrig;

  SetStatus(MAXIMIZE_WINDOW);
  ResetStatus(MINIMIZE_WINDOW);

  if (UseTaskbar && motion) {
    PlaySound(MaximizeSound);
    MotionTitlebar(rcSrc, rcDest);
  }

  ConfigureNewRect(rc);

  if (CheckFlags(THIN_BORDER))
    XSetWindowBorderWidth(display, frame, 0);

  RedrawWindow();
  MapWindows(mapTransient);
  SetFocus();

  Gnome::SetState(this, WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ);

  XUngrabServer(display);
}

/*
 * MinimizeWindow --
 *   Minimize the window.
 */
void Qvwm::MinimizeWindow(Bool motion, Bool unmapTransient)
{
  Rect rcSrc, rcDest, rect;

  if (this == rootQvwm)
    return;
  if (CheckStatus(MINIMIZE_WINDOW))
    return;

  // cannot minimize without taskbar button
  if (CheckFlags(NO_TBUTTON))
    return;

  XGrabServer(display);

  if (CheckStatus(MAXIMIZE_WINDOW)) {
    /*
     * max->min
     */
    rcSrc = GetFixSize(rcScreen, hints.max_width, hints.max_height,
		       hints.width_inc, hints.height_inc,
		       hints.base_width, hints.base_height);
  }
  else {
    ASSERT(paging);
    /*
     * normal->min
     */
    rcSrc = rc;
    rcSrc.x -= paging->origin.x;
    rcSrc.y -= paging->origin.y;
    flagsBak = flags;
  }

  if (UseTaskbar) {
    ASSERT(tButton);
    rcDest = tButton->GetRect();
    rect = taskBar->GetRect();
    rcDest.x += rect.x;
    rcDest.y += rect.y;
  }

  SetStatus(MINIMIZE_WINDOW);
  SetStateProperty(IconicState);

  if (UseTaskbar && motion) {
    PlaySound(MinimizeSound);
    MotionTitlebar(rcSrc, rcDest);
  }

  YieldFocus();
  UnmapWindows(unmapTransient);

  if (!ClickToFocus)
    desktop.ChangeFocusToCursor();
  
  Gnome::SetState(this, WIN_STATE_MINIMIZED);

  // dummy use of an icon window spcified by application (for Tgif)
  if (wmHints && (wmHints->flags & IconWindowHint) &&
      wmHints->icon_window != None) {
    Rect rcRoot = rootQvwm->GetRect();
    XMoveWindow(display, wmHints->icon_window, rcRoot.width, rcRoot.height);
    XMapWindow(display, wmHints->icon_window);
  }

  XUngrabServer(display);
}

/*
 * RestoreWindow --
 *   Restore the window to previous state.
 */
void Qvwm::RestoreWindow(Bool motion, Bool mapTransient)
{
  Rect rcSrc, rcDest, rect;

  if (this == rootQvwm)
    return;
  if (!CheckStatus(MAXIMIZE_WINDOW | MINIMIZE_WINDOW))
    return;

  XGrabServer(display);

  if (CheckStatus(MINIMIZE_WINDOW)) {
    // unmap dummy icon window (for Tgif)
    if (wmHints && (wmHints->flags & IconWindowHint) &&
	wmHints->icon_window != None)
      XUnmapWindow(display, wmHints->icon_window);
  }

  ASSERT(paging);

  if (CheckStatus(MINIMIZE_WINDOW) && CheckStatus(MAXIMIZE_WINDOW)) {
    /*
     * min -> max
     */
    ResetStatus(MINIMIZE_WINDOW);
    SetStateProperty(NormalState);

    if (UseTaskbar) {
      ASSERT(tButton);
      rcSrc = tButton->GetRect();
      rect = taskBar->GetRect();
      rcSrc.x += rect.x;
      rcSrc.y += rect.y;
    }
    rcDest = GetFixSize(rcScreen, hints.max_width, hints.max_height,
			hints.width_inc, hints.height_inc,
			hints.base_width, hints.base_height);
    rc = rcDest;

    rc.x += paging->origin.x;
    rc.y += paging->origin.y;

    ResetFlags(BORDER);

    if (UseTaskbar && motion) {
      PlaySound(RestoreUpSound);
      MotionTitlebar(rcSrc, rcDest);
    }
  }
  else if (CheckStatus(MINIMIZE_WINDOW)) {
    /*
     * min -> normal
     */
    ResetStatus(MAXIMIZE_WINDOW | MINIMIZE_WINDOW);
    SetStateProperty(NormalState);

    if (UseTaskbar) {
      ASSERT(tButton);
      rcSrc = tButton->GetRect();
      rect = taskBar->GetRect();
      rcSrc.x += rect.x;
      rcSrc.y += rect.y;
    }
    flags = flagsBak;
    CalcWindowPos(rcOrig);

    rcDest = rc;
    rcDest.x -= paging->origin.x;
    rcDest.y -= paging->origin.y;

    if (UseTaskbar && motion) {
      PlaySound(RestoreUpSound);
      MotionTitlebar(rcSrc, rcDest);
    }
  }
  else if (CheckStatus(MAXIMIZE_WINDOW)) {
    /*
     * max -> normal
     */
    ResetStatus(MAXIMIZE_WINDOW | MINIMIZE_WINDOW);

    rcSrc = GetFixSize(rcScreen, hints.max_width, hints.max_height,
		       hints.width_inc, hints.height_inc,
		       hints.base_width, hints.base_height);
    flags = flagsBak;
    rcOrig = rcOrigBak;
    CalcWindowPos(rcOrig);

    rcDest = rc;
    rcDest.x -= paging->origin.x;
    rcDest.y -= paging->origin.y;

    // XXX
    for (int i = 0; i < 4; i++) {
      XMapWindow(display, side[i]);
      XMapWindow(display, corner[i]);
    }

    if (UseTaskbar && motion) {
      PlaySound(RestoreDownSound);
      MotionTitlebar(rcSrc, rcDest);
    }
  }

  ConfigureNewRect(rc);

  if (CheckFlags(THIN_BORDER))
    XSetWindowBorderWidth(display, frame, 1);

  RedrawWindow();
  MapWindows(mapTransient);
  SetFocus();
  AdjustPage();

  Gnome::ResetState(this, WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ
		          | WIN_STATE_MINIMIZED);

  XUngrabServer(display);
}

/*
 * ExpandWindow --
 *   Maximize the window without obscuring windows that aren't already
 *   obscured.
 */
void Qvwm::ExpandWindow(int dir, Bool motion)
{
  if (this == rootQvwm)
    return;

  RectPt rpScr(paging->origin.x + rcScreen.x,
	       paging->origin.y + rcScreen.y,
	       paging->origin.x + rcScreen.x + rcScreen.width - 1,
	       paging->origin.y + rcScreen.y + rcScreen.height - 1);
  
  if (!Intersect(rc, Rect(rpScr)))
    return;

  XGrabServer(display);
  ASSERT(paging);
  
  Rect rcOld = rc, rect;
  RectPt rpOrig = (RectPt)rc;
  RectPt rpNew((dir & EXPAND_LEFT)  ? rpScr.left   : rc.x,
	       (dir & EXPAND_UP)    ? rpScr.top    : rc.y,
	       (dir & EXPAND_RIGHT) ? rpScr.right  : rc.x + rc.width - 1,
	       (dir & EXPAND_DOWN)  ? rpScr.bottom : rc.y + rc.height - 1);
  List<Qvwm>::Iterator i(&desktop.GetQvwmList());

  // check for all windows
  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
    if (qvWm->CheckStatus(MINIMIZE_WINDOW) || !qvWm->CheckMapped())
      continue;

    FixExpandedShape(dir, (RectPt)qvWm->GetRect(), rpOrig, rpNew);
  }

  // check for taskbar
  if (UseTaskbar) {
    rect = taskBar->GetRect();
    rect.x += paging->origin.x;
    rect.y += paging->origin.y;
    FixExpandedShape(dir, (RectPt)rect, rpOrig, rpNew);
  }
  
  // check for pager
  if (UsePager) {
    rect = pager->GetRect();
    rect.x += paging->origin.x;
    rect.y += paging->origin.y;
    FixExpandedShape(dir, (RectPt)rect, rpOrig, rpNew);
  }

  rc = GetFixSize((Rect)rpNew, hints.max_width, hints.max_height,
		  hints.width_inc, hints.height_inc,
		  hints.base_width, hints.base_height);

  if (UseTaskbar && motion) {
    PlaySound(ExpandSound);
    MotionTitlebar(rcOld, rc);
  }

  ConfigureNewRect(rc);

  RedrawWindow();
  MapWindows(True);
  SetFocus();

  XUngrabServer(display);
}

/*
 * Calculate new rpNew from rcTest and rpOrig.
 */
void Qvwm::FixExpandedShape(int directions, const RectPt& rpTest,
			    const RectPt& rpOrig, RectPt& rpNew)
{
  if (directions & EXPAND_LEFT)
    if (rpTest.right <= rpOrig.left && rpTest.bottom > rpOrig.top &&
	rpTest.top < rpOrig.bottom)
      rpNew.left = Max(rpNew.left, rpTest.right + 1);

  if (directions & EXPAND_RIGHT)
    if (rpOrig.right <= rpTest.left && rpTest.bottom > rpOrig.top &&
	rpTest.top < rpOrig.bottom)
      rpNew.right = Min(rpNew.right, rpTest.left + 1);

  if (directions & EXPAND_UP)
    if (rpTest.bottom <= rpOrig.top && rpTest.right > rpOrig.left &&
	rpTest.left < rpOrig.right)
      rpNew.top = Max(rpNew.top, rpTest.bottom + 1);

  if (directions & EXPAND_DOWN)
    if (rpOrig.bottom <= rpTest.top && rpTest.right > rpOrig.left &&
	rpTest.left < rpOrig.right)
      rpNew.bottom = Min(rpNew.bottom, rpTest.top + 1);
}
