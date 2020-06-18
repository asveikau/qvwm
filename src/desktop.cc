/*
 * desktop.cc
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
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "desktop.h"
#include "focus_mgr.h"
#include "paging.h"
#include "icon.h"
#include "indicator.h"
#include "taskbar.h"
#include "startmenu.h"
#include "switcher.h"
#include "pager.h"
#include "exit_dialog.h"
#include "confirm_dialog.h"
#include "hash.h"
#include "qvwmrc.h"
#include "gnome.h"
#include "mini.h"
#include "image.h"
#include "callback.h"
#include "accessory.h"
#include "session.h"
#include "fbutton.h"
#include "screen_saver.h"
#include "remote_cmd.h"

Desktop::Desktop()
{
  initPos = 0;
  imgDesktop = NULL;
  curCmap = None;
  curCmapQvwm = NULL;
}

/*
 * LookInList -- 
 *   Return the pointer if the window exists in qvwm list;
 *   otherwise, return NULL.
 */
Qvwm* Desktop::LookInList(Window w)
{
  List<Qvwm>::Iterator i(&qvwmList);
  Qvwm* qvWm;

  for (qvWm = i.GetHead(); qvWm; qvWm = i.GetNext())
    if (w == qvWm->GetWin())
      break;

  if (qvWm) {
    qvWm->CalcWindowPos(qvWm->GetOrigRect());
    Rect rect = qvWm->GetRect();

    if (Intersect(rect, rootQvwm->GetRect()))
      Gnome::ResetState(qvWm, WIN_STATE_HID_WORKSPACE);
    else
      Gnome::SetState(qvWm, WIN_STATE_HID_WORKSPACE);
  }

  return qvWm;
}

/*
 * CaptureAllWindows --
 *   Decorate all windows.
 */
void Desktop::CaptureAllWindows()
{
  Qvwm* qvWm;
  Window junkRoot, junkParent, *children;
  unsigned int nChildren;
  long state;
  XWMHints* wmHints;

  if (!XQueryTree(display, root, &junkRoot, &junkParent, &children,
		  &nChildren))
    return;

  for (int i = 0; i < (int)nChildren; i++) {
    if (children[i] && MappedNotOverride(children[i], &state)) {
      qvWm = new Qvwm(children[i], True);
      qvwmList.InsertTail(qvWm);

      if (qvWm->CheckFlags(CLOSE_SOON)) {
	XReparentWindow(display, children[i], root,
			rcScreen.x + rcScreen.width,
			rcScreen.y + rcScreen.height);
	qvWm->CloseWindow();
	qvwmList.Remove(qvWm);
	delete qvWm;
	continue;
      }

      wmHints = qvWm->GetWMHints();

      if (state == IconicState)
        qvWm->MinimizeWindow(False, False);
      else if (wmHints && (wmHints->flags & StateHint) && 
               wmHints->initial_state == IconicState)
        qvWm->MinimizeWindow(False, False);
      else if (qvWm->CheckFlags(INIT_MINIMIZE))
        qvWm->MinimizeWindow(False, False);
      else if (qvWm->CheckFlags(INIT_MAXIMIZE)) {
        qvWm->fButton[1]->ChangeImage(FrameButton::RESTORE);
        qvWm->MaximizeWindow(False, False);
      }
      else
        qvWm->MapWindows();

      ASSERT(qvWm->tButton);
      qvWm->tButton->MapButton();

      if (!qvWm->CheckStatus(MINIMIZE_WINDOW)) {
	if (ClickToFocus)
	  qvWm->SetFocus();
      }
      else
	focusMgr.InsertUnmapList(qvWm);
    }
  }

  Gnome::ChangeClientList();

  ChangeFocusToCursor();

  if (nChildren)
    XFree(children);
}      

/*
 * Decide placement of a window with size.
 * SmartPlacement seems to have a bug...
 */
