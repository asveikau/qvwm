/*
 * exec.h
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

#ifndef EXEC_H_
#define EXEC_H_

#include "misc.h"
#include "list.h"

class ExecPending {
private:
  Point m_pageoff;
  char* m_comp;

public:
  static List<ExecPending> m_pendingList;

public:
  ExecPending(const Point& pageoff, char* comp);
  ~ExecPending() { delete m_comp; }

  Point GetPageOff() const { return m_pageoff; }

  static ExecPending* LookInList(char* name, XClassHint& classHints);
};

#endif // EXEC_H_
