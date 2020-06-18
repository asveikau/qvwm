/*
 * exec.cc
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
#include "main.h"
#include "exec.h"
#include "qvwm.h"

ExecPending::ExecPending(const Point& pageoff, char* comp)
: m_pageoff(pageoff)
{
  m_comp = new char[strlen(comp) + 1];
  strcpy(m_comp, comp);
}

ExecPending* ExecPending::LookInList(char* name, XClassHint& classHints)
{
  List<ExecPending>::Iterator i(&m_pendingList);

  for (ExecPending* pending = i.GetHead(); pending; pending = i.GetNext())
    if ((name && strcmp(pending->m_comp, name) == 0) ||
	(classHints.res_name &&
	 strcmp(pending->m_comp, classHints.res_name) == 0) ||
	(classHints.res_class &&
	 strcmp(pending->m_comp, classHints.res_class) == 0))
      return (pending);

  return NULL;
}