Point Desktop::GetNextPlace(const Dim& size, const Point& pageoff)
{
  Point pt;

  if (SmartPlacement) {
    List<Qvwm>::Iterator i(&qvwmList);
    Qvwm* tmpQvwm;
    int miny;
    Rect rcTmp;
    Bool found;

    for (pt.y = pageoff.y + rcScreen.y;
	 pt.y + size.height < pageoff.y + rcScreen.y + rcScreen.height;
	 pt.y = miny) {
      miny = pageoff.y + rcScreen.y + rcScreen.height;

      for (pt.x = pageoff.x + rcScreen.x;
	   pt.x + size.width < pageoff.x + rcScreen.x + rcScreen.width;
	   pt.x++) {
	// assume this position is ok
	found = True;

	for (tmpQvwm = i.GetHead(); tmpQvwm; tmpQvwm = i.GetNext()) {
	  if (!(tmpQvwm->CheckStatus(MINIMIZE_WINDOW)) &&
	      tmpQvwm->CheckMapped()) {
	    rcTmp = tmpQvwm->GetRect();

	    if (rcTmp.x < pt.x + size.width &&
		rcTmp.x + rcTmp.width > pt.x &&
		rcTmp.y < pt.y + size.height &&
		rcTmp.y + rcTmp.height > pt.y) {
	      found = False; // overlap
	      pt.x = rcTmp.x + rcTmp.width - 1; 
	      miny = Min(miny, rcTmp.y + rcTmp.height);
	      break;
	    }
	  }
	}
	if (found)
	  return pt;
      }
    }
  }

  // when not SmartPlacement or not found
  if (initPos + size.width + Qvwm::BORDER_WIDTH * 2 > rcScreen.width ||
      initPos + size.height + Qvwm::TITLE_HEIGHT + Qvwm::TOP_BORDER
      + Qvwm::BORDER_WIDTH > rcScreen.height)
    initPos = 0;
  
  pt.x = initPos + rcScreen.x + pageoff.x;
  pt.y = initPos + rcScreen.y + pageoff.y;
  initPos += Qvwm::TITLE_HEIGHT + Qvwm::TOP_BORDER;

  return pt;
}

/*
 * RecalcAllWindows --
 *   Recalculate the position of all windows.
 */
void Desktop::RecalcAllWindows()
{
  List<Qvwm>::Iterator i(&qvwmList);

  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext())
    qvWm->RecalcWindow();
}

static void TrapChild2(int sig);
static int FinishErrorHandler(Display* display, XErrorEvent* ev);

/*
 * FinishQvwm --
 *   Free all resources and finish qvwm.
 */
void Desktop::FinishQvwm(Bool reStart)
{
  // when parse error before a start flag is set
  if (!start) {
    XCloseDisplay(display);
    return;
  }

  ASSERT(paging);

  /*
   * Restore default page.
   */
  paging->PagingProc(Point(0, 0));

  XGrabServer(display);

  signal(SIGCHLD, TrapChild2);
  XSetErrorHandler(FinishErrorHandler);

  /*
   * Delete all qvwms.
   */
  List<Qvwm>::Iterator i1(&qvwmList);

  for (Qvwm* qvWm = i1.GetHead(); qvWm; qvWm = i1.Remove()) {
    XWindowAttributes attr;
    Rect rcOrig = qvWm->GetOrigRect();

    XGetWindowAttributes(display, qvWm->GetWin(), &attr);

    if (attr.map_state != IsUnmapped || qvWm->CheckStatus(MINIMIZE_WINDOW))
      XReparentWindow(display, qvWm->GetWin(), root, rcOrig.x, rcOrig.y);
    else {
      /*
       * Adjust the position of a window unmapped by an application,
       * preparing for the next map.
       */
      Rect rcRoot = rootQvwm->GetRect();
      XReparentWindow(display, qvWm->GetWin(), root,
		      rcOrig.x % rcRoot.width, rcOrig.y % rcRoot.height);
    }

    XSetWindowBorderWidth(display, qvWm->GetWin(), qvWm->GetOrigBW());

    delete qvWm;
  }

  XUngrabServer(display);

  // Delete all icons.
  List<Icon>::Iterator i2(&iconList);
  for (Icon* icon = i2.GetHead(); icon; icon = i2.Remove())
    delete icon;

  // Delete all accessories.
  List<Accessory>::Iterator i4(&accList);
  for (Accessory* acc = i4.GetHead(); acc; acc = i4.Remove())
    delete acc;

  // Delete all indicators.
  List<Indicator>::Iterator i3(&Indicator::indList);
  for (Indicator* ind = i3.GetHead(); ind; ind = i3.Remove())
    delete ind;

  // free fonts
  int fsNum = (UseBoldFont) ? FsNum : FsNum - 1;
  for (int i = 0; i < fsNum; i++) {
    if (*fsSet[i].fs == fsDefault)
      continue;

    Bool match = False;
    for (int j = 0; j < i; j++) {
      if (*fsSet[i].fs == *fsSet[j].fs) {
	match = True;
	break;
      }
    }
    if (!match)
      XFreeFontSet(display, *fsSet[i].fs);
  }
  XFreeFontSet(display, fsDefault);

#ifdef USE_XSMP
  delete session;
#endif
  delete rootQvwm;
  delete taskBar;
  delete startMenu;
  delete taskSwitcher;
  delete paging;
  delete pager;
  delete ::ctrlMenu;
  delete exitDlg;
  delete confirmDlg;
  delete Qvwm::appHashTable;
#ifdef USE_SS
  delete scrSaver;
#endif
#ifdef ALLOW_RMTCMD
  delete remoteCmd;
#endif

  QvImage::Destroy(imgDesktop);

  XSetInputFocus(display, root, RevertToNone, CurrentTime);

  XUndefineCursor(display, root);

  XClearWindow(display, root);

  if (!reStart) {
    PlaySound(SystemExitSound, True);
    XCloseDisplay(display);
    exit(0);
  }
  else
    XCloseDisplay(display);
}

