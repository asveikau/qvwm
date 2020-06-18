/*
 * indicator.cc
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
#include <string.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "misc.h"
#include "util.h"
#include "indicator.h"
#include "taskbar.h"
#include "qvwm.h"
#include "qvwmrc.h"

List<Indicator> Indicator::indList;
List<Indicator> Indicator::pendingList;
int Indicator::INDICATOR_SIZE;

Indicator::Indicator(char* exec, char* comp)
: wOrig(None), name(comp)
{
  pendingList.InsertTail(this);

  pid = ExecCommand(exec);
}  

Indicator::~Indicator()
{
  if (wOrig != None)
    XDestroyWindow(display, parent);

  if (TaskbarImage)
    QvImage::Destroy(imgIndicator);

  kill(pid, SIGKILL);
}

void Indicator::CreateIndicator(Window w)
{
  wOrig = w;
  pendingList.Remove(this);

  XSetWindowAttributes attributes;
  unsigned long valueMask;
  
  attributes.background_pixel = gray.pixel;
  valueMask = CWBackPixel;

  parent = XCreateWindow(display, taskBar->tbox,
			 -INDICATOR_SIZE, -INDICATOR_SIZE,
			 INDICATOR_SIZE, INDICATOR_SIZE,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);

  if (TaskbarImage) {
    QvImage* img = taskBar->GetTaskbarImage();
    imgIndicator = img->Duplicate();
    imgIndicator->SetBackground(parent);
  }

  /*
   * Change window property for indicator.
   */
  XSetWindowBorderWidth(display, w, 0);
  XResizeWindow(display, w, INDICATOR_SIZE, INDICATOR_SIZE);
  XReparentWindow(display, w, parent, 0, 0);

  XMapWindow(display, parent);
  XMapWindow(display, w);

  indList.InsertTail(this);

  taskBar->MoveResizeTaskbarBox();
  taskBar->DrawTaskbarBox();
  taskBar->RedrawAllTaskbarButtons();
}

/*
 * RedrawAllIndicators --
 *   Relocate all indicators in the taskbar box.
 */
int Indicator::RedrawAllIndicators()
{
  int num, needWidth, row;
  int tWidth = taskBar->rc[taskBar->GetPos()].width;

  num = indList.GetSize();
  if (num == 0)
    return Taskbar::TBOX_MARGIN - Taskbar::IC_MARGIN;

  /*
   * necessary width for taskbar box.
   */
  needWidth = num * (INDICATOR_SIZE + 1) + IND_MARGIN * 2 +
    Taskbar::IC_MARGIN + taskBar->clockWidth + Taskbar::TBOX_MARGIN;

  /*
   * row is the number of indicators per one row.
   */
  row = (tWidth - 15) / (INDICATOR_SIZE + 1);
  if (row == 0)
    row = 1;

  List<Indicator>::Iterator i(&indList);
  Indicator* ind;
  int j;

  for (ind = i.GetHead(), j = 0; j < num; j++, ind = i.GetNext()) {
    int x, y;

    x = 1 + IND_MARGIN + (INDICATOR_SIZE + 1) * (j % row);
    if (needWidth <= tWidth - 9)
      y = (TaskbarButton::BUTTON_HEIGHT - INDICATOR_SIZE) / 2;
    else
      y = TaskbarButton::BUTTON_HEIGHT + (INDICATOR_SIZE + 2) * (j / row);

    ind->MoveIndicator(x, y);
  }

  if (needWidth <= tWidth - 9) {
    taskBar->dTbox.height = TaskbarButton::BUTTON_HEIGHT;
    return (INDICATOR_SIZE + 1) * num + IND_MARGIN * 2;
  }
  else {
    taskBar->dTbox.height = TaskbarButton::BUTTON_HEIGHT
      + (INDICATOR_SIZE + 2) * ((num + row - 1) / row) + 1;
    return Taskbar::TBOX_MARGIN - Taskbar::IC_MARGIN;
  }
}

/*
 * LookInList --
 *   Look for a pending indicator with name in pending list.
 */
Indicator* Indicator::LookInList(char* name, XClassHint& classHints)
{
  List<Indicator>::Iterator i(&pendingList);

  for (Indicator* ind = i.GetHead(); ind; ind = i.GetNext())
    if ((name && strcmp(ind->name, name) == 0) ||
	(classHints.res_name && strcmp(ind->name, classHints.res_name) == 0) ||
	(classHints.res_class && strcmp(ind->name, classHints.res_class) == 0))
      return (ind);

  return NULL;
}

Bool Indicator::NotifyDeadPid(pid_t pid)
{
  List<Indicator>::Iterator i(&indList);

  for (Indicator* ind = i.GetHead(); ind; ind = i.GetNext()) {
    if (pid == ind->pid) {
      i.Remove();
      delete ind;
      taskBar->MoveResizeTaskbarBox();
      taskBar->DrawTaskbar();
      return True;
    }
  }

  return False;
}

void Indicator::Initialize()
{
  INDICATOR_SIZE = IndicatorSize;
}
