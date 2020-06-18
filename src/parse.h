/*
 * parse.h
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

#ifndef _PARSE_H_
#define _PARSE_H_

#include "util.h"

enum VarAttribute {
  F_STRING,
  F_NATURAL,
  F_POSITIVE,
  F_FLOATING,
  F_COLOR,
  F_BOOL,
  F_TASKBAR,
  F_GEOMETRY,
  F_OFFSET,
  F_SIZE,
  F_GRADSTYLE,
  F_MODMASK
};

struct VariableTable {
  char* name;
  VarAttribute attr;
  void* var;
};

struct AttrStream {
  unsigned long attr;		// frame attribute
  Bool act;			// act direction (True:+, False:-)
  char* value;			// pixmap name etc.
  AttrStream* next;
};

class ShortCutKey;

extern void ParseQvwmrc(char* rcFileName);
extern FuncNumber GetFuncNumber(const char* func);
extern void AssignVariable(const char* var, char* str);
extern MenuElem* MakeExecItem(char* name, char* file, char* exec);
extern MenuElem* MakeDesktopItem(char* name, char* file, char* exec,
				 char* x, char* y);
extern MenuElem* MakeDesktopFuncItem(char* name, char* file, char* func,
				     char* x, char* y);
extern MenuElem* MakeFuncItem(char* name, char* file, char* func);
extern MenuElem* MakeDirItem(char* name, char* file, MenuElem* next);
extern MenuElem* MakeDlgItem(char* name, char* str, char* func);
extern MenuElem* MakeDlgFuncItem(char* name, char* str, char* func);
extern MenuElem* ChainMenuItem(MenuElem* mItem, MenuElem* nextItem);
extern void CompleteMenu(char* menuName, MenuElem* mList);
extern void DoAllSetting();
extern AttrStream* MakeStream(char* attr, char* value, AttrStream* stream);
extern void CreateAppHash(char* name, AttrStream* stream);
extern unsigned int MakeModifier(char* mod, unsigned int modifier);
extern void CreateSCKey(char* key, unsigned int mod, char* exec);
extern void CreateSCKeyFunc(char* key, unsigned int mod, char* func);
extern void CreateIndicator(char* comp, char* exec);
extern void CreateAccessory(char* filename, char* pos, char* mode);
extern Bool IsFunction(const char* text);
extern Bool IsAttribute(const char* text);

#endif // _PARSE_H_
