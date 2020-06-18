/*
 * qvwm.cc
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
#include <X11/Xresource.h>
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include <X11/Xatom.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "util.h"
#include "fbutton.h"
#include "tbutton.h"
#include "sbutton.h"
#include "menu.h"
#include "startmenu.h"
#include "event.h"
#include "taskbar.h"
#include "icon.h"
#include "qvwmrc.h"
#include "switcher.h"
#include "paging.h"
#include "pager.h"
#include "mini.h"
#include "key.h"
#include "indicator.h"
#include "focus_mgr.h"
#include "desktop.h"
#include "mwm.h"
#include "gnome.h"
#include "exec.h"
#include "image.h"
#include "session.h"
#include "tooltip.h"
#include "screen_saver.h"

Bool		Qvwm::umReserved = False;
XContext	Qvwm::context;

int Qvwm::TITLE_HEIGHT;
int Qvwm::BORDER_WIDTH;
int Qvwm::TOP_BORDER;

/*
 * Qvwm class constructor --
 *   Position the window at that point if usrPos is True;
 *   otherwise, the position of the window obeys a given rule.
 */
Qvwm::Qvwm(Window w, Bool usrPos)
: wOrig(w)
{
  flags = 0;
  status = 0;
  mapped = False;
  imgLarge = NULL;
  imgSmall = NULL;
  nCmapWins = 0;
  name = shortname = NULL;
  isFirstMapped = True;
#ifdef USE_XSMP
  smClientId = NULL;
  wmRole = NULL;
#endif

  XGrabServer(display);

  /*
   * Get some hints.
   */
  wmHints = XGetWMHints(display, w);
  GetWindowSizeHints();
  Gnome::GetHints(this);
  Mwm::GetHints(this);
  GetWindowClassHints();
  FetchWMProtocols();
  GetTransientForHint();

  Gnome::SetInitStates(this);

  GetWindowAttrs(usrPos);

  if (Intersect(rc, rootQvwm->GetRect()))
    Gnome::ResetState(this, WIN_STATE_HID_WORKSPACE);
  else
    Gnome::SetState(this, WIN_STATE_HID_WORKSPACE);

  Rect rcCtrl, rcButton[3];

  CalcFramePos(rc, &rcParent, &rcTitle, rcCorner, rcSide, rcEdge, &rcCtrl,
	       rcButton);

  CreateFrame(rc);
  CreateParent(rcParent);
  CreateSides(rcSide);
  CreateCorners(rcCorner);  
  CreateEdges(rcEdge);
  CreateTitle(rcTitle);
  CreateCtrlButton(rcCtrl);

  if (CheckFlags(THIN_BORDER))
    XSetWindowBorderWidth(display, frame, 1);

  /*
   * Create three kinds of frame button.
   */
  fButton[0] = new FrameButton1(this, title, rcButton[0]);
  fButton[0]->ChangeImage(FrameButton::MINIMIZE);
  fButton[1] = new FrameButton2(this, title, rcButton[1]);
  fButton[1]->ChangeImage(FrameButton::MAXIMIZE);
  fButton[2] = new FrameButton3(this, title, rcButton[2]);
  fButton[2]->ChangeImage(FrameButton::CLOSE);
  
  if (imgLarge == NULL)
    imgLarge = GetFixedIcon(32);
  if (imgSmall == NULL)
    imgSmall = GetFixedIcon(16);

  Rect rect(-50, -50, 50, 50);
  QvImage* img = imgSmall->Duplicate();
  tButton = new TaskbarButton(this, rect, img);

  ctrlMenu = ::ctrlMenu;

  ChangeOrigWindow();
  SendConfigureEvent();

#ifdef USE_SHAPE
  int bShaped, cShaped;
  int xbs, ybs, xcs, ycs;
  unsigned int wbs, hbs, wcs, hcs;

  if (shapeSupport) {
    XShapeSelectInput(display, w, ShapeNotifyMask);
    XShapeQueryExtents(display, w, &bShaped, &xbs, &ybs, &wbs, &hbs,
		       &cShaped, &xcs, &ycs, &wcs, &hcs);
    if (bShaped) {
      SetFlags(SHAPED);
      SetShape();
    }
  }
#endif // USE_SHAPE

  if (UsePager)
    mini = new Miniature(this, rc);

  scKey->GrabKeys(frame);

  SetTransient();

  XUngrabServer(display);
}

