/*
 * confirm_dialog.cc
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
#include "confirm_dialog.h"
#include "resource.h"
#include "qvwm.h"

ConfirmDialog::ConfirmDialog()
: SystemDialog()
{
  Rect rcText(25, 20, -1, -1);
  char* text = "Some applications are still running.  Really exit?";
  Rect rcYesButton(109, 50, 88, 23);
  Rect rcNoButton(204, 50, 88, 23);
  DialogRes* dr[3];

  dr[0] = new DialogRes(STATICTEXT, NO_ID, rcText, text, fsDialog);
  dr[1] = new DialogRes(STRINGBUTTON, ID_OK, rcYesButton, "Y\\&es",
			fsDialog);
  dr[2] = new DialogRes(STRINGBUTTON, ID_CANCEL, rcNoButton, "N\\&o",
			fsDialog);

  CreateDialogResource(dr, 3);

  for (int i = 0; i < 3; i++)
    delete dr[i];

  Rect rcRoot = rootQvwm->GetRect();
  Rect rect((rcRoot.width - ConfirmDlgWidth) / 2,
	    (rcRoot.height - ConfirmDlgHeight) / 2,
	    ConfirmDlgWidth, ConfirmDlgHeight);
  
  SetRect(rect);
  SetTitle("Confirm");
}

void ConfirmDialog::ProcessDialog()
{
  ResourceId id;

  XGrabServer(display);

  id = EventLoop();

  XUngrabServer(display);
  UnmapDialog();
  RefreshScreen();

  if (id == ID_OK)
    FinishQvwm();
}
