/*
 * screen_saver.h
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

#ifndef SCREEN_SAVER_H_
#define SCREEN_SAVER_H_

#ifdef USE_SS

#ifdef USE_SSEXT
#include <X11/extensions/scrnsaver.h>
#endif
#include "misc.h"

class ScreenSaver {
private:
  Bool m_ssSupport;
  unsigned long m_lockMsec;
  char* m_locker;
  pid_t m_lockerPid;

#ifdef USE_SSEXT
  XScreenSaverInfo* m_info;
#else
  Point m_ptPrevRoot;
  unsigned int m_prevMask;
#endif

public:
  ScreenSaver(char* locker, unsigned long lockSec);
  ~ScreenSaver();

  void Start();
  Bool NotifyDeadPid(pid_t pid);

#ifndef USE_SSEXT
  void SelectEventsForAll();
  void SelectEvents(Window win);
  void ResetTimer();
  void QueryPointer();
#endif
};

#endif // USE_SS

#endif // SCREEN_SAVER_H_
