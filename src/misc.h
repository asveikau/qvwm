/*
 * misc.h
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

#ifndef _MISC_H_
#define _MISC_H_

#ifdef __EMX__
extern "C" char *__XOS2RedirRoot(char *filename);
#endif

const int MAX_NAME = 200;
const int MAX_WINDOW_WIDTH = 32767;
const int MAX_WINDOW_HEIGHT = 32767;
const int MAX_FAILURE_COUNT = 16;

/*
 * Cursor name
 */
enum CursorName {
  SYS,
  X_RESIZE,
  Y_RESIZE,
  RD_RESIZE,
  LD_RESIZE,
  WAIT,
  MOVE,
  BGWORK,
  DISALLOW
};

// Position
#define P_TOP		(1 << 0)
#define P_BOTTOM	(1 << 1)
#define P_LEFT		(1 << 2)
#define P_RIGHT		(1 << 3)

/*
 * Point class
 */
class Point {
public:
  int x, y;

public:
  Point() {}
  Point(int px, int py) : x(px), y(py) {}
  Point(const Point& pt) : x(pt.x), y(pt.y) {}
  Point& operator=(const Point& pt);
  Point& operator+=(const Point& pt);
  Point& operator-=(const Point& pt);
  Point operator+(const Point& pt);
  Point operator-(const Point& pt);
};

inline Point& Point::operator=(const Point& pt)
{
  x = pt.x;
  y = pt.y;

  return *this;
}

inline Point& Point::operator+=(const Point& pt)
{
  x += pt.x;
  y += pt.y;

  return *this;
}

inline Point& Point::operator-=(const Point& pt)
{
  x -= pt.x;
  y -= pt.y;

  return *this;
}

inline Point Point::operator+(const Point& pt)
{
  return Point(x + pt.x, y + pt.y);
}

inline Point Point::operator-(const Point& pt)
{
  return Point(x - pt.x, y - pt.y);
}

/*
 * Rect class
 */
class Rect {
public:
  int x, y;
  int width, height;

public:
  Rect() {}
  Rect(int rx, int ry, int rwidth, int rheight)
    : x(rx), y(ry), width(rwidth), height(rheight) {}
  Rect(const Rect& rc)
    : x(rc.x), y(rc.y), width(rc.width), height(rc.height) {}
  Rect& operator=(const Rect& rc);
  operator class RectPt() const;
};

inline Rect& Rect::operator=(const Rect& rc)
{
  x = rc.x;
  y = rc.y;
  width = rc.width;
  height = rc.height;

  return *this;
}

/*
 * RectPt class
 */
class RectPt {
public:
  int left, top, right, bottom;

public:
  RectPt() {}
  RectPt(int left, int top, int right, int bottom)
    : left(left), top(top), right(right), bottom(bottom) {}
  RectPt(const RectPt& rp)
    : left(rp.left), top(rp.top), right(rp.right), bottom(rp.bottom) {}
  RectPt& operator=(const RectPt& rp) {
    left = rp.left;  top = rp.top;  right = rp.right;  bottom = rp.bottom;
    return *this;
  }
  operator Rect() const {
    return Rect(left, top, right - left + 1, bottom - top + 1);
  }
};

inline Rect::operator RectPt() const
{
  return RectPt(x, y, x + width - 1, y + height - 1);
}

/*
 * Dim class
 */
class Dim {
public:
  int width, height;

public:
  Dim() {}
  Dim(int dwidth, int dheight)
    : width(dwidth), height(dheight) {}
  Dim(const Dim& dim)
    : width(dim.width), height(dim.height) {}
  Dim& operator=(const Dim& dim) {
    width = dim.width;  height = dim.height;
    return *this;
  }
};

#endif // _MISC_H_
