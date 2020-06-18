/*
 * radio_set.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "misc.h"
#include "qvwm.h"
#include "radio_set.h"
#include "radio_button.h"

RadioSet::RadioSet(RadioButton* rbutton[], int rbNum, ResourceId res_id,
		   ResourceId initId)
: rb(rbutton), num(rbNum), rid(res_id)
{
  int i;

  for (i = 0; i < rbNum; i++) {
    ASSERT(rb[i]);
    rb[i]->SetRS(this);
  }

  for (i = 0; i < rbNum; i++) 
    if (rb[i]->GetResId() == initId)
      rbFocus = rb[i];

  ASSERT(rbFocus);

  rbFocus->SetStatus(RADIO_CHECK);
}

RadioSet::~RadioSet()
{
  for (int i = 0; i < num; i++)
    delete rb[i];
  if (num > 0)
    delete [] rb;
}

/*
 * MapRadioButtons --
 *   Map all radio buttons in this radio set.
 */
void RadioSet::MapRadioButtons()
{
  for (int i = 0; i < num; i++) {
    ASSERT(rb[i]);
    rb[i]->MapButton();
  }
}

/*
 * GetFocusRB --
 *   Get focus radio button in this radio set.
 */
ResourceId RadioSet::GetFocusRB()
{
  for (int i = 0; i < num; i++)
    if (rbFocus == rb[i]) {
      ASSERT(rb[i]);
      return rb[i]->GetResId();
    }

  return NO_ID;
}