static void TrapChild2(int sig)
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  signal(SIGCHLD, TrapChild2);
}

/*
 * FinishErrorHandler --
 *   Jump to here when error happens in FinishQvwm. This error is ignored.
 */
static int FinishErrorHandler(Display* display, XErrorEvent* ev)
{
  return 0;
}

/*
 * ChangeFocusToCursor --
 *   Give the focus to the window where cursor exists.
 */
void Desktop::ChangeFocusToCursor()
{
  Window junkRoot, junkChild, w;
  Point ptRoot, ptJunk;
  unsigned int mask;
  Qvwm* qvWm;

  XQueryPointer(display, root, &junkRoot, &junkChild, &ptRoot.x, &ptRoot.y,
		&ptJunk.x, &ptJunk.y, &mask);
  XTranslateCoordinates(display, root, root, ptRoot.x, ptRoot.y,
			&ptJunk.x, &ptJunk.y, &w);

  if (XFindContext(display, w, Qvwm::context, (caddr_t *)&qvWm) == XCSUCCESS) {
    qvWm->SetFocus();
    if (AutoRaise)
      qvWm->RaiseWindow(False);
  }
  else
    rootQvwm->SetFocus();  /* w == None, i.e. cursor on the root window */
}

/*
 * CreateIcons --
 *   Create shortcut icons.
 */
void Desktop::CreateIcons()
{
  if (ShortCutItem == NULL)
    return;

  Icon* icon;
  QvImage* img;
  MenuElem* mItem = ShortCutItem;
  MenuElem* tmpItem;

  while (mItem) {
    if (strcmp(mItem->file, "") == 0)
      img = Icon::imgIcon->Duplicate();
    else {
      img = CreateImageFromFile(mItem->file, timer);
      if (img == NULL)
	img = Icon::imgIcon->Duplicate();
    }

    if (strcmp(mItem->exec, "") != 0)
      icon = new Icon(img, mItem->name, mItem->exec, mItem->x, mItem->y);
    else
      icon = new Icon(img, mItem->name, mItem->func, mItem->x, mItem->y);
    desktop.GetIconList().InsertTail(icon);

    icon->MoveIcon(Point(-(IconSize + IconHorizontalSpacing),
			 -(IconSize + IconVerticalSpacing)));
    icon->MapIcon();

    tmpItem = mItem;
    mItem = mItem->next;
    
    delete tmpItem;
  }
}

/*
 * RedrawAllIcons --
 *   Redraw all icons in icon list.
 */
