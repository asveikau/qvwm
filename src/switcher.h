/*
 * switcher.h
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

#ifndef _SWITCH_H_
#define _SWITCH_H_

#include "misc.h"
#include "list.h"

class Qvwm;
class Timer;
class QvImage;

/*
 * TaskSwitcher class
 */
class TaskSwitcher {
private:
  Window frame;

  Window* itemwin;
  int winNum;

  Qvwm* qvWm;       // currently chosen application
  int focusWin;     // it's number in the menu

  Bool scroll;      // scroll the switcher window vertically
  int* scroll_to;

  Rect rc;

  // don't use these iterator for the other purposes
  List<Qvwm>::Iterator* iterMap;   // iterator to search next mapped window
  List<Qvwm>::Iterator* iterUnmap; // iterator to search next unmapped window
  Bool LookMapList;                // flag for looking in iterMap or iterUnmap

  Bool direct;  // forward if True

  QvImage* imgSwitcher;
  QvImage* imgTitle;

  
public:
  static Timer* swTimer;

private:
  void MapSwitcher( );
  void Unmap( );
  void EventLoop( KeyCode code, unsigned int mod);

  Qvwm* GetFirstFocus();
  Qvwm* GetNextFocus();
  Qvwm* GetPrevFocus();

  Bool FindApplication( Qvwm* qvWm );
  void SwitchToAndUnmap();  
  void Restart( KeyCode code, unsigned int mod );

  void DrawTaskSwitcher();
  void DrawSwitcherContents( int current );
  void DrawItem( int num, Qvwm* qvWm, Bool chosen );

public:
  TaskSwitcher();
  ~TaskSwitcher();

  void SwitchTask(Bool forward, KeyCode code, unsigned int mod);

  static void Initialize();
};

#endif // SWITCH_H
