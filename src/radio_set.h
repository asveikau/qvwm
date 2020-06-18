/*
 * radio_set.h
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

#ifndef _RADIO_SET_H_
#define _RADIO_SET_H_

#include "resource.h"

class RadioButton;

/*
 * RadioSet class
 */
class RadioSet {
  friend class RadioButton;
private:
  RadioButton** rb;               // RadioButton belonging to this set
  int num;                        // # of RadioButton
  RadioButton* rbFocus;           // RadioButton with focus
  ResourceId rid;

public:
  RadioSet(RadioButton* rbutton[], int rbNum, ResourceId res_id,
	   ResourceId initId);
  ~RadioSet();

  ResourceId GetResId() const { return rid; }
  ResourceId GetFocusRB();

  void MapRadioButtons();
};

#endif // _RADIO_SET_H_
