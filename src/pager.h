/*
 * pager.h
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

#ifndef _PAGER_H_
#define _PAGER_H_

#include "misc.h"
#include "util.h"

class QvImage;

/*
 * Pager class
 */
class Pager {
friend class Miniature;
private:
  Window frame;
  Window title;
  Window pages;             // window standing for whole page
  Window visual;            // window standing for current page
  Rect rc;                  // size and position of frame
  Rect rcOrig;              // size and position of pages
  Point gravity;            // gravity offset (see util.cc)
  Dim pageSize;             // size of one page in pager

  QvImage* imgPager;

  static const int BORDER_WIDTH = 3;
  static const int TITLE_HEIGHT = 7;

public:
  Pager(InternGeom geom);
  ~Pager();

  Window GetFrameWin() const { return frame; }
  Window GetVisualWin() const { return visual; }
  Window GetPagesWin() const { return pages; }
  Rect GetRect() const { return rc; }

  void MapPager();
  void UnmapPager();

  Bool IsPagerWindows(Window win);
  void CalcPagerPos();
  void RecalcPager();
  Point ConvertToPagerPos(const Point& pt);
  Rect ConvertToPagerSize(const Rect& rect);
  Point ConvertToRealPos(const Point& pt);

  void DrawPages();
  void DrawVisualPage();
  void DrawFrame();
  void Button1Press(Window win, const Point& ptRoot, const Point& ptWin);
  void Button3Press(const Point& ptRoot);
  void Exposure(Window win);

  void RaisePager();
};

#endif // _PAGER_H_