void Desktop::RedrawAllIcons()
{
  List<Icon>::Iterator i(&iconList);
  int count = 0;
  int iconColumnNum;

  iconColumnNum = rcScreen.height / (IconSize + IconVerticalSpacing);

  for (Icon* tmpIcon = i.GetHead(); tmpIcon; tmpIcon = i.GetNext()) {
    Rect rcVirt = tmpIcon->GetVirtRect();

    if (rcVirt.x == -1)
      rcVirt.x = (IconSize + IconHorizontalSpacing) * (count / iconColumnNum);
    else if (rcVirt.x >= Icon::SFACTOR)
      rcVirt.x = (IconSize + IconHorizontalSpacing) * (rcVirt.x - Icon::SFACTOR);

    if (rcVirt.y == -1)
      rcVirt.y = (IconSize + IconVerticalSpacing) * (count % iconColumnNum);
    else if (rcVirt.y >= Icon::SFACTOR)
      rcVirt.y = (IconSize + IconVerticalSpacing) * (rcVirt.y - Icon::SFACTOR);

    tmpIcon->SetVirtRect(rcVirt);

    Rect rc = tmpIcon->GetRect();
    rc.x = rcScreen.x + rcVirt.x;
    rc.y = rcScreen.y + rcVirt.y;
    tmpIcon->SetRect(rc);
    
    tmpIcon->MoveIcon(Point(rc.x, rc.y));

    count++;
  }
}

/*
 * LineUpAllIcons --
 *   Line up all icons in icon list.
 */
void Desktop::LineUpAllIcons()
{
  List<Icon>::Iterator i(&iconList);
  int modulo = 0;

  for (Icon* tmpIcon = i.GetHead(); tmpIcon; tmpIcon = i.GetNext()) {
    Rect rcVirt = tmpIcon->GetVirtRect();

    modulo = rcVirt.x % (IconSize + IconHorizontalSpacing);
    rcVirt.x += (modulo > (IconSize + IconHorizontalSpacing) / 2) ?
      IconSize + IconHorizontalSpacing - modulo : -modulo;
    modulo = rcVirt.y % (IconSize + IconVerticalSpacing);
    rcVirt.y += (modulo > (IconSize + IconVerticalSpacing) / 2) ?
      IconSize + IconVerticalSpacing - modulo : -modulo;
  
    tmpIcon->SetVirtRect(rcVirt);

    Rect rc = tmpIcon->GetRect();
    rc.x = rcScreen.x + rcVirt.x;
    rc.y = rcScreen.y + rcVirt.y;
    tmpIcon->SetRect(rc);

    tmpIcon->MoveIcon(Point(rc.x, rc.y));
  }
}

/*
 * GetDesktopArea --
 *   Devide screen to 4 areas of up, down, left and right, and return the
 *   position where a given point belongs to.
 */
Taskbar::TaskbarPos Desktop::GetDesktopArea(const Point& pt)
{
  Rect rcRoot = rootQvwm->GetRect();
  int x1, x2;
  int y1, y2;

  x1 = int(double(rcRoot.width - 1) / (rcRoot.height - 1) * pt.y);
  x2 = (rcRoot.width - 1) - x1;

  if (x2 < pt.x && pt.x < x1)
    return Taskbar::BOTTOM;
  else if (x1 < pt.x && pt.x < x2)
    return Taskbar::TOP;

  y1 = int(double(rcRoot.height - 1) / (rcRoot.width - 1) * pt.x);
  y2 = (rcRoot.height - 1) - y1;

  if (y1 < pt.y && pt.y < y2)
    return Taskbar::LEFT;
  else if (y2 < pt.y && pt.y < y1)
    return Taskbar::RIGHT;

  return Taskbar::BOTTOM;
}

/*
 * SetWallPaper --
 *   Read wall paper(pixmap only), and make it be background of root window.
 */
void Desktop::SetWallPaper()
{
  if (strcmp(WallPaper, "Windows2000") == 0 ||
      strcmp(WallPaper, "Windows98") == 0 ||
      strcmp(WallPaper, "Windows95") == 0) {
    XSetWindowBackground(display, root, DesktopColor.pixel);
    XClearWindow(display, root);
  }
  else if (strcmp(WallPaper, "") != 0) {
    imgDesktop = CreateImageFromFile(WallPaper, timer);
    if (imgDesktop) {
      imgDesktop->SetBackground(root);
      XClearWindow(display, root);
    }
  }
}

// before creating accessories
void Desktop::CreateTopWindow()
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.override_redirect = True;
  valueMask = CWOverrideRedirect;

  topWin = XCreateWindow(display, root,
			 -1, -1, 1, 1,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);

  XMapRaised(display, topWin);
}

void Desktop::CreateAccessories()
{
  List<Accessory>::Iterator li(&accList);
  Accessory* acc = li.GetHead();

  while (acc) {
    acc->CreateAccessory();
    acc = li.GetNext();
  }
}

void Desktop::SetFocus()
{
  XSetInputFocus(display, topWin, RevertToParent, CurrentTime);
}
