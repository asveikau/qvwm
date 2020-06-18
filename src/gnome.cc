/*
 * gnome.cc
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
#include <X11/Xatom.h>
#include "main.h"
#include "gnome.h"
#include "paging.h"
#include "desktop.h"
#include "qvwm.h"
#include "fbutton.h"
#include "list.h"
#include "qvwmrc.h"

Atom Gnome::XA_WIN_SUPPORTING_WM_CHECK;
Atom Gnome::XA_WIN_PROTOCOLS;
Atom Gnome::XA_WIN_CLIENT_LIST;
Atom Gnome::XA_WIN_WORKSPACE_COUNT;
Atom Gnome::XA_WIN_WORKSPACE_NAMES;
Atom Gnome::XA_WIN_WORKSPACE;
Atom Gnome::XA_WIN_AREA_COUNT;
Atom Gnome::XA_WIN_AREA;
Atom Gnome::XA_WIN_STATE;
Atom Gnome::XA_WIN_HINTS;
Atom Gnome::XA_WIN_LAYER;
Atom Gnome::XA_WIN_EXPANDED_SIZE;
Atom Gnome::XA_WIN_DESKTOP_BUTTON_PROXY;

// called once at initialization
void Gnome::Init()
{
  // for detection of a GNOME compliant window manager
  XA_WIN_SUPPORTING_WM_CHECK = XInternAtom(display, "_WIN_SUPPORTING_WM_CHECK",
					   False);

  // providing shortcuts managed clients
  XA_WIN_CLIENT_LIST = XInternAtom(display, "_WIN_CLIENT_LIST", False);

  // providing multiple/virtual desktop information
  SetWorkspace();

  // set active desktop
  SetActiveDesktop();

  InitClientProperties();

  // listing GNOME window manager compliance
  SetProtocols();

  XA_WIN_DESKTOP_BUTTON_PROXY = XInternAtom(display,
					    "_WIN_DESKTOP_BUTTON_PROXY",
					    False);
}

void Gnome::SetProtocols()
{
  Atom list[10];  

  XA_WIN_PROTOCOLS = XInternAtom(display, "_WIN_PROTOCOLS", False);

  list[0] = XA_WIN_LAYER;
  list[1] = XA_WIN_STATE;
  list[2] = XA_WIN_HINTS;
  list[3] = XInternAtom(display, "_WIN_APP_STATE", False);
  list[4] = XA_WIN_EXPANDED_SIZE;
  list[5] = XInternAtom(display, "_WIN_ICONS", False);
  list[6] = XA_WIN_WORKSPACE;
  list[7] = XA_WIN_WORKSPACE_COUNT;
  list[8] = XA_WIN_WORKSPACE_NAMES;
  list[9] = XA_WIN_CLIENT_LIST;

  XChangeProperty(display, root, XA_WIN_PROTOCOLS, XA_ATOM, 32,
		  PropModeReplace, (unsigned char *)list, 10);
}

//
// Qvwm uses WIN_AREA although it does not support multiple workspaces
// and Gnome recommends to use only WIN_WORKSPACE in this case.
// This is because WIN_WORKSPACE has no information on width and height
// of virtual desktop and gnome-pager's shape does not match qvwm pager's.
//
void Gnome::SetWorkspace()
{
  Rect rcVirt = paging->GetVirtRect();
  int num, val[2];

  // set the number of desktops for workspace
  XA_WIN_WORKSPACE_COUNT = XInternAtom(display, "_WIN_WORKSPACE_COUNT", False);
  num = 1;  // fixed
  XChangeProperty(display, root, XA_WIN_WORKSPACE_COUNT, XA_CARDINAL, 32, 
		  PropModeReplace, (unsigned char *)&num, 1);

  // set the number of desktops for area
  XA_WIN_AREA_COUNT = XInternAtom(display, "_WIN_AREA_COUNT", False);
  val[0] = rcVirt.width;
  val[1] = rcVirt.height;
  XChangeProperty(display, root, XA_WIN_AREA_COUNT, XA_CARDINAL, 32, 
		  PropModeReplace, (unsigned char *)val, 2);

  // set desktop name
  char* name = "Qvwm Desktop";
  XTextProperty text;

  if (XStringListToTextProperty(&name, 1, &text)) {
    XA_WIN_WORKSPACE_NAMES = XInternAtom(display, "_WIN_WORKSPACE_NAMES",
					 False);
    XSetTextProperty(display, root, &text, XA_WIN_WORKSPACE_NAMES);
    XFree(text.value);
  }

  // for active desktop
  XA_WIN_WORKSPACE = XInternAtom(display, "_WIN_WORKSPACE", False);
  XA_WIN_AREA = XInternAtom(display, "_WIN_AREA", False);

  int index = 0;  // fixed
  XChangeProperty(display, root, XA_WIN_WORKSPACE, XA_CARDINAL, 32, 
		  PropModeReplace, (unsigned char *)&index, 1);
}

void Gnome::InitClientProperties()
{
  XA_WIN_STATE = XInternAtom(display, "_WIN_STATE", False);
  XA_WIN_HINTS = XInternAtom(display, "_WIN_HINTS", False);
  XA_WIN_LAYER = XInternAtom(display, "_WIN_LAYER", False);
  XA_WIN_EXPANDED_SIZE = XInternAtom(display, "_WIN_EXPANDED_SIZE", False);
}

// called when a new window is created
void Gnome::SetGnomeCompliant(Window win)
{
  XChangeProperty(display, root, XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32,
		  PropModeReplace, (unsigned char *)&win, 1);
  XChangeProperty(display, win, XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32,
		  PropModeReplace, (unsigned char *)&win, 1);
}

// called when a window is created and destroyed
void Gnome::ChangeClientList()
{
  List<Qvwm>* qvwmList = &desktop.GetQvwmList();
  List<Qvwm>::Iterator i(qvwmList);
  Window* cliList;
  Qvwm* qvWm;
  int num = 0;
  
  cliList = new Window[qvwmList->GetSize()];

  for (qvWm = i.GetHead(); qvWm; qvWm = i.GetNext())
    cliList[num++] = qvWm->GetWin();

  XChangeProperty(display, root, XA_WIN_CLIENT_LIST, XA_CARDINAL, 32, 
		  PropModeReplace, (unsigned char *)cliList, num);

  delete [] cliList;
}

// called when the desktop is changed
// If the desktop is not located according to the grid, the position is
// rounded off to the grid.
void Gnome::SetActiveDesktop()
{
  Rect rcRoot = rootQvwm->GetRect();
  Rect rcVirt = paging->GetVirtRect();
  int val[2];

  if (paging->origin.x >= 0)
    val[0] = (paging->origin.x + rcRoot.width / 2) / rcRoot.width - rcVirt.x;
  else
    val[0] = (paging->origin.x - rcRoot.width / 2) / rcRoot.width - rcVirt.x;

  if (paging->origin.y >= 0)
    val[1] = (paging->origin.y + rcRoot.height / 2) / rcRoot.height - rcVirt.y;
  else
    val[1] = (paging->origin.y - rcRoot.height / 2) / rcRoot.height - rcVirt.y;

  XChangeProperty(display, root, XA_WIN_AREA, XA_CARDINAL, 32, 
		  PropModeReplace, (unsigned char *)val, 2);
}

// change the window property
void Gnome::SetProperty(Qvwm* qvWm, Atom atom, long* data)
{
  if (atom == XA_WIN_STATE)
    ProcessStateChange(qvWm, data[0], data[1]);
  else if (atom == XA_WIN_HINTS)
    ProcessHintsChange(qvWm, data[0], data[1]);
  else if (atom == XA_WIN_LAYER)
    ProcessLayerChange(qvWm, data[0]);
  else if (atom == XA_WIN_AREA) {
    Rect rcVirt = paging->GetVirtRect();
    Rect rcRoot = rootQvwm->GetRect();
    Point pt;

    pt.x = (data[0] + rcVirt.x) * rcRoot.width;
    pt.y = (data[1] + rcVirt.y) * rcRoot.height;

    paging->PagingProc(pt);
  }
  else if (atom == XA_WIN_EXPANDED_SIZE) {
#ifdef DEBUG
    QvwmError("_WIN_EXPANDED_SIZE is currently not supported");
#endif
  }
}

// mask stands for changed bits; state stands for a new state
void Gnome::ProcessStateChange(Qvwm* qvWm, long mask, long state)
{
  if (mask & WIN_STATE_STICKY) {
    if (state & WIN_STATE_STICKY) {
      if (!qvWm->CheckFlags(STICKY)) {
	qvWm->SetFlags(STICKY);
	qvWm->ConfigureNewRect(qvWm->GetRect());
      }
    }
    else {
      if (qvWm->CheckFlags(STICKY)) {
	qvWm->ResetFlags(STICKY);
	qvWm->ConfigureNewRect(qvWm->GetRect());
      }
    }
  }

  if (mask & WIN_STATE_MINIMIZED) {
    if (state & WIN_STATE_MINIMIZED) {
      if (!qvWm->CheckStatus(MINIMIZE_WINDOW))
	qvWm->MinimizeWindow(True, False);
    }
    else {
      if (qvWm->CheckStatus(MINIMIZE_WINDOW))
	qvWm->RestoreWindow(True, False);
    }
  }

  if (mask & (WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ)) {
    if (state & (WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ)) {
      if (!qvWm->CheckStatus(MAXIMIZE_WINDOW)) {
	qvWm->fButton[1]->ChangeImage(FrameButton::RESTORE);
	qvWm->MaximizeWindow(True, False);
	rootQvwm->SetFocus();
	qvWm->SetFocus();
      }
    }
    else {
      if (qvWm->CheckStatus(MAXIMIZE_WINDOW)) {
	qvWm->fButton[1]->ChangeImage(FrameButton::MAXIMIZE);
	qvWm->RestoreWindow(True, False);
	rootQvwm->SetFocus();
	qvWm->SetFocus();
      }	
    }
  }

  if (mask & WIN_STATE_HIDDEN) {
    if (state & WIN_STATE_HIDDEN) {
      if (!qvWm->CheckFlags(NO_TBUTTON)) {
	qvWm->SetFlags(NO_TBUTTON);
	qvWm->tButton->UnmapButton();
	if (UseTaskbar)
	  taskBar->RedrawAllTaskbarButtons();
      }
    }
  }
}

// mask stands for changed bits; hints stands for a new hints
void Gnome::ProcessHintsChange(Qvwm* qvWm, long mask, long hints)
{
  if (mask & WIN_HINTS_SKIP_FOCUS) {
    if (hints & WIN_HINTS_SKIP_FOCUS)
      qvWm->SetFlags(SKIP_FOCUS);  // XXX currently not effect
    else
      qvWm->ResetFlags(SKIP_FOCUS);
  }

  if (mask & WIN_HINTS_SKIP_TASKBAR) {  // same as WIN_STATE_HIDDEN?
    if (hints & WIN_HINTS_SKIP_TASKBAR) {
      if (!qvWm->CheckFlags(NO_TBUTTON)) {
	qvWm->SetFlags(NO_TBUTTON);
	qvWm->tButton->UnmapButton();
	if (UseTaskbar)
	  taskBar->RedrawAllTaskbarButtons();
      }
    }
  }

  if (mask & WIN_HINTS_FOCUS_ON_CLICK) {
    if (hints & WIN_HINTS_FOCUS_ON_CLICK)
      qvWm->SetFlags(FOCUS_ON_CLICK);
  }
  
  // XXX currently ignored
  if ((hints & WIN_HINTS_SKIP_WINLIST) ||
      (hints & WIN_HINTS_GROUP_TRANSIENT)) {
#ifdef DEBUG
    QvwmError("_WIN_HINTS %d is currently not supported", hints);
#endif
  }
}

void Gnome::ProcessLayerChange(Qvwm* qvWm, long layer)
{
  switch (layer) {
  case WIN_LAYER_DESKTOP:
  case WIN_LAYER_BELOW:
    if (qvWm->CheckFlags(ONTOP)) {
      qvWm->ResetFlags(ONTOP);
      desktop.GetOnTopList().Remove(qvWm);
    }
    qvWm->LowerWindow();
    break;
    
  case WIN_LAYER_NORMAL:
    if (qvWm->CheckFlags(ONTOP)) {
      qvWm->ResetFlags(ONTOP);
      desktop.GetOnTopList().Remove(qvWm);
    }
    break;
    
  case WIN_LAYER_ONTOP:
    if (!qvWm->CheckFlags(ONTOP)) {
      qvWm->SetFlags(ONTOP);
      desktop.GetOnTopList().InsertTail(qvWm);
      qvWm->RaiseWindow(True);
    }
    break;
    
  default:
#ifdef DEBUG
    QvwmError("_WIN_LAYER %d is currently not supported", layer);
#endif
    break;
  }
}

Bool Gnome::GetAtomValue(Qvwm* qvWm, Atom atom, long* val)
{
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;
  unsigned char *prop = NULL;

  if (XGetWindowProperty(display, qvWm->GetWin(), atom, 0, 1, False, atom,
			 &atype, &aformat, &nitems, &bytes_remain, &prop)
      != Success)
    return False;

  if (prop) {
    if (nitems == 1)
      *val = *(long *)&prop;

    XFree(prop);
  }

  return (nitems == 1);
}

void Gnome::SetState(Qvwm* qvWm, long state)
{
  long val;

  if (GetAtomValue(qvWm, XA_WIN_STATE, &val))
    val |= state;
  else
    val = state;

  XChangeProperty(display, qvWm->GetWin(), XA_WIN_STATE, XA_CARDINAL, 32,
		  PropModeReplace, (unsigned char *)&val, 1);
}

void Gnome::ResetState(Qvwm* qvWm, long state)
{
  long val;

  if (!GetAtomValue(qvWm, XA_WIN_STATE, &val))
    return;

  val &= ~state;

  XChangeProperty(display, qvWm->GetWin(), XA_WIN_STATE, XA_CARDINAL, 32,
		  PropModeReplace, (unsigned char *)&val, 1);
}

void Gnome::SetInitStates(Qvwm* qvWm)
{
  long state = 0;

  if (qvWm->CheckFlags(STICKY))
    state |= WIN_STATE_STICKY;
  if (qvWm->CheckFlags(INIT_MAXIMIZE))
    state |= WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ;
  if (qvWm->CheckFlags(INIT_MINIMIZE))
    state |= WIN_STATE_MINIMIZED;
  if (qvWm->CheckFlags(NO_TBUTTON))
    state |= WIN_STATE_HIDDEN;

  if (state)
    SetState(qvWm, state);
}    

void Gnome::ChangeLayer(Qvwm* qvWm, long layer)
{
  XChangeProperty(display, qvWm->GetWin(), XA_WIN_LAYER, XA_CARDINAL, 32,
		  PropModeReplace, (unsigned char *)&layer, 1);
}

// check state, hints, and layer at creating a window
void Gnome::GetHints(Qvwm* qvWm)
{
  // check state
  long state;

  if (GetAtomValue(qvWm, XA_WIN_STATE, &state)) {
    if (state & WIN_STATE_STICKY)
      qvWm->SetFlags(STICKY);
    if (state & WIN_STATE_MINIMIZED)
      qvWm->SetFlags(INIT_MINIMIZE);
    if (state & (WIN_STATE_MAXIMIZED_VERT | WIN_STATE_MAXIMIZED_HORIZ))
      qvWm->SetFlags(INIT_MAXIMIZE);
    if (state & WIN_STATE_HIDDEN)
      qvWm->SetFlags(NO_TBUTTON);
    if (state & WIN_STATE_FIXED_POSITION)
      qvWm->SetFlags(FIXED_POS);
  }

  // check hints
  long hints;
  
  if (GetAtomValue(qvWm, XA_WIN_HINTS, &hints)) {
    if (hints & WIN_HINTS_SKIP_FOCUS)
      qvWm->SetFlags(SKIP_FOCUS);  // XXX currently not effect
    if (hints & WIN_HINTS_SKIP_TASKBAR)
      qvWm->SetFlags(NO_TBUTTON);
    if (hints & WIN_HINTS_FOCUS_ON_CLICK)
      qvWm->SetFlags(FOCUS_ON_CLICK);
  }

  // check layer
  long layer;

  if (GetAtomValue(qvWm, XA_WIN_LAYER, &layer)) {
    if (layer == WIN_LAYER_ONTOP)
      qvWm->SetFlags(ONTOP);
  }
}

// XXX currently not proxy any desktop button events
void Gnome::ProxyDesktopEvent(XEvent* ev)
{
  Window win;

  if (ev->type == ButtonPress)
    XUngrabPointer(display, CurrentTime);

  if (!GetAtomValue(rootQvwm, XA_WIN_DESKTOP_BUTTON_PROXY, (long *)&win))
    return;

  if (win != None)
    XSendEvent(display, win, False, SubstructureNotifyMask, ev);
}
