/*
 * util.h
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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <X11/Xutil.h>

#include "misc.h"
#include "image.h"
#include "function.h"

/*
 * Size and bitmask from GetGeometry().
 */
class InternGeom {
public:
  Rect rc;
  Point gravity;
  int bitmask;

public:
  InternGeom() {}
  InternGeom(int x, int y, unsigned int width, unsigned int height,
	     const Point& grav)
    : rc(Rect(x, y, width, height)), gravity(grav),
      bitmask(XValue|YValue|WidthValue|HeightValue) {}
};

/*
 * The origin of menu.
 */
struct MenuElem {
  char* name;
  char* file;
  FuncNumber func;
  char* exec;
  int x, y;
  MenuElem* next;
  MenuElem* child;
};

/*
 * Fontset and fontname.
 */
struct FsNameSet {
  XFontSet* fs;
  char** fontname;
};

/*
 * Attribute bit and the name.
 */
struct AttrNameSet {
  char* name;
  unsigned long flag;
  Bool act;
};

class AppAttr {
public:
  unsigned long flags;
  char* small_file;
  QvImage* small_img;
  char* large_file;
  QvImage* large_img;
  InternGeom geom;

  AppAttr() : flags(0), small_file(NULL), small_img(NULL),
              large_file(NULL), large_img(NULL) {}
  ~AppAttr() {
    if (small_img != NULL)
      QvImage::Destroy(small_img);
    if (large_img != NULL)
      QvImage::Destroy(large_img);
  }
};

/*
 * Key modifier
 */
struct KeyMod
{
  char* str;
  unsigned int mask;
};

/*
 * Offset
 */
const int CENTER = -1;
const int EAST = -2;
const int WEST = 0;
const int NORTH = 0;
const int SOUTH = -2;

extern Point GravityOffset[];

/*
 * inline function
 */
inline int Max(int x, int y)
{
  return ((x > y) ? x : y);
}

inline int Min(int x, int y)
{
  return ((x < y) ? x : y);
}

inline int RoundDown(int x, int y)
{
  return x / y * y;
}

inline int RoundUp(int x, int y)
{
  return (x + y - 1) / y * y;
}

inline int RoundOff(int x, int y)
{
  int rdx = x / y * y;

  return (x - rdx > y / 2) ? (rdx + y) : rdx;
}

class Menu;
class QvImage;
class Timer;

extern Bool InRect(const Point& pt, const Rect& rc);
extern Bool Intersect(const Rect& rc1, const Rect& rc2);
extern Bool IsDoubleClick(Time prevTime, Time clickTime, const Point& ptPrev,
			  const Point& ptClick);
extern int GetMenuItemNum(const MenuElem* mItem);
extern QvImage* CreateImageFromFile(char* file, Timer* timer);
extern QvImage* CreateImage(char* filename, Timer* timer);
extern char* GetFixName(XFontSet& fs, const char* name, int width);
extern Bool MappedNotOverride(Window w, long* state);
extern GC CreateTileGC(Drawable d);
extern void DarkenScreen();
extern void RefreshScreen();
extern pid_t ExecCommand(char* exec);
extern void RestoreCursor();
extern char* ExtractPathName(char* name);
extern char* GetNextDelim(char* path, char* name, int maxSize);

#ifndef HAVE_USLEEP
#ifdef __EMX__
#include <emx/syscalls.h>
#define usleep __sleep2
#else
extern void usleep(unsigned long);
#endif
#endif // HAVE_USLEEP

extern int GetRealWidth(XFontSet fs, char* str);
extern void DrawRealString(Window w, XFontSet fs, GC gc, int x, int y,
			   char* str);
extern void PlaySound(char* file, Bool sync = False);
extern Bool IsPointerInWindow(const Point& ptRoot);
extern int CreateColor(unsigned short red, unsigned short green,
		       unsigned short blue, XColor* color,
		       XColor* substitute = NULL, const char* comment = NULL);
Bool SetGeometry(char* str, InternGeom* geom);
void SetDisplayEnv(const char* displayName);
int GetXColorFromName(const char* str, XColor* color);

#endif // _UTIL_H_