/*
 * Qvwm class constructor --
 *   For only root window.
 */
Qvwm::Qvwm(Window root)
: wOrig(root), frame(root)
{
  Rect rect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT);

  rc = Rect(0, 0,
	    DisplayWidth(display, screen), DisplayHeight(display, screen));

  parent = title = ctrl = None;
  for (int i = 0; i < 4; i++) {
    corner[i] = None;
    side[i]= None;
  }
  mapped = True;
  flags = 0L;
  status = 0;
  nCmapWins = 0;
  name = new char[10];
  strcpy(name, "qvwm root");
  shortname = NULL;
  toolTip = NULL;

#ifdef USE_XSMP
  smClientId = NULL;
  wmRole = NULL;
#endif

  XRectangle ink, log;

  XmbTextExtents(fsTaskbar, StartButtonTitle, strlen(StartButtonTitle),
                 &ink, &log);
  log.width += 2 + StartButton::SYMBOL_SIZE + 4 + 2 + 4;

  taskBar = new Taskbar(this, log.width + 4, TaskbarRows);
  tButton = new StartButton(this, log.width);

  rect = Rect(0, 0, 0, 0);
  ctrlMenu = new Menu(DesktopMenuItem, fsCtrlMenu, NULL, this);

  XSaveContext(display, root, context, (caddr_t)this);

  focusQvwm = this;

  scKey->GrabKeys(root);
}

Qvwm::~Qvwm()
{
  int i;

  if (this != rootQvwm) {
    XDeleteContext(display, frame, context);
    XDeleteContext(display, parent, context);
    XDeleteContext(display, wOrig, context);
    XDeleteContext(display, title, context);
    for (i = 0; i < 4; i++) {
      XDeleteContext(display, corner[i], context);
      XDeleteContext(display, side[i], context);
    }
    XDeleteContext(display, ctrl, context);

    QvImage::Destroy(imgLarge);
    QvImage::Destroy(imgSmall);

    if (TitlebarImage)
      QvImage::Destroy(imgTitle);
    if (TitlebarActiveImage)
      QvImage::Destroy(imgActiveTitle);

    if (FrameImage) {
      for (i = 0; i < 4; i++) {
	QvImage::Destroy(imgSide[i]);
	QvImage::Destroy(imgCorner[i]);
      }
    }
    if (FrameActiveImage) {
      for (i = 0; i < 4; i++) {
	QvImage::Destroy(imgActiveSide[i]);
	QvImage::Destroy(imgActiveCorner[i]);
      }
    }

    if (GradTitlebar) {
      if (pixGrad)
	XFreePixmap(display, pixGrad);
      if (pixActiveGrad)
	XFreePixmap(display, pixActiveGrad);
    }

    delete toolTip;

    if (wmHints)
      XFree(wmHints);

    if (classHints.res_name)
      XFree(classHints.res_name);
    if (classHints.res_class)
      XFree(classHints.res_class);

#ifdef USE_XSMP
    if (wmRole)
      XFree(wmRole);
    if (smClientId)
      XFree(smClientId);
#endif

    /*
     * Delete frame buttons before the parent window(frame) is destroyed.
     */
    for (i = 0; i < 3; i++)
      delete fButton[i];
    delete tButton;

    /*
     * When this is a transient window.
     */
    if (CheckFlags(TRANSIENT) && qvMain) {
      List<Qvwm>::Iterator li(&qvMain->trList);
      Qvwm* qvWm = li.GetHead();

      while (qvWm) {
	if (qvWm == this) {
	  qvMain->trList.Remove(this);
	  break;
	}
	qvWm = li.GetNext();
      }
    }
    else {
      /*
       * When this is a main window which has some transient windows.
       */
      List<Qvwm>::Iterator li(&trList);
      Qvwm* qvTrans = li.GetHead();

      while (qvTrans) {
	qvTrans->qvMain = NULL;
	qvTrans = li.GetNext();
      }
    }

    if (CheckFlags(ONTOP))
      desktop.GetOnTopList().Remove(this);

    if (UsePager)
      delete mini;

    XDestroyWindow(display, frame);
  }
  else
    delete ctrlMenu;

  delete [] name;
  delete [] shortname;
}

