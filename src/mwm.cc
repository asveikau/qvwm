/*
 * mwm.cc
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "main.h"
#include "mwm.h"
#include "qvwm.h"

Atom Mwm::XA_MOTIF_WM_HINTS;

void Mwm::Init()
{
  XA_MOTIF_WM_HINTS = XInternAtom(display, "_MOTIF_WM_HINTS", False);
}

void Mwm::GetHints(Qvwm* qvWm)
{
  MWMHints* hints = NULL;
  unsigned long flags = 0;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;

  XGetWindowProperty(display, qvWm->GetWin(), XA_MOTIF_WM_HINTS, 0, 20,
		     False, XA_MOTIF_WM_HINTS, &atype, &aformat, &nitems,
		     &bytes_remain, (unsigned char **)&hints);

  if (hints) {
    if (hints->mwmFlags & MWM_HINTS_FUNCTIONS) {
      if (hints->mwmFunctions & MWM_FUNC_ALL)
	;
      if (hints->mwmFunctions & MWM_FUNC_RESIZE)
	;
      if (hints->mwmFunctions & MWM_FUNC_MOVE)
	;
      if (hints->mwmFunctions & MWM_FUNC_MINIMIZE)
	;
      if (hints->mwmFunctions & MWM_FUNC_MAXIMIZE)
	;
      if (hints->mwmFunctions & MWM_FUNC_CLOSE)
	;
    }

    if (hints->mwmFlags & MWM_HINTS_DECORATIONS) {
      if (hints->mwmDecorations & MWM_DECOR_ALL)
	flags |= TITLE | BORDER | BORDER_EDGE | CTRL_MENU | BUTTON1 | BUTTON2;
      if (hints->mwmDecorations & MWM_DECOR_TITLE)
	flags |= TITLE;
      if (hints->mwmDecorations & MWM_DECOR_BORDER)
	flags |= BORDER | BORDER_EDGE;
      if (hints->mwmDecorations & MWM_DECOR_MENU)
	flags |= CTRL_MENU;
      if (hints->mwmDecorations & MWM_DECOR_MINIMIZE)
	flags |= BUTTON1;
      if (hints->mwmDecorations & MWM_DECOR_MAXIMIZE)
	flags |= BUTTON2;

      flags |= BUTTON3; // close button
    }
    else
      flags = TITLE | BORDER | BORDER_EDGE | CTRL_MENU | BUTTON1 | BUTTON2
	| BUTTON3;
  }
  else
    flags = TITLE | BORDER | BORDER_EDGE | CTRL_MENU | BUTTON1 | BUTTON2
      | BUTTON3;

  qvWm->SetFlags(flags);
}
