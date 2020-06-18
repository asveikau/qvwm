/*
 * mini.h
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

#ifndef MINI_H_
#define MINI_H_

class Qvwm;
class Tooltip;

/*
 * Miniature class
 *   Miniture window in pager according to real window.
 */
class Miniature {
friend class Pager;
private:
  Window wMini;
  Rect rc;
  Qvwm* qvWm;
  Bool focus;
  
  Tooltip* toolTip;

public:
  static XContext context;

public:
  Miniature(Qvwm* qvWm, const Rect& rect);
  ~Miniature();

  void SetFocus() { focus = True; DrawMiniature(); }
  void ResetFocus() { focus = False; DrawMiniature(); }

  Window GetWin() const { return wMini; }

  void MapMiniature() { XMapWindow(display, wMini); }
  void UnmapMiniature() { XUnmapWindow(display, wMini); }
  void RaiseMiniature() { XRaiseWindow(display, wMini); }
  void MoveMiniature(const Point& pt);
  void MoveResizeMiniature(const Rect& rect);
  void DrawMiniature();
  void Button1Press(const Point& ptWin);
  void Button2Press(const Point& ptRoot);
  void Enter();
  void Leave();
  void PointerMotion();

  static void RestackMiniatures(Miniature* minis[], int nminis);
  static void Initialize();
};

#endif // MINI_H_
