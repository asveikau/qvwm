/*
 * window.cc
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
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#if defined(sun) && !defined(__SVR4)
#include <sys/resource.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include "main.h"
#include "qvwm.h"
#include "misc.h"
#include "util.h"
#include "fbutton.h"
#include "tbutton.h"
#include "menu.h"
#include "qvwmrc.h"
#include "pager.h"
#include "mini.h"
#include "timer.h"
#include "list.h"
#include "focus_mgr.h"
#include "desktop.h"
#include "callback.h"
#include "gnome.h"
#include "paging.h"

Qvwm* Qvwm::activeQvwm = NULL;

/*
 * MapWindows --
 *   Map this window and the transient windows that should be mapped.
 */
void Qvwm::MapWindows(Bool mapTransient)
{
  if (CheckMapped())
    return;

  if (isFirstMapped) {
    RaiseWindow(True);
    isFirstMapped = False;
  }

  MapWindow(mapTransient);

  trMapped = True;

  RaiseWindow(True);

  if (!CheckFlags(NO_FOCUS) && !CheckFlags(TRANSIENT)) {
    focusMgr.RemoveUnmapList(this);
    focusMgr.Push(this);
  }
}

/*
 * MapWindow --
 *   Map window.
 */
void Qvwm::MapWindow(Bool mapTransient)
{
  mapped = True;

  if (CheckFlags(BORDER)) {
    for (int i = 0; i < 4; i++) {
      XMapWindow(display, side[i]);
      XMapWindow(display, corner[i]);
    }
  }
  else {
    for (int i = 0; i < 4; i++) {
      XUnmapWindow(display, side[i]);
      XUnmapWindow(display, corner[i]);
    }
  }

  if (CheckFlags(BORDER_EDGE)) {
    for (int i = 0; i < 4; i++)
      XMapWindow(display, edge[i]);
  }
  else {
    for (int i = 0; i < 4; i++)
      XUnmapWindow(display, edge[i]);
  }

  if (CheckFlags(TITLE)) {
    XMapRaised(display, title);
    if (CheckFlags(CTRL_MENU))
      XMapWindow(display, ctrl);
    else
      XUnmapWindow(display, ctrl);
    if (CheckFlags(BUTTON1))
      fButton[0]->MapButton();
    else
      fButton[0]->UnmapButton();
    if (CheckFlags(BUTTON2))
      fButton[1]->MapButton();
    else
      fButton[1]->UnmapButton();
    if (CheckFlags(BUTTON3)) {
      SetFlags(BUTTON3);
      fButton[2]->MapButton();
    }
    else
      fButton[2]->UnmapButton();
  }
  else
    XUnmapWindow(display, title);

  XMapRaised(display, parent);

  /*
   * Don't notify MapNotify to qvwm.
   */
  XWindowAttributes attr;
  long tmpMask;

  XGetWindowAttributes(display, wOrig, &attr);

  tmpMask = attr.your_event_mask &
    ~(StructureNotifyMask | SubstructureNotifyMask);
  XSelectInput(display, wOrig, tmpMask);
  XMapWindow(display, wOrig);
  XSelectInput(display, wOrig, attr.your_event_mask);

  if (GradWindowMapStyle != Normal)
    XResizeWindow(display, frame, 1, 1);

  XMapWindow(display, frame);

  if (GradWindowMapStyle != Normal)
    AnimateWindow(True);

  SetStateProperty(NormalState);

  if (UsePager) {
    ASSERT(mini);
    mini->MapMiniature();
  }

  if (mapTransient) {
    /*
     * When this is a main window, mapping unmapped transient windows due to
     * unmapping this window.
     */
    List<Qvwm>::Iterator li(&trList);
    Qvwm* qvWm = li.GetHead();
    
    while (qvWm) {
      if (qvWm->trMapped) {
	qvWm->MapWindow(mapTransient);
	Gnome::ResetState(qvWm, WIN_STATE_HID_TRANSIENT);
      }
      qvWm = li.GetNext();
    }
  }
}

/*
 * UnmapWindows --
 *   Unmap this window and the transient windows that are mapped.
 */
void Qvwm::UnmapWindows(Bool unmapTransient)
{
  if (!CheckMapped())
    return;

  /*
   * Unmap all menus.
   */
  if (CheckMenuMapped())
    UnmapMenu();

  if (GradWindowMapStyle != Normal)
    AnimateWindow(False);

  UnmapWindow(unmapTransient);

  trMapped = False;

  if (!CheckFlags(TRANSIENT)) {
    focusMgr.Remove(this);
    focusMgr.InsertUnmapList(this);
  }
}

