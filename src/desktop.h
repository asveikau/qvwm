/*
 * desktop.h
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

#ifndef DESKTOP_H_
#define DESKTOP_H_

#include "misc.h"
#include "list.h"
#include "taskbar.h"

class Qvwm;
class Icon;
class Accessory;
class QvImage;

class Desktop {
private:
  List<Qvwm> qvwmList;		// window list
  List<Icon> iconList;		// shortcut icon list
  List<Qvwm> onTopList;		// on top window list
  List<Accessory> accList;	// accessory list
  
  int initPos;

  QvImage* imgDesktop;

  Window topWin;                // fake window between ontop acc and ontop win

  Colormap curCmap;
  Qvwm*	curCmapQvwm;

public:
  Desktop();

  List<Qvwm>& GetQvwmList() { return qvwmList; }
  List<Icon>& GetIconList() { return iconList; }
  List<Qvwm>& GetOnTopList() { return onTopList; }
  List<Accessory>& GetAccList() { return accList; }

  Qvwm* LookInList(Window w);
  void CaptureAllWindows();
  Point GetNextPlace(const Dim& size, const Point& pageoff);
  void FinishQvwm(Bool reStart);
  void RecalcAllWindows();
  void ChangeFocusToCursor();
  void SetFocus();

  // Window rearrangement
  void Overlap(Bool all, const Rect& vt);
  void TileHorz(Bool all, const Rect& vt);
  void TileVert(Bool all, const Rect& vt);
  void MinimizeAll(Bool all, const Rect& vt = Rect());
  void RestoreAll(Bool all, const Rect& vt = Rect());
  void CloseAll(Bool all, const Rect& vt = Rect());

  void CreateIcons();
  void RedrawAllIcons();
  void LineUpAllIcons();

  Taskbar::TaskbarPos GetDesktopArea(const Point& pt);

  void SetWallPaper();

  void CreateTopWindow();
  Window GetTopWindow() const { return topWin; }

  void CreateAccessories();

  // colormap
  Qvwm* GetCmapInstalled() const { return curCmapQvwm; }
  void SetCmapInstalled(Qvwm* qvWm) { curCmapQvwm = qvWm; }
  Colormap GetCurrentCmap() const { return curCmap; }
  void SetCurrentCmap(Colormap cmap) { curCmap = cmap; }
};

#endif // DESKTOP_H_
