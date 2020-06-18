/*
 * string_button.h
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

#ifndef _STRING_BUTTON_H_
#define _STRING_BUTTON_H_

#include "button.h"
#include "resource.h"

class StringButton : public Button {
private:
  char* str;
  XFontSet& fs;
  ResourceId rid;
  
  Bool triggered;

public:
  StringButton(Window parent, const Rect& rc, char* label, XFontSet& labelfs,
	       ResourceId res_id);
  ~StringButton() {}

  Bool IsTriggered() { return triggered; }
  ResourceId GetResId() { return rid; }
  
  void DrawButton();
  void ExecButtonFunc(ButtonState state);
};

#endif // _STRING_BUTTON_H_
