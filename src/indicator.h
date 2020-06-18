/*
 * indicator.h
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

#ifndef _INDICATOR_H_
#define _INDICATOR_H_

#include "list.h"

class QvImage;

class Indicator {
private:
  Window wOrig;				// original window
  unsigned int bwOrig;			// original border width
  char* name;				// window indentical name
  pid_t pid;				// application pid
  Window parent;			// reparented window
  QvImage* imgIndicator;

  static List<Indicator> pendingList;

  static int INDICATOR_SIZE;
  static const int IND_MARGIN = 2;

public:
  static List<Indicator> indList;

public:
  Indicator(char* exec, char* comp);
  ~Indicator();

  void CreateIndicator(Window w);
  void MoveIndicator(int x, int y) { XMoveWindow(display, parent, x, y); }
  static int RedrawAllIndicators();
  static Indicator* LookInList(char* name, XClassHint& classHints);
  static Bool NotifyDeadPid(pid_t pid);
  static void Initialize();
};

#endif // _INDICATOR_H_
