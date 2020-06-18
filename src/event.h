/*
 * event.h
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

#ifndef _EVENTS_H_
#define _EVENTS_H_

#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif

class Point;

class Event {
public:
  Point ptPrevRoot;
  int shapeEventBase;

//  XContext m_context;

private:
  void DestroyQvwm(Qvwm* qvWm);

public:
  void MainLoop();
  void DispatchEvent(const XEvent& ev);

  void ExposeProc(const XExposeEvent& ev);

  void EnterNotifyProc(const XCrossingEvent& ev);
  void LeaveNotifyProc(const XCrossingEvent& ev);
  void MotionNotifyProc(const XMotionEvent& ev);

  void ButtonPressProc(const XButtonEvent& ev);
  void ButtonReleaseProc(const XButtonEvent& ev);

  void KeyPressProc(const XKeyEvent& ev);

  void MapRequestProc(const XMapRequestEvent& ev);
  void MapNotifyProc(const XMapEvent& ev);
  void UnmapNotifyProc(const XUnmapEvent& ev);
  void DestroyNotifyProc(const XDestroyWindowEvent& ev);

  void ClientMessageProc(const XClientMessageEvent& ev);
  void ColormapNotifyProc(const XColormapEvent& ev);
  void ConfigureRequestProc(const XConfigureRequestEvent& ev);
  void CirculateRequestProc(const XCirculateRequestEvent& ev);
  void PropertyNotifyProc(const XPropertyEvent& ev);

  void FocusInProc(const XFocusChangeEvent& ev);

#ifdef USE_SHAPE
  void ShapeNotifyProc(const XShapeEvent& ev);
#endif
};

#endif // _EVENTS_H_