void Qvwm::UnmapWindow(Bool unmapTransient)
{
  mapped = False;

  /*
   * Don't notify UnmapNotify to qvwm.
   */
  XWindowAttributes attr;
  long tmpMask;

  XGetWindowAttributes(display, wOrig, &attr);

  tmpMask = attr.your_event_mask &
    ~(StructureNotifyMask | SubstructureNotifyMask);
  XSelectInput(display, wOrig, tmpMask);
  XUnmapWindow(display, wOrig);
  XSelectInput(display, wOrig, attr.your_event_mask);

  XUnmapWindow(display, frame);

  if (UsePager) {
    ASSERT(mini);
    mini->UnmapMiniature();
  }

  if (unmapTransient) {
    /*
     * When this is a main window, unmapping all transient windows.
     */
    List<Qvwm>::Iterator li(&trList);
    Qvwm* qvWm = li.GetHead();
    
    while (qvWm) {
      if (qvWm->trMapped) {
	qvWm->UnmapWindow(unmapTransient);
	Gnome::SetState(qvWm, WIN_STATE_HID_TRANSIENT);
      }
      qvWm = li.GetNext();
    }
  }
}

/*
 * RaiseWindow --
 *   Raise this window on top, but keep transient windows to be on the top.
 *   If soon is False, called this routine later again.
 */
void Qvwm::RaiseWindow(Bool soon)
{
  if (this == rootQvwm)
    return;

  if (!soon && AutoRaise && AutoRaiseDelay > 0)
    timer->SetTimeout(AutoRaiseDelay,
		      new Callback<Qvwm>(this, &Qvwm::RaiseSoon));
  else if (CheckMapped()) {
    Window* wins;
    Miniature** minis;
    int nwindows = 0;
    int maxNum;
    Bool already = False;

    maxNum = desktop.GetQvwmList().GetSize();

    wins = new Window[maxNum + 1];
    minis = new Miniature*[maxNum];

    wins[0] = desktop.GetTopWindow();

    if (CheckFlags(ONTOP))
      StackWindows(wins, minis, &nwindows);
    else {
      /*
       * In the most top, ONTOP windows is located.
       */
      List<Qvwm>::Iterator li(&desktop.GetOnTopList());
      Qvwm* qvWm = li.GetTail();
    
      while (qvWm) {
	if (qvWm->CheckMapped())
	  qvWm->StackWindows(wins, minis, &nwindows);
	if (qvWm == this)
	  already = True;
	qvWm = li.GetPrev();
      }

      /*
       * If this window is not included in ontop window list, locate it.
       */
      if (!already)
	StackWindows(wins, minis, &nwindows);
    }

    XRestackWindows(display, wins, nwindows + 1);
    if (UsePager) {
      ASSERT(minis[0]);
      minis[0]->RaiseMiniature();
      Miniature::RestackMiniatures(minis, nwindows);
    }

    delete [] wins;
    delete [] minis;

/*
    if (UseTaskbar && OnTopTaskbar)
      taskBar->RaiseTaskbar();
    if (UsePager && OnTopPager)
      pager->RaisePager();
*/

    if (this != activeQvwm)
      activeQvwm = this;
  }
}

void Qvwm::StackWindows(Window* wins, Miniature** minis, int *nwindows)
{
  /*
   * First locate the transient windows of this window.
   */
  List<Qvwm>::Iterator li(&trList);
  Qvwm* qvWm = li.GetHead();

  while (qvWm) {
    if (qvWm->CheckMapped())
      qvWm->StackWindows(wins, minis, nwindows);
    qvWm = li.GetNext();
  }

  /*
   * Next locate this window.
   */
  ASSERT(*nwindows < desktop.GetQvwmList().GetSize())

  wins[*nwindows + 1] = frame;
  if (UsePager)
    minis[*nwindows] = mini;
  (*nwindows)++;
}

/*
 * RaiseSoon --
 *   Raise soon if the window is raisable.
 */
void Qvwm::RaiseSoon()
{
  if (focusQvwm == this && CheckMapped() && !CheckMenuMapped())
    RaiseWindow(True);
}

/*
 * LowerWindow --
 *   Lower this window, but keep it above the icons.
 *   This is achieved by searching the lowest "real" window and putting this
 *   just below it.
 */
void Qvwm::LowerWindow()
{
  if (this == rootQvwm)
    return;
  
  if (CheckFlags(ONTOP))
    return;

  if (CheckMapped()) {
    Window junkRoot, junkParent, *children;
    unsigned int nChildren;

    // XQueryTree returns windows in correct stacking order, lowest first
    XQueryTree(display, root, &junkRoot, &junkParent, &children, &nChildren);

    Qvwm* qvWm;
    int j;
    
    for (j = 0; j < (int)nChildren; j++) {
      if (XFindContext(display, children[j], context, (caddr_t*)&qvWm)
	  == XCSUCCESS) {
         if (qvWm == this) // this is already lowest window
           return;

         break; // found a "real" window
       }
    }

    if (nChildren)
      XFree(children);

    // not found
    if (j == (int)nChildren)
      return;

    Window wins[2];
    wins[0] = qvWm->GetFrameWin();
    wins[1] = frame;
    XRestackWindows(display, wins, 2);

    if (UsePager) {
      // find lowest pager mini window to place this mini window below it
      XQueryTree(display, pager->GetPagesWin(), &junkRoot, &junkParent,
		 &children, &nChildren);
      ASSERT(nChildren >= 2);
      ASSERT(children[0] == pager->GetVisualWin()); // lowest should be visual
      wins[0] = children[1];
      wins[1] = mini->GetWin();
      XRestackWindows(display, wins, 2);
      XFree(children);
    }
  }
}

