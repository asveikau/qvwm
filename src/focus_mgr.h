/*
 * focus_mgr.h
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

#ifndef FOCUS_MGR_H_
#define FOCUS_MGR_H_

#include "list.h"

class Qvwm;

/*
 * FocusMgr keeps only non-trasient windows without NOFOCUS.
 * This list stands for an order which a focus moves according to.
 */
class FocusMgr {
private:
  List<Qvwm> mapList;
  List<Qvwm> unmapList;

public:
  void Push(Qvwm* qvWm) {
    ASSERT(Check(qvWm));
    mapList.InsertHead(qvWm);
  }
  Qvwm* Pop() { return mapList.RemoveHead(); }

  void InsertBottom(Qvwm* qvWm) {
    ASSERT(Check(qvWm));
    mapList.InsertTail(qvWm);
  }
  Qvwm* RemoveBottom() { return mapList.RemoveTail(); }

  Bool Remove(Qvwm* qvWm) { return mapList.Remove(qvWm); }

  Qvwm* Top() const { return mapList.GetHead(); }
  Qvwm* TopUnmapList() const { return unmapList.GetHead(); }

  void InsertUnmapList(Qvwm* qvWm) { unmapList.InsertTail(qvWm); }
  Bool RemoveUnmapList(Qvwm* qvWm) { return unmapList.Remove(qvWm); }

  List<Qvwm>& GetMapList() { return mapList; }
  List<Qvwm>& GetUnmapList() { return unmapList; }

  int GetAllNum() const { return mapList.GetSize() + unmapList.GetSize(); }

  void SetFocus(Qvwm* qvWm);
  void ResetFocus(Qvwm* qvWm);

  void RollFocus(Bool forward);
  void RollFocusWithinScreen(Bool forward);

public:
  void DumpStack();
  Bool Check(Qvwm* qvWm);
};

#endif // FOCUS_MGR_H_
