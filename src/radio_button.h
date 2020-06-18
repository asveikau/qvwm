/*
 * radio_button.h
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

#ifndef _RADIO_BUTTON_H_
#define _RADIO_BUTTON_H_

#include "resource.h"

#define RADIO_PUSH     (1 << 0)
#define RADIO_CHECK    (1 << 1)

class RadioSet;
class QvImage;

/*
 * RadioButton class
 */
class RadioButton {
private:
  Window frame;                      // radio button frame
  Point pt;                          // position
  unsigned int status;               // status
  char* name;                        // right text of radio button
  XFontSet& fs;
  ResourceId rid;

  int rbY, rnY;
  Rect rc;

  RadioSet* rs;

  QvImage* imgRadioBack;

  static const int BUTTON_SIZE = 12;
  static const int CHECK_SIZE = 4;

public:
  static XContext context;

  static QvImage* imgRadio[3];

public:
  RadioButton(Window parent, const Point& point, char* btnname,
	      XFontSet& btnfs, ResourceId res_id);
  ~RadioButton();
  void SetStatus(unsigned int st) { status |= st; }
  void SetRS(RadioSet* set) { rs = set; }
  ResourceId GetResId() { return rid; }
  
  void MapButton();
  void DrawButton(Bool all);
  void Button1Press();

  static void Initialize();
};

#endif // _RADIO_BUTTON_H_
