/*
 * paging.h
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

#ifndef _PAGING_H_
#define _PAGING_H_

#include "misc.h"

/*
 * Paging class
 */
class Paging {
private:
  enum BeltPos {
    BOTTOM = 0, TOP, LEFT, RIGHT,
    BOTTOM_LEFT, BOTTOM_RIGHT, TOP_LEFT, TOP_RIGHT
  };

private:
  Window belt[8];
  Bool mapped[8];

public:
  Point origin;             // left-top point in logical coordinates
  Rect rcVirt;              // size and top-left page of virtual page

private:
  BeltPos GetBeltPos(const Point& pt);

public:
  Paging(const Point& topLeft, const Dim& size);
  ~Paging();

  Bool IsPagingBelt(Window win);
  Rect GetVirtRect() const { return rcVirt; }

  Point HandlePaging(const Point& pt);
  Point PagingProc(Window win, Bool rootFocus = True);
  void PagingProc(const Point& pt, Bool rootFocus = True);
  void PagingAllWindows(const Point& oldOrigin, Bool rootFocus = True);
  void MapBelts();
  void RaisePagingBelt();

  void SwitchPageLeft();
  void SwitchPageRight();
  void SwitchPageUp();
  void SwitchPageDown();
};

#endif // _PAGING_H_
