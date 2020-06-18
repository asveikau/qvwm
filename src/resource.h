/*
 * resource.h
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

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "misc.h"
#include "main.h"

// Predefine
#define NO_ID     -1
#define ID_OK      0
#define ID_CANCEL  1
#define ID_HELP    2

typedef int ResourceId;

enum ItemKind {
  STRINGBUTTON,
  RADIOBUTTON,
  RADIOSET,
  STATICTEXT,
  ICONPIXMAP
};

class QvImage;

class DialogRes {
public:
  ItemKind kind;
  ResourceId rid;
  Rect rc;
  char* str;
  XFontSet& fs;
  ResourceId initId;
  QvImage* img;

public:
  // for STATICTEXT, RADIOBUTTON, STRINGBUTTON
  DialogRes(ItemKind ik, ResourceId res_id, const Rect& rect, char* label,
	    XFontSet& labelfs) : kind(ik), rid(res_id), rc(rect), str(label),
            fs(labelfs) {}
  // for ICONPIXMAP
  DialogRes(ItemKind ik, ResourceId res_id, const Rect& rect,
	    XFontSet& labelfs, ResourceId init, QvImage* image)
    : kind(ik), rid(res_id), rc(rect), fs(labelfs), initId(init), img(image)
      {}
  // for RADIOSET
  DialogRes(ItemKind ik, ResourceId res_id, ResourceId init)
    : kind(ik), rid(res_id), fs(fsDialog), initId(init) {}
};

class StaticText {
public:
  Point pt;
  char* text;
  XFontSet& fs;
  
public:
  StaticText(const Point& point, char* label, XFontSet& labelfs)
    : pt(point), text(label), fs(labelfs) {}
};

class IconPixmap {
public:
  QvImage* img;
  Point pt;

public:
  IconPixmap(QvImage* image, const Point& ptImg) : img(image), pt(ptImg) {}
};

#endif // _RESOURCE_H_
