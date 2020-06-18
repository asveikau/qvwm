/*
 * timer.h
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

#ifndef _TIMER_H_
#define _TIMER_H_

#if defined(sun) && !defined(__SVR4)
extern "C" int gettimeofday(struct timeval *, struct timezone *);
#endif

#include <sys/time.h>
#include "list.h"
#include "callback.h"

class Timer {
private:
  class Callout {
  private:
    struct timeval timeo;
    BasicCallback* cb;

  public:
    Callout(unsigned long msec, BasicCallback* bcb) : cb(bcb) {
      struct timeval now;

      gettimeofday(&now, NULL);

      timeo.tv_sec = now.tv_sec;
      timeo.tv_usec = now.tv_usec + msec * 1000;
      if (timeo.tv_usec > 1000 * 1000) {
	timeo.tv_sec += timeo.tv_usec / (1000 * 1000);
	timeo.tv_usec %= 1000 * 1000;
      }
    }
    ~Callout() { delete cb; }

    struct timeval GetTimeout() const { return timeo; }
    BasicCallback* GetCallback() const { return cb; }

    void Execute() {
      if (cb)
	cb->Execute();
    }
  };
  
private:
  List<Callout> coList;

public:
  void SetTimeout(unsigned long msec, BasicCallback* cb);
  Bool ResetTimeout(BasicCallback* cb);
  void ResetAllTimeouts();
  Bool CheckTimeout(struct timeval* tm);
  void ForceTimeout(BasicCallback* cb);
};

inline struct timeval operator-(const struct timeval& tm1,
				const struct timeval& tm2)
{
  struct timeval tm;
  
  tm.tv_sec = tm1.tv_sec - tm2.tv_sec;
  tm.tv_usec = tm1.tv_usec - tm2.tv_usec;
  if (tm.tv_usec < 0) {
    tm.tv_sec--;
    tm.tv_usec += 1000 * 1000;
  }
  return tm;
}

inline Bool operator>(const struct timeval& tm1, const struct timeval& tm2)
{
  if (tm1.tv_sec > tm2.tv_sec ||
      (tm1.tv_sec == tm2.tv_sec && tm1.tv_usec > tm2.tv_usec))
    return True;
  else
    return False;
}

#endif // _TIMER_H_