void Qvwm::GetWindowAttrs(Bool usrPos)
{
  XWindowAttributes attr;

  XGetWindowAttributes(display, wOrig, &attr);

  /*
   * Preserve original border width and set border width to 0.
   */
  bwOrig = attr.border_width;
  XSetWindowBorderWidth(display, wOrig, 0);

#ifdef USE_XSMP
  /*
   * Restore properties and attributes from the previous session
   */
  GetSMClientId(); // initialize smClientId
  GetWMRole(); // initialize wmRole
  int ax, ay, aw, ah;
  if (session->SearchProto(smClientId, wmRole,
			   classHints.res_class,
			   classHints.res_name,
			   name, &ax, &ay, &aw, &ah)) {
    usrPos = True; // force place
    attr.x = ax;
    attr.y = ay;
    attr.width = aw;
    attr.height = ah;
  }
#endif // USE_XSMP

  Point pageoff;
  ExecPending* pending = ExecPending::LookInList(name, classHints);

  if (pending) {
    pageoff = pending->GetPageOff();
    ExecPending::m_pendingList.Remove(pending);
    delete pending;
  }
  else
    pageoff = paging->origin;

  /*
   * Calculate the window position.
   */
  if (usrPos || (flags & (TRANSIENT | FIXED_POS)) ||
      (hints.flags & (USPosition | PPosition))) {
    if (CheckFlags(GEOMETRY)) {
      if (geom->bitmask & WidthValue)
	attr.width = geom->rc.width;
      if (geom->bitmask & HeightValue)
	attr.height = geom->rc.height;
    }

    if (CheckFlags(STICKY))
      rcOrig = Rect(attr.x, attr.y, attr.width, attr.height);
    else
      rcOrig = Rect(attr.x + pageoff.x, attr.y + pageoff.y,
		    attr.width, attr.height);

    CalcWindowPos(rcOrig);
  }
  else {
    if (CheckFlags(GEOMETRY)) {
      if (geom->bitmask & WidthValue)
	attr.width = geom->rc.width;
      if (geom->bitmask & HeightValue)
	attr.height = geom->rc.height;

      if (geom->bitmask & XNegative)
	attr.x = DisplayWidth(display, screen) + geom->rc.x - attr.width;
      else if (geom->bitmask & XValue)
	attr.x = geom->rc.x;

      if (geom->bitmask & YNegative)
	attr.y = DisplayHeight(display, screen) + geom->rc.y - attr.height;
      else if (geom->bitmask & YValue)
	attr.y = geom->rc.y;

      if (CheckFlags(STICKY))
	rcOrig = Rect(attr.x, attr.y, attr.width, attr.height);
      else
	rcOrig = Rect(attr.x + pageoff.x, attr.y + pageoff.y,
		      attr.width, attr.height);
      
      CalcWindowPos(rcOrig);
    }
    else {
      int borderWidth, topBorder, titleHeight, titleEdge;

      GetBorderAndTitle(borderWidth, topBorder, titleHeight, titleEdge);

      rc.width = attr.width + borderWidth * 2;
      rc.height = attr.height + borderWidth + topBorder + titleHeight
	+ titleEdge;

      Point pt = desktop.GetNextPlace(Dim(rc.width, rc.height), pageoff);

      rc.x = pt.x;
      rc.y = pt.y;

      if (CheckFlags(STICKY))
	rcOrig = Rect(rc.x - rcScreen.x - pageoff.x,
		      rc.y - rcScreen.y - pageoff.y,
		      attr.width,
		      attr.height);
      else
	rcOrig = Rect(rc.x - rcScreen.x, rc.y - rcScreen.y,
		      attr.width, attr.height);
    }
  }
}