/*
 * SetShape --
 *   Transparentize the shaped window.
 */
void Qvwm::SetShape()
{
#ifdef USE_SHAPE
  int borderWidth, topBorder, titleHeight, titleEdge;

  GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

  /*
   * Transparentize the original window.
   */
  XShapeCombineShape(display, frame, ShapeBounding,
		     borderWidth, topBorder + titleHeight + titleEdge,
		     wOrig, ShapeBounding, ShapeSet);

  if (CheckFlags(TITLE)) {
    XRectangle rect;

    rect.x = topBorder;
    rect.y = topBorder;
    rect.width = rc.width - topBorder * 2;
    rect.height = titleHeight;
    
    XShapeCombineRectangles(display, frame, ShapeBounding, 0, 0, &rect, 1,
			    ShapeUnion, Unsorted);
  }

  if (CheckFlags(BORDER)) {
    XRectangle rect[4];

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width = rc.width;
    rect[0].height = topBorder + titleHeight + titleEdge;

    rect[1].x = 0;
    rect[1].y = topBorder + titleHeight + titleEdge;
    rect[1].width = borderWidth;
    rect[1].height = rc.height - (borderWidth + topBorder + titleHeight
				  + titleEdge);

    rect[2].x = 0;
    rect[2].y = rc.height - borderWidth;
    rect[2].width = rc.width;
    rect[2].height = borderWidth;
    
    rect[3].x = rc.width - borderWidth;
    rect[3].y = topBorder + titleHeight + titleEdge;
    rect[3].width = borderWidth;
    rect[3].height = rc.height - (borderWidth + topBorder + titleHeight +
				  titleEdge);

    XShapeCombineRectangles(display, frame, ShapeBounding, 0, 0, rect, 4,
			    ShapeUnion, Unsorted);
  }
#endif // USE_SHAPE
}

void Qvwm::AnimateWindow(Bool map)
{
  int i;
  int width, height;

  switch (GradWindowMapStyle) {
  case TopToBottom:
    for (i = 1; i <= GradWindowMapSpeed; i++) {
      if (map)
	height = rc.height * i / GradWindowMapSpeed;
      else
	height = rc.height * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;

      if (height == 0)
	height = 1;

      XResizeWindow(display, frame, rc.width, height);

      XFlush(display);
      usleep(10000);
    }
    break;

  case LeftToRight:
    for (i = 1; i <= GradWindowMapSpeed; i++) {
      if (map)
	width = rc.width * i / GradWindowMapSpeed;
      else
	width = rc.width * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;

      if (width == 0)
	width = 1;

      XResizeWindow(display, frame, width, rc.height);

      XFlush(display);
      usleep(10000);
    }
    break;

  case CenterToTopBottom:
    for (i = 1; i <= GradWindowMapSpeed; i++) {
      if (map)
	height = rc.height * i / GradWindowMapSpeed;
      else
	height = rc.height * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;

      if (height == 0)
	height = 1;

      int y = rc.y + (rc.height - height) / 2;
      
      XMoveResizeWindow(display, frame,
			rc.x - paging->origin.x, y - paging->origin.y,
			rc.width, height);

      XFlush(display);
      usleep(10000);
    }
    break;

  case CenterToLeftRight:
    for (i = 1; i <= GradWindowMapSpeed; i++) {
      if (map)
	width = rc.width * i / GradWindowMapSpeed;
      else
	width = rc.width * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;

      if (width == 0)
	width = 1;

      int x = rc.x + (rc.width - width) / 2;
      
      XMoveResizeWindow(display, frame,
			x - paging->origin.x, rc.y - paging->origin.y,
			width, rc.height);

      XFlush(display);
      usleep(10000);
    }
    break;

  case CenterToAll:
    for (i = 1; i <= GradWindowMapSpeed; i++) {
      if (map) {
	width = rc.width * i / GradWindowMapSpeed;
	height = rc.height * i / GradWindowMapSpeed;
      }
      else {
	width = rc.width * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;
	height = rc.height * (GradWindowMapSpeed - i + 1) / GradWindowMapSpeed;
      }

      int x = rc.x + (rc.width - width) / 2;
      int y = rc.y + (rc.height - height) / 2;

      XMoveResizeWindow(display, frame,
			x - paging->origin.x, y - paging->origin.y,
			width, height);

      XFlush(display);
      usleep(10000);
    }

  case Normal:
    break;
  }
}
