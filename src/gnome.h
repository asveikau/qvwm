/*
 * gnome.h
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

#ifndef GNOME_H_
#define GNOME_H_

// bitmasks for the _WIN_STATE property
#define WIN_STATE_STICKY          (1<<0)  // everyone knows sticky
#define WIN_STATE_MINIMIZED       (1<<1)  // Reserved - definition is unclear
#define WIN_STATE_MAXIMIZED_VERT  (1<<2)  // window in maximized V state
#define WIN_STATE_MAXIMIZED_HORIZ (1<<3)  // window in maximized H state
#define WIN_STATE_HIDDEN          (1<<4)  // not on taskbar but window visible
#define WIN_STATE_SHADED          (1<<5)  // shaded (MacOS / Afterstep style)
#define WIN_STATE_HID_WORKSPACE   (1<<6)  // not on current desktop
#define WIN_STATE_HID_TRANSIENT   (1<<7)  // owner of transient is hidden
#define WIN_STATE_FIXED_POSITION  (1<<8)  // window is fixed in position even
#define WIN_STATE_ARRANGE_IGNORE  (1<<9)  // ignore for auto arranging

// bitmasks for the _WIN_HINTS property
#define WIN_HINTS_SKIP_FOCUS      (1<<0)  // "alt-tab" skips this win
#define WIN_HINTS_SKIP_WINLIST    (1<<1)  // do not show in window list
#define WIN_HINTS_SKIP_TASKBAR    (1<<2)  // do not show on taskbar
#define WIN_HINTS_GROUP_TRANSIENT (1<<3)  // Reserved - definition is unclear
#define WIN_HINTS_FOCUS_ON_CLICK  (1<<4)  // app only accepts focus if clicked

// values for the _WIN_LAYER property
#define WIN_LAYER_DESKTOP         0
#define WIN_LAYER_BELOW           2
#define WIN_LAYER_NORMAL          4
#define WIN_LAYER_ONTOP           6
#define WIN_LAYER_DOCK            8
#define WIN_LAYER_ABOVE_DOCK      10
#define WIN_LAYER_MENU            12

class Gnome {
private:
  static Atom XA_WIN_SUPPORTING_WM_CHECK;
  static Atom XA_WIN_PROTOCOLS;
  static Atom XA_WIN_CLIENT_LIST;
  static Atom XA_WIN_WORKSPACE_COUNT;
  static Atom XA_WIN_WORKSPACE_NAMES;
  static Atom XA_WIN_WORKSPACE;
  static Atom XA_WIN_AREA_COUNT;
  static Atom XA_WIN_AREA;
  static Atom XA_WIN_STATE;
  static Atom XA_WIN_HINTS;
  static Atom XA_WIN_LAYER;
  static Atom XA_WIN_EXPANDED_SIZE;
  static Atom XA_WIN_DESKTOP_BUTTON_PROXY;

private:
  static void SetProtocols();
  static void SetWorkspace();
  static void InitClientProperties();

  static void ProcessStateChange(Qvwm* qvWm, long mask, long state);
  static void ProcessHintsChange(Qvwm* qvWm, long mask, long hints);
  static void ProcessLayerChange(Qvwm* qvWm, long layer);

  static Bool GetAtomValue(Qvwm* qvWm, Atom atom, long* val);

public:
  static void Init();
  static void SetGnomeCompliant(Window win);
  static void ChangeClientList();
  static void SetActiveDesktop();
  static void SetProperty(Qvwm* qvWm, Atom atom, long* data);

  static void SetState(Qvwm* qvWm, long state);
  static void ResetState(Qvwm* qvWm, long state);
  static void SetInitStates(Qvwm* qvWm);

  static void ChangeLayer(Qvwm* qvWm, long layer);
  static void GetHints(Qvwm* qvWm);
  static void ProxyDesktopEvent(XEvent* ev);
};

#endif // GNOME_H_