/*
 * ChangeOrigWindow --
 *   Change parent and attributes of original window.
 */
void Qvwm::ChangeOrigWindow()
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  XReparentWindow(display, wOrig, parent, 0, 0);

  attributes.event_mask = StructureNotifyMask | PropertyChangeMask |
    EnterWindowMask | LeaveWindowMask | ColormapChangeMask;

#if defined(USE_SS) && !defined(USE_SSEXT)
  XWindowAttributes attr;

  XGetWindowAttributes(display, wOrig, &attr);
  attributes.event_mask |= attr.your_event_mask;
#endif

  attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
  valueMask = CWEventMask | CWDontPropagate;

  XChangeWindowAttributes(display, wOrig, valueMask, &attributes);

  XSaveContext(display, wOrig, context, (caddr_t)this);
  XAddToSaveSet(display, wOrig);

#if defined(USE_SS) && !defined(USE_SSEXT)
  // check KeyPressMask of subwindows of wOrig again
  scrSaver->SelectEvents(wOrig);
#endif
}

/*
 * SetTransient
 *   Search pre-mapped transient windows for this window to handle
 *   a transient window properly even if the main window is mapped
 *   after the transient window is mapped first.
 */
void Qvwm::SetTransient()
{
  List<Qvwm>::Iterator i(&desktop.GetQvwmList());
  Window tWin;

  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
    if (qvWm->CheckFlags(TRANSIENT) && qvWm->qvMain == NULL)
      if (XGetTransientForHint(display, qvWm->GetWin(), &tWin))
	if (tWin == wOrig) {
	  qvWm->qvMain = this;  // qvWm is a transient window of this
	  if (CheckFlags(ONTOP) && !qvWm->CheckFlags(ONTOP)) {
	    qvWm->SetFlags(ONTOP);
	    desktop.GetOnTopList().InsertTail(qvWm);
	    Gnome::ChangeLayer(qvWm, WIN_LAYER_ONTOP);
	  }
	  trList.InsertTail(qvWm);
	}
  }
}

/*
 * SendMessage --
 *   Send the messeage to this window.
 */
void Qvwm::SendMessage(Atom atom)
{
  XClientMessageEvent cev;

  cev.type = ClientMessage;
  cev.window = wOrig;
  cev.message_type = _XA_WM_PROTOCOLS;
  cev.format = 32;
  cev.data.l[0] = atom;
  cev.data.l[1] = CurrentTime;

#ifdef DEBUG
  if (atom == _XA_WM_DELETE_WINDOW)
    printf("Send delete msg to '%s'\n", name);
#endif

  XSendEvent(display, wOrig, False, 0L, (XEvent *)&cev);
}

/*
 * AdjustPage --
 *   Switch to the page where the window center point exists.
 */
void Qvwm::AdjustPage()
{
  if (this == rootQvwm)
    return;

  if (CheckFlags(STICKY))
    return;

  Rect rect = GetRect();
  Point ptCenter(rect.x+rect.width/2, rect.y+rect.height/2);
  Rect rcRoot = rootQvwm->GetRect();
  
  if (ptCenter.x >= 0)
    ptCenter.x =  ptCenter.x / rcRoot.width * rcRoot.width;
  else
    ptCenter.x = (ptCenter.x / rcRoot.width - 1) * rcRoot.width;

  if (ptCenter.y >= 0)
    ptCenter.y = ptCenter.y / rcRoot.height * rcRoot.height;
  else 
    ptCenter.y = (ptCenter.y / rcRoot.height - 1) * rcRoot.height;
  
  ASSERT(paging);

  paging->PagingProc(ptCenter, False);
}

