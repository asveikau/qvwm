/*
 * screen_saver.cc
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

#ifdef USE_SS

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "screen_saver.h"
#include "main.h"
#include "misc.h"
#include "timer.h"
#include "qvwm.h"

ScreenSaver::ScreenSaver(char* locker, unsigned long lockSec)
{
  m_lockerPid = 0;
  m_locker = locker;
  m_lockMsec = lockSec * 1000;

  if (lockSec == 0) {
    m_ssSupport = False;
    return;
  }

#ifdef USE_SSEXT
  int ssEventBase, ssErrorBase;

  m_ssSupport = XScreenSaverQueryExtension(display, &ssEventBase,
					   &ssErrorBase);
  if (!m_ssSupport) {
    QvwmError("Screen saver extension not supported");
    return;
  }

  m_info = XScreenSaverAllocInfo();
#else // USE_SSEXT
  m_ssSupport = True;

  Callback<ScreenSaver>* cb =
    new Callback<ScreenSaver>(this, &ScreenSaver::QueryPointer);
  timer->SetTimeout(1000, cb);  // 1sec

  SelectEventsForAll();
#endif // USE_SSEXT

  timer->SetTimeout(m_lockMsec,
		    new Callback<ScreenSaver>(this, &ScreenSaver::Start));
}

ScreenSaver::~ScreenSaver()
{
#ifdef USE_SSEXT
  if (m_ssSupport)
    XFree(m_info);
#endif
}

void ScreenSaver::Start()
{
#ifdef USE_SSEXT
  XScreenSaverQueryInfo(display, root, m_info);

  if (m_info->idle < m_lockMsec) {
    timer->SetTimeout(m_lockMsec - m_info->idle,
		      new Callback<ScreenSaver>(this, &ScreenSaver::Start));
    return;
  }
#endif // USE_SSEXT

  if (m_lockerPid == 0) {
    rootQvwm->InstallWindowColormaps();

    m_lockerPid = vfork();
    if (m_lockerPid == 0) {
      execl("/bin/sh", "/bin/sh", "-c", m_locker, NULL);
      QvwmError("cannot start screen saver %s", m_locker);
      _exit(1);
    }
  }
}

Bool ScreenSaver::NotifyDeadPid(pid_t pid)
{
  if (pid == m_lockerPid) {
    m_lockerPid = 0;
    timer->SetTimeout(m_lockMsec,
		      new Callback<ScreenSaver>(this, &ScreenSaver::Start));
    return True;
  }

  return False;
}

#ifndef USE_SSEXT
void ScreenSaver::SelectEventsForAll()
{
  SelectEvents(root);

  Callback<ScreenSaver>* cb =
    new Callback<ScreenSaver>(this, &ScreenSaver::SelectEventsForAll);
  timer->SetTimeout(10000, cb);  // 10sec
}

void ScreenSaver::SelectEvents(Window win)
{
  XWindowAttributes attr;
  int keyMask, newMask;
  Window junkRoot, junkParent;
  Window *children;
  unsigned int nChildren;

  if (!m_ssSupport)
    return;

  if (win != root) {
    if (XGetWindowAttributes(display, win, &attr) == 0)
      return;

    // if a foreign window
    if (attr.your_event_mask == 0 ||
	attr.your_event_mask & SubstructureNotifyMask) {
      keyMask = (attr.all_event_masks | attr.do_not_propagate_mask)
	& KeyPressMask;
      newMask = attr.your_event_mask | SubstructureNotifyMask | keyMask;

      XSelectInput(display, win, newMask);
    }
  }

  if (!XQueryTree(display, win, &junkRoot, &junkParent, &children, &nChildren))
    return;

  for (int i = 0; i < nChildren; i++)
    SelectEvents(children[i]);

  if (nChildren)
    XFree(children);
}

void ScreenSaver::ResetTimer()
{
  if (!m_ssSupport)
    return;

  timer->ResetTimeout(new Callback<ScreenSaver>(this, &ScreenSaver::Start));
  timer->SetTimeout(m_lockMsec,
		    new Callback<ScreenSaver>(this, &ScreenSaver::Start));
}

// approximation instead of setting *MotionMask to all windows
void ScreenSaver::QueryPointer()
{
  Window junkRoot, junkChild;
  Point ptRoot, ptJunk;
  unsigned int mask;

  XQueryPointer (display, root, &junkRoot, &junkChild, &ptRoot.x, &ptRoot.y,
		 &ptJunk.x, &ptJunk.y, &mask);
      
  if (m_ptPrevRoot.x != ptRoot.x || m_ptPrevRoot.y != ptRoot.y ||
      m_prevMask != mask) {
    ResetTimer();

    m_ptPrevRoot = ptRoot;
    m_prevMask = mask;
  }
  
  Callback<ScreenSaver>* cb =
    new Callback<ScreenSaver>(this, &ScreenSaver::QueryPointer);
  timer->SetTimeout(1000, cb);
}
#endif // !USE_SSEXT

#endif // USE_SS
