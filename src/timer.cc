/*
 * timer.cc
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
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "timer.h"

/*
 * SetTimeout --
 *   Set exclusive timeout.
 *   The argument cb is freed in timeout.
 */
void Timer::SetTimeout(unsigned long msec, BasicCallback* cb)
{
  /*
   * Execute soon if msec is 0.
   */
  if (msec == 0 && cb) {
    cb->Execute();
    delete cb;
    return;
  }

  Callout* newco = new Callout(msec, cb);

  // no element in a list
  if (coList.GetSize() == 0)
    coList.InsertHead(newco);
  else {
    List<Callout>::Iterator i(&coList);

    for (Callout* co = i.GetHead(); co; co = i.GetNext()) {
      if (co->GetTimeout() > newco->GetTimeout()) {
	i.InsertBefore(newco);
	return;
      }
    }

    // last element (the longest timeout)
    coList.InsertTail(newco);
  }
}

/*
 * ResetTimeout --
 *   Reset timeout.
 *   The argument cb is freed.
 */
Bool Timer::ResetTimeout(BasicCallback* cb)
{
  List<Callout>::Iterator i(&coList);
  Callout* co = i.GetHead();
  Bool hit = False;

  while (co) {
    if (cb->Equal(co->GetCallback())) {
      delete co;
      co = i.Remove();
      hit = True;
    }
    else
      co = i.GetNext();
  }

  delete cb;

  return hit;
}

void Timer::ResetAllTimeouts()
{
  List<Callout>::Iterator i(&coList);
  Callout* co = i.GetHead();

  while (co) {
    delete co;
    co = i.Remove();
  }
}  

/*
 * Return True if next timeout exists. (the rest of time is in *tm)
 */
Bool Timer::CheckTimeout(struct timeval* tm)
{
  if (coList.GetSize() == 0)
    return False;

  struct timeval now;
  List<Callout>::Iterator i(&coList);
  Callout* co = i.GetHead();

  gettimeofday(&now, NULL);

  while (co) {
    if (now > co->GetTimeout()) {
      i.Remove();
      co->Execute();
      delete co;
      co = i.GetHead(); // change list in co->Execute()
    }
    else
      co = i.GetNext();
  }

  if (coList.GetSize() > 0) {
    Callout* first = i.GetHead();
    *tm = first->GetTimeout() - now;
    return True;
  }

  return False;
}

/*
 * ForceTimeout --
 *   Timeout forcely and execute action.
 */
void Timer::ForceTimeout(BasicCallback* cb)
{
  List<Callout>::Iterator i(&coList);
  Callout* co = i.GetHead();
  
  while (co) {
    if (cb->Equal(co->GetCallback())) {
      i.Remove();
      co->Execute();
      delete co;
      co = i.GetHead();  // change list in co->Execute()
    }
    else
      co = i.GetNext();
  }
}