/*
 * Exposure --
 *   Process exposure event.
 */
void Qvwm::Exposure(Window win)
{
  if (win == ctrl)
    DrawCtrlMenuMark();
  else if (win == title)
    DrawTitle(False);
  else
    DrawFrame(win);  // win may not be a part of frame windows
}

/*
 * Button1Press --
 *   Process the press of button1(mouse left button).
 */
void Qvwm::Button1Press(Window win, Time clickTime, const Point& ptRoot,
			unsigned int state)
{
  if (this != rootQvwm)
    toolTip->Disable();

  if (UseTaskbar && TaskbarAutoHide && !taskBar->IsHiding())
    taskBar->HideTaskbar();

  if (win == ctrl)
    CtrlButton1Press(clickTime, ptRoot, state);
  else {
    Menu::UnmapAllMenus();
    
    if (!CheckFocus())
      SetFocus();

    if (Icon::focusIcon != NULL)
      Icon::focusIcon->ResetFocus();

    if (ClickingRaises || win != wOrig)
      RaiseWindow(True);

    /*
     * Title bar.
     */
    if (win == title)
      TitleButton1Press(clickTime, ptRoot);

    SetStatus(PRESS_FRAME);
  }

  if (AutoRaise && focusQvwm == this && activeQvwm != this)
    RaiseWindow(True);
}

/*
 * Button1Release --
 *   Process the release of button1(mouse left button).
 */
void Qvwm::Button1Release()
{
  ResetStatus(PRESS_FRAME);

  /*
   * Unmap control menu if the menu is shown and menu button is pressed.
   */
  if (umReserved) {
    ASSERT(ctrlMenu);
    ctrlMenu->UnmapMenu();
    umReserved = False;
  }
}    

void Qvwm::Button2Press(Window win)
{
  if (this != rootQvwm)
    toolTip->Disable();

  if (win == side[0] || win == side[1] || win == side[2] || win == side[3] ||
      win == corner[0] || win == corner[1] || win == corner[2] ||
      win == corner[3])
    SetStatus(PRESS_FRAME);

  if (AutoRaise && focusQvwm == this && activeQvwm != this)
    RaiseWindow(True);
}

void Qvwm::Button2Release(Window win)
{
  ResetStatus(PRESS_FRAME);
  if (win != frame)
    LowerWindow();
}

void Qvwm::Button3Press()
{
  if (this != rootQvwm)
    toolTip->Disable();

  if (AutoRaise && focusQvwm == this && activeQvwm != this)
    RaiseWindow(True);
}

/*
 * Button3Release --
 *   Process the release of button3(mouse right button).
 */
void Qvwm::Button3Release(Window win, const Point& ptRoot)
{
  Point pt;
  int dir;

  if (this == rootQvwm) {
    Menu::UnmapAllMenus();
    
    if (!CheckFocus())
      SetFocus();
    
    ASSERT(ctrlMenu);
    pt = ctrlMenu->GetFixedMenuPos(ptRoot, dir);
    ctrlMenu->MapMenu(pt.x, pt.y, dir);
  }
  else {
    Menu::UnmapAllMenus();
    
    if (!CheckFocus())
      SetFocus();

    if (CheckMapped())
      RaiseWindow(True);
    else
      RestoreWindow();

    ASSERT(ctrlMenu);
    ctrlMenu->SetQvwm(this);
    pt = ctrlMenu->GetFixedMenuPos(ptRoot, dir);
    ctrlMenu->MapMenu(pt.x, pt.y, dir);
  }
}

/*
 * Button1Motion --
 *   Move or resize according to mouse movement.
 */
