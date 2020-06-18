/*
 * system_dialog.h
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

#ifndef _SYSDLG_H_
#define _SYSDLG_H_

#include "dialog.h"

class StringButton;
class RadioSet;

class SystemDialog : public Dialog {
private:
  Window parent;
  Window title;

  int sbNum;                      // # of string button
  int rsNum;                      // # of radio button set
  int stNum;                      // # of explanation texts
  int ipNum;                      // # of icon pixmaps

  StringButton** sb;              // string buttons
  RadioSet** rs;                  // radio button sets
  StaticText** st;                // explanation texts
  IconPixmap** ip;                // icon pixmaps on system dialog

  ResourceId scKeyTable[256];

  static const int TITLE_HEIGHT = 18;
  
public:
  SystemDialog(const Rect& rc = Rect(0, 0, 1, 1));
  virtual ~SystemDialog();

  ResourceId GetSelectedRB(ResourceId idRadioSet);
  void CreateDialogResource(DialogRes** dr, int drNum);
  void DrawDialog();
  void DrawTitle();
  void SetRect(const Rect& rect);

  virtual void MapDialog();
  virtual void UnmapDialog();

  void DrawClientWin();
  void Exposure(Window win);
  ResourceId Button1Press(Window win);
  ResourceId FindShortCutKey(char key);

  void SetShortcutKeyTable(char* str, ResourceId id);
};

#endif // _SYSDLG_H_
