/*
 * exit_dialog.h
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

#ifndef EXIT_DIALOG_H_
#define EXIT_DIALOG_H_

#include "system_dialog.h"

// Resource ID
#define STARTID         100
#define IDS_1           200

class Function;

class ExitDialog : public SystemDialog {
private:
  Function** rFunc;
  int nitems;

  static const int ExitDlgWidth = 385;

private:
  int GetExistingWins();

public:
  ExitDialog();
  ~ExitDialog();

  void MapDialog();
  void ProcessDialog();
};

class Function {
private:
  FuncNumber fn;
  char* exec;

public:
  Function(FuncNumber num, char* cmd) : fn(num), exec(cmd) {}

  FuncNumber GetFuncNumber() const { return fn; }
  char* GetExec() const { return exec; }
};

#endif // EXIT_DIALOG_H_