void Qvwm::Button1Motion(Window win, const Point& ptRoot)
{
  if (win == title && !CheckStatus(MAXIMIZE_WINDOW))
    MoveWindow(ptRoot, True);
  else if (win == side[0])
    ResizeWindow(P_LEFT, True);
  else if (win == side[1])
    ResizeWindow(P_RIGHT, True);
  else if (win == side[2])
    ResizeWindow(P_TOP, True);
  else if (win == side[3])
    ResizeWindow(P_BOTTOM, True);
  else if (win == corner[0])
    ResizeWindow(P_LEFT | P_TOP, True);
  else if (win == corner[1])
    ResizeWindow(P_RIGHT | P_TOP, True);
  else if (win == corner[2])
    ResizeWindow(P_LEFT | P_BOTTOM, True);
  else if (win == corner[3])
    ResizeWindow(P_RIGHT | P_BOTTOM, True);
}

void Qvwm::Button2Motion(Window win, const Point& ptRoot)
{
  if (!CheckStatus(MAXIMIZE_WINDOW))
    MoveWindow(ptRoot, True);
}

void Qvwm::Enter(Window win, unsigned int state)
{
  if (win == title) {
    if (strcmp(name, shortname) != 0)
      toolTip->SetTimer();
  }

  if ((state & NoFocusChangeMask) == NoFocusChangeMask)
    return;

  if (!ClickToFocus && !(NoDesktopFocus && this == rootQvwm) &&
      !CheckStatus(MINIMIZE_WINDOW) && !CheckFlags(FOCUS_ON_CLICK)) {
    if (!CheckFocus() && !Menu::CheckAnyMenusMapped()) {
      if (AutoRaise)
	RaiseWindow(False);
      SetFocus();
    }
  }
}

void Qvwm::Leave(Window win)
{
  if (win == title)
    toolTip->Disable();
}

void Qvwm::PointerMotion(Window win)
{
  if (win == title) {
    if (!toolTip->IsMapped())
      toolTip->ResetTimer();
  }
}

void Qvwm::Initialize()
{
  context = XUniqueContext();

  TITLE_HEIGHT = FrameTitleHeight;
  BORDER_WIDTH = FrameBorderWidth + 4;
  TOP_BORDER = FrameBorderWidth + 2;

  CreateTitlebarPixmap();
  CreateFramePixmap();
}

#ifdef USE_XSMP
/*
 * SavePropertiesToFile --
 *   Save the state of this window to the FILE for XSMP
 */
Bool Qvwm::SavePropertiesToFile(int idx, FILE* fp)
{
  if (qvMain) {
    // this window is a transient win.
    return True;
  }
  GetSMClientId();
  GetWMRole();
  // WM_CLASS  (if WM_WINDOW_ROLE is not specified)
  XClassHint wm_class;        // STRING
  if (wmRole == NULL) {
    if (XGetClassHint(display, wOrig, &wm_class) == 0)
      return False;
  }

  // Write out to the 'fp'
  if (smClientId != NULL)
    fprintf(fp, "%d,id=%s\n", idx, smClientId);       // SM_CLIENT_ID

  fprintf(fp, "%d,wOrig=%x\n", idx, (int)wOrig);
  fprintf(fp, "%d,frame=%x\n", idx, (int)frame);
  if (wmRole)
    fprintf(fp, "%d,role=%s\n", idx, wmRole); // WM_WINDOW_ROLE
  else {
    fprintf(fp, "%d,class=%s\n", idx, wm_class.res_class);// WM_CLASS
    fprintf(fp, "%d,rname=%s\n", idx, wm_class.res_name);
    fprintf(fp, "%d,name=%s\n", idx, name);   // WM_NAME
  }
  fprintf(fp, "%d,pos=%d,%d\n", idx, rcOrig.x, rcOrig.y);
  fprintf(fp, "%d,size=%d,%d\n", idx, rcOrig.width, rcOrig.height);

  if (wmRole == NULL) {
    XFree(wm_class.res_class);
    XFree(wm_class.res_name);
  }
  return True;
}
#endif // USE_XSMP
