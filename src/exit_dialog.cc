/*
 * exit_dialog.cc
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
#include <X11/Xresource.h>
#include "main.h"
#include "misc.h"
#include "exit_dialog.h"
#include "resource.h"
#include "radio_button.h"
#include "qvwm.h"
#include "event.h"
#include "qvwmrc.h"
#include "desktop.h"
#include "image.h"
#include "confirm_dialog.h"

ExitDialog::ExitDialog()
: SystemDialog()
{
  int i = 0, j, base = 12;
  ResourceId dlgid = STARTID, initRB = 0;
  ItemKind prevItem = ICONPIXMAP;
  const char* exitTitle = "Exit qvwm";
  QvImage* img;
  MenuElem* mItem;
  DialogRes** dr;
  
  nitems = GetMenuItemNum(ExitDlgItem);
  dr = new DialogRes*[nitems + 1];
  rFunc = new Function*[nitems];
  
  for (j = 0; j < nitems; j++)
    rFunc[j] = NULL;

  mItem = ExitDlgItem;

  while (mItem) {
    if (strcmp(mItem->name, "StaticText") == 0) {
      if (prevItem != STATICTEXT)
	base += 9;
      if (prevItem == STRINGBUTTON)
	base += 36;
      
      ASSERT(i <= nitems);
      
      Rect rect(70, base, -1, -1);
      dr[i++] = new DialogRes(STATICTEXT, NO_ID, rect, mItem->file,
			      fsDialog);
      base += 21;
      prevItem = STATICTEXT;
    }
    else if (strcmp(mItem->name, "RadioButton") == 0) {
      if (prevItem != RADIOBUTTON)
	base += 9;
      if (prevItem == STRINGBUTTON)
	base += 36;
      
      ASSERT(dlgid - STARTID >= 0 && dlgid - STARTID < nitems);
      
      rFunc[dlgid - STARTID] = new Function(mItem->func, mItem->exec);
      
      if (initRB == 0)
	initRB = dlgid;
      
      ASSERT(i <= nitems);
      
      Rect rect(75, base, -1, -1);
      dr[i++] = new DialogRes(RADIOBUTTON, dlgid++, rect, mItem->file,
			      fsDialog);
      base += 21;
      prevItem = RADIOBUTTON;
    }
    else if (strcmp(mItem->name, "IconImage") == 0 ||
	     strcmp(mItem->name, "IconPixmap") == 0) {
      Rect rect(21, 17, 32, 32);
      img = CreateImageFromFile(mItem->file, Dialog::dlgTimer);
      if (img) {
	ASSERT(i <= nitems);
	dr[i++] = new DialogRes(ICONPIXMAP, dlgid++, rect, fsDialog, NO_ID,
				img);
      }
    }
    else if (strcmp(mItem->name, "OKButton") == 0) {
      if (prevItem != STRINGBUTTON)
	base += 22;
      
      ASSERT(i <= nitems);
      
      Rect rect(84, base, 88, 23);
      dr[i++] = new DialogRes(STRINGBUTTON, ID_OK, rect, mItem->file,
			      fsDialog);
      prevItem = STRINGBUTTON;
    }
    else if (strcmp(mItem->name, "CancelButton") == 0) {
      if (prevItem != STRINGBUTTON)
	base += 22;
      
      ASSERT(i <= nitems);
      
      Rect rect(179, base, 88, 23);
      dr[i++] = new DialogRes(STRINGBUTTON, ID_CANCEL, rect, mItem->file,
			      fsDialog);
      prevItem = STRINGBUTTON;
    }
    else if (strcmp(mItem->name, "HelpButton") == 0) {
      if (prevItem != STRINGBUTTON)
	base += 22;
      
      ASSERT(i <= nitems);
      
      Rect rect(274, base, 88, 23);
      dr[i++] = new DialogRes(STRINGBUTTON, ID_HELP, rect, mItem->file,
			      fsDialog);
      prevItem = STRINGBUTTON;
    }
    else if (strcmp(mItem->name, "Title") == 0)
      exitTitle = mItem->file;
    else
      QvwmError("Type '%s' is invalid", mItem->name);
    
    mItem = mItem->next;
  }
  if (initRB != 0) {
    ASSERT(i <= nitems);
    dr[i++] = new DialogRes(RADIOSET, IDS_1, initRB);
  }
  
  CreateDialogResource(dr, i);

  for (j = 0; j < i; j++)
    delete dr[j];
  delete [] dr;

  Rect rcRoot = rootQvwm->GetRect();
  Rect rect((rcRoot.width - ExitDlgWidth) / 2,
	    (rcRoot.height - (base + 37)) / 2,
	    ExitDlgWidth, base + 37);
  
  SetRect(rect);
  SetTitle(exitTitle);
}

ExitDialog::~ExitDialog()
{
  for (int i = 0; i < nitems; i++)
    delete rFunc[i];
  delete [] rFunc;
}

void ExitDialog::MapDialog()
{
  DarkenScreen();

  SystemDialog::MapDialog();
}

void ExitDialog::ProcessDialog()
{
  ResourceId id;
  Function* func;

  XGrabServer(display);

  while (1) {
    id = EventLoop();

    switch (id) {
    case ID_OK:
      UnmapDialog();
      
      ASSERT(GetSelectedRB(IDS_1) >= 0 &&
	     GetSelectedRB(IDS_1) - STARTID < nitems);

      func = rFunc[GetSelectedRB(IDS_1) - STARTID];
      switch (func->GetFuncNumber()) {
      case Q_EXIT:
	if (!UseConfirmDialog || GetExistingWins() == 0) {
	  XUngrabServer(display);
	  RefreshScreen();
	  FinishQvwm();
	}
	else {
	  DarkenScreen();

	  if (confirmDlg == NULL)
	    confirmDlg = new ConfirmDialog();
	  confirmDlg->MapDialog();
	  confirmDlg->ProcessDialog();
	  return;
	}
	break;
	
      case Q_EXEC:
	XUngrabServer(display);
	RefreshScreen();
	ExecCommand(func->GetExec());
	break;
	
      default:
	XUngrabServer(display);
	RefreshScreen();
	QvFunction::execFunction(func->GetFuncNumber());
      }
      return;
      
    case ID_CANCEL:
      XUngrabServer(display);
      UnmapDialog();
      RefreshScreen();
      return;
    }
  }
}

int ExitDialog::GetExistingWins()
{
  List<Qvwm> qvwmList = desktop.GetQvwmList();

  return qvwmList.GetSize();
}
