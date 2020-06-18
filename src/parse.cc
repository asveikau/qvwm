/*
 * parse.cc
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
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include "main.h"
#include "util.h"
#include "taskbar.h"
#include "icon.h"
#include "key.h"
#include "parse.h"
#include "hash.h"
#include "qvwmrc.h"
#include "qvwm.h"
#include "indicator.h"
#include "desktop.h"
#include "accessory.h"
#include "timer.h"
#include "function.h"

static Hash<VariableTable>*	varHashTable;

extern FILE*	yyin;
extern int 	yyparse();
extern int 	line;
extern char     filename[256];

void SetFont();
#ifdef __EMX__
static char* TranslateForOS2(char* name);
#endif

/*
 * ParseQvwmrc --
 *   Parse '.qvwmrc', 'system.qvwmrc', etc.
 */
void ParseQvwmrc(char* rcFileName)
{
  char *home, *rcPath;
  int i;

  // This hash table is used if not parsing
  Qvwm::appHashTable = new Hash<AppAttr>(HashTableSize);

  if (noParse) {
    DoAllSetting();  // minimum setting
    return;
  }

  /*
   * Initialize hash table.
   */
  varHashTable = new Hash<VariableTable>(HashTableSize);
  for (i = 0; i < VarTableSize; i++)
    varHashTable->SetHashItem(varTable[i].name, &varTable[i]);

/*
  // set YYDEBUG in yaccsrc.yy to 1 if debugging
  extern int yydebug;
  yydebug = 1;
*/

  if (rcFileName != NULL) {
#ifdef __EMX__
    if ((strchr(rcFileName, '/') == NULL) &&
	(strchr(rcFileName, '\\') == NULL))
      yyin = fopen(rcFileName, "r");
    else
      yyin = fopen(__XOS2RedirRoot((char*)rcFileName), "r");
    if (yyin != NULL) {
#else    
    if ((yyin = fopen(rcFileName, "r")) != NULL) {
#endif    
      line = 1;
      strncpy(filename, rcFileName, 255);
      filename[255] = '\0';

      yyparse();
      fclose(yyin);
    }
    else {
      QvwmError("Can't open rcfile '%s'", rcFileName);
      SetFont();
    }

    delete varHashTable;

    return;
  }

  home = getenv("HOME");
  rcPath = new char[strlen(home)+9];
  sprintf(rcPath, "%s/.qvwmrc", home);

  if ((yyin = fopen(rcPath, "r")) != NULL) {
    line = 1;
    strncpy(filename, rcPath, 255);
    filename[255] = '\0';

    yyparse();
    fclose(yyin);
  }
  else {
    delete [] rcPath;
    rcPath = new char[strlen(QVWMDIR) + 15];
    sprintf(rcPath, "%s/system.qvwmrc", QVWMDIR);

#ifdef __EMX__
    if ((yyin = fopen(__XOS2RedirRoot(rcPath), "r")) != NULL) {
#else        
    if ((yyin = fopen(rcPath, "r")) != NULL) {
#endif    
      line = 1;
      strncpy(filename, rcPath, 255);
      filename[255] = '\0';

      yyparse();
      fclose(yyin);
    }
    else {
      QvwmError("Can't open file '%s'", rcPath);
      SetFont();
    }
  }

  delete [] rcPath;
  delete varHashTable;
}

/*
 * AssignVariable --
 *   Assign a value to string variable.
 */
void AssignVariable(const char* var, char* str)
{
  VariableTable* vItem;

  if ((vItem = varHashTable->GetHashItem(var))) {
    ASSERT(vItem);
    switch (vItem->attr) {
    case F_STRING:
      {
	char** sVal = (char **)vItem->var;
	*sVal = str;
      }
      break;

    case F_NATURAL:
      {
	int val;

	if ((val = atoi(str)) >= 0) {
	  int* nVal = (int *)vItem->var;
	  *nVal = val;
	}
	else
	  QvwmError("%d: '%s' is not natural number", line, str);
      }
      break;
	
    case F_POSITIVE:
      {
	int val;

	if ((val = atoi(str)) > 0) {
	  int* nVal = (int *)vItem->var;
	  *nVal = val;
	}
	else
	  QvwmError("%d: '%s' is not positive number", line, str);
      }
      break;
	
    case F_FLOATING:
      {
	double val;

	if ((val = strtod(str, NULL)) > 0) {
	  double* nVal = (double *)vItem->var;
	  *nVal = val;
	}
	else
	  QvwmError("%d: '%s' is not floating point number", line, str);
      }
      break;
	
    case F_COLOR:
      {
	XColor* pVal = (XColor *)vItem->var;

	if (GetXColorFromName(str, pVal) == -1)
	  QvwmError("%d: '%s' is invalid color", line, str);
      }
      break;

    case F_BOOL:
      {
	int* nVal = (int *)vItem->var;

	if (strcmp(str, "True") == 0)
	  *nVal = 1;
	else if (strcmp(str, "False") == 0)
	  *nVal = 0;
	else
	  QvwmError("%d: '%s' is not boolean", line, str);
      }
      break;

    case F_TASKBAR:
      {
	Taskbar::TaskbarPos* tVal = (Taskbar::TaskbarPos *)vItem->var;

	if (strcmp(str, "Bottom") == 0)
	  *tVal = Taskbar::BOTTOM;
	else if (strcmp(str, "Top") == 0)
	  *tVal = Taskbar::TOP;
	else if (strcmp(str, "Left") == 0)
	  *tVal = Taskbar::LEFT;
	else if (strcmp(str, "Right") == 0)
	  *tVal = Taskbar::RIGHT;
	else
	  QvwmError("%d: '%s' is not a taskbar position", line, str);
      }
      break;

    case F_GRADSTYLE:
      {
	Qvwm::GradStyle* tVal = (Qvwm::GradStyle *)vItem->var;

	if (strcmp(str, "Normal") == 0)
	  *tVal = Qvwm::Normal;
	else if (strcmp(str, "TopToBottom") == 0)
	  *tVal = Qvwm::TopToBottom;
	else if (strcmp(str, "LeftToRight") == 0)
	  *tVal = Qvwm::LeftToRight;
	else if (strcmp(str, "CenterToTopBottom") == 0)
	  *tVal = Qvwm::CenterToTopBottom;
	else if (strcmp(str, "CenterToLeftRight") == 0)
	  *tVal = Qvwm::CenterToLeftRight;
	else if (strcmp(str, "CenterToAll") == 0)
	  *tVal = Qvwm::CenterToAll;
	else
	  QvwmError("%d: '%s' is not a grad window style", line, str);
      }
      break;
	
    case F_GEOMETRY:
      {
	InternGeom* iVal = (InternGeom *)vItem->var;

	ASSERT(iVal);

	SetGeometry(str, iVal);
      }
      break;

    case F_OFFSET:
      {
	Point* pVal = (Point *)vItem->var;
	unsigned int dummy;
	
	ASSERT(pVal);

	XParseGeometry(str, &pVal->x, &pVal->y, &dummy, &dummy);
      }
      break;
      
    case F_SIZE:
      {
	Dim* dVal = (Dim *)vItem->var;
	int dummy;

	ASSERT(dVal);

	XParseGeometry(str, &dummy, &dummy,
		       (unsigned int *)&dVal->width,
		       (unsigned int *)&dVal->height);
      }
      break;

    case F_MODMASK:
      {
	unsigned int* mask = (unsigned int *)vItem->var;
	*mask = MakeModifier(str, 0);
      }	
      break;
    }
  }
  else
    QvwmError("%d: '%s' is unknown variable", line, var);
}

/*
 * MakeExecItem --
 *
 */
MenuElem* MakeExecItem(char* name, char* file, char* exec)
{
  MenuElem* mItem = new MenuElem();

  mItem->name = name;

#ifdef __EMX__
  mItem->file = TranslateForOS2(file);
#else
  mItem->file = file;
#endif

  mItem->func = Q_EXEC;

#ifdef __EMX__
  mItem->exec = TranslateForOS2(exec);
#else
  mItem->exec = exec;
#endif  

  mItem->child = NULL;

  return mItem;
}

/*
 * MakeDesktopItem --
 *
 */
MenuElem* MakeDesktopItem(char* name, char* file, char* exec, char* x, char* y)
{
  MenuElem* mItem;
  
  mItem = MakeExecItem(name, file, exec);

  mItem->x = mItem->y = -1;
  if (x != NULL) {
    if (*x == '!')
      mItem->x = atoi(x+1) + Icon::SFACTOR;
    else
      mItem->x = atoi(x);
  }
  if (y != NULL) {
    if (*y == '!')
      mItem->y = atoi(y+1) + Icon::SFACTOR;
    else
      mItem->y = atoi(y);
  }

  return mItem;
}  

/*
 * MakeDesktopFuncItem --
 *
 */
MenuElem* MakeDesktopFuncItem(char* name, char* file, char* func,
			      char* x, char* y)
{
  MenuElem* mItem;
  
  mItem = MakeFuncItem(name, file, func);

  mItem->x = mItem->y = -1;
  if (x != NULL) {
    if (*x == '!')
      mItem->x = atoi(x+1) + Icon::SFACTOR;
    else
      mItem->x = atoi(x);
  }
  if (y != NULL) {
    if (*y == '!')
      mItem->y = atoi(y+1) + Icon::SFACTOR;
    else
      mItem->y = atoi(y);
  }

  return mItem;
}  

/*
 * MakeFuncItem --
 */
MenuElem* MakeFuncItem(char* name, char* file, char* func)
{
  MenuElem* mItem = new MenuElem;
  FuncNumber* fNum;

  mItem->name = name;

#ifdef __EMX__
  mItem->file = TranslateForOS2(file);
#else
  mItem->file = file;
#endif  

  if ((fNum = QvFunction::funcHashTable->GetHashItem(func)) != NULL)
    mItem->func = *fNum;
  else
    mItem->func = Q_NONE;
  mItem->exec = "";
  mItem->child = NULL;

  return mItem;
}

/*
 * MakeDirItem --
 *
 */
MenuElem* MakeDirItem(char* name, char* file, MenuElem* child)
{
  MenuElem* mItem = new MenuElem();

  mItem->name = name;

#ifdef __EMX__
  mItem->file = TranslateForOS2(file);
#else
  mItem->file = file;
#endif  

  mItem->func = Q_DIR;
  mItem->exec = "";
  mItem->child = child;

  return mItem;
}

/*
 * MakeDlgFuncItem --
 */
MenuElem* MakeDlgFuncItem(char* name, char* str, char* func)
{
  MenuElem* mItem = new MenuElem();
  FuncNumber* fNum;

  mItem->name = name;
  mItem->file = str;
  if (func == NULL ||
      (fNum = QvFunction::funcHashTable->GetHashItem(func)) == NULL)
    mItem->func = Q_NONE;
  else
    mItem->func = *fNum;
  mItem->exec = "";
  mItem->child = NULL;

  return mItem;
}

/*
 * MakeDlgItem --
 */
MenuElem* MakeDlgItem(char* name, char* str, char* exec)
{
  MenuElem* mItem = new MenuElem();

  mItem->name = name;
  mItem->file = str;
  mItem->func = Q_EXEC;
  mItem->exec = exec;
  mItem->child = NULL;

  return mItem;
}

/*
 * ChainMenuItem --
 *   Chain two MenuItems.
 */
MenuElem* ChainMenuItem(MenuElem* mItem, MenuElem* nextItem)
{
  ASSERT(mItem);

  mItem->next = nextItem;

  return mItem;
}

/*
 * CompleteMenu --
 *   Complete menu.
 */
void CompleteMenu(char* menuName, MenuElem* mItem)
{
  if (strcmp(menuName, "Shortcuts") == 0) {
    ShortCutItem = mItem;
    return;
  }
  else if (strcmp(menuName, "ExitDialog") == 0) {
    ExitDlgItem = mItem;
    return;
  }

  if (mItem == NULL) {
    mItem = new MenuElem();

    mItem->name = "(None) ";
    mItem->func = Q_NONE;
    mItem->file = "";
    mItem->exec = "";
    mItem->next = NULL;
    mItem->child = NULL;
  }
  
  if (strcmp(menuName, "StartMenu") == 0)
    StartMenuItem = mItem;
  else if (strcmp(menuName, "CtrlMenu") == 0)
    CtrlMenuItem = mItem;
  else if (strcmp(menuName, "DesktopMenu") == 0)
    DesktopMenuItem = mItem;
  else if (strcmp(menuName, "IconMenu") == 0)
    IconMenuItem = mItem;
  else if (strcmp(menuName, "TaskbarMenu") == 0)
    TaskbarMenuItem = mItem;
}

/*
 * DoAllSetting --
 *   Complete Setting.
 */
void DoAllSetting()
{
  SetFont();

  // backward compatibility
  if (SystemStartSound == NULL)
    SystemStartSound = OpeningSound;
  if (SystemExitSound == NULL)
    SystemExitSound = EndingSound;
  if (SystemRestartSound == NULL)
    SystemRestartSound = RestartSound;
  if (RestoreUpSound == NULL && RestoreSound != NULL)
    RestoreUpSound = RestoreSound;
  if (RestoreDownSound == NULL && RestoreSound != NULL)
    RestoreDownSound = RestoreSound;
}

/*
 * MakeStream --
 *   Make stream item for attribute.
 */
AttrStream* MakeStream(char* attr, char* value, AttrStream* stream)
{
  AttrStream* newStream = new AttrStream;
  int i;

  newStream->next = stream;

  for (i = 0; i < AttrNum; i++)
    if (strcmp(attr, attrSet[i].name) == 0) {
      newStream->attr = attrSet[i].flag;
      newStream->act = attrSet[i].act;
      newStream->value = value;
      break;
    }

  if (i == AttrNum) {
    QvwmError("'%s' is invalid attribute.", attr);
    delete newStream;
    return stream;
  }

  return newStream;
}

/*
 * CreateAppHash --
 *   Create a hash for application attributes.
 */
void CreateAppHash(char* appName, AttrStream* stream)
{
  AttrStream* tmpStream;
  AppAttr* attrs;

  while (stream) {
    if (!(attrs = Qvwm::appHashTable->GetHashItem(appName))) {
      attrs = new AppAttr;
      attrs->flags = TITLE | BORDER | BORDER_EDGE | CTRL_MENU | BUTTON1
	| BUTTON2 | BUTTON3;
      Qvwm::appHashTable->SetHashItem(appName, attrs);
    }
    if (stream->act)
      attrs->flags |= stream->attr;
    else
      attrs->flags &= ~stream->attr;

    if (stream->attr == SMALL_IMG)
      attrs->small_file = stream->value;
    else if (stream->attr == LARGE_IMG)
      attrs->large_file = stream->value;
    else if (stream->attr == GEOMETRY)
      SetGeometry(stream->value, &attrs->geom);

    tmpStream = stream;
    stream = stream->next;

    delete tmpStream;
  }
}

/*
 * MakeModifier --
 *   Make OR'd modifier flag.
 */
unsigned int MakeModifier(char* mod, unsigned int modifier)
{
  int i;

  for (i = 0; i < KeyModNum; i++)
    if (strcmp(mod, keyMod[i].str) == 0) {
      modifier |= keyMod[i].mask;
      break;
    }

  if (i == KeyModNum)
    QvwmError("'%s' is invalid key modifier.", mod);

  return modifier;
}

/*
 * CreateSCKey --
 *   Make ShortCutKey from key + mod and command.
 */
void CreateSCKey(char* key, unsigned int mod, char* exec)
{
  KeySym sym;

  if ((sym = XStringToKeysym(key)) == NoSymbol) {
    if (key[0] == '#')
      scKey->AddSCKey((KeyCode)atoi(&key[1]), mod, exec);
    else
      QvwmError("'%s' is invalid key.", key);
  }
  else
    scKey->AddSCKey(sym, mod, exec);
}

/*
 * CreateSCKeyFunc --
 *   Make ShortCutKey from key + mod and func.
 */
void CreateSCKeyFunc(char* key, unsigned int mod, char* func)
{
  KeySym sym;
  FuncNumber* fNum;

  if ((fNum = QvFunction::funcHashTable->GetHashItem(func)) == NULL) {
    QvwmError("'%s' is invalid function.", func);
    return;
  }

  if ((sym = XStringToKeysym(key)) == NoSymbol) {
    if (key[0] == '#')
      scKey->AddSCKey((KeyCode)atoi(&key[1]), mod, *fNum);
    else
      QvwmError("'%s' is invalid key.", key);
  }
  else
    scKey->AddSCKey(sym, mod, *fNum);
}

/*
 * CreateIndicator --
 *   Create an indicator.
 */
void CreateIndicator(char* comp, char* exec)
{
  Indicator *ind;

  ind = new Indicator(exec, comp);
}

void CreateAccessory(char* filename, char* pos, char* mode)
{
  Accessory* acc;

  acc = new Accessory(filename, pos, mode);

  desktop.GetAccList().InsertTail(acc);
}

#define DEFAULT_FONT "-*-*-medium-r-normal-*-14-*-*-*-*-*-*-*,-*-*-medium-r-normal-*,*"

/*
 * SetFont --
 *   Set fonts.
 */
void SetFont()
{
  if (setlocale(LC_ALL, LocaleName) == NULL)
    QvwmError("Can't set the locale");

  // FontSets are created here, but they are freed in desktop.cc, in the
  // Desktop::FinishQvwm() function.

  char **miss, *def;
  int nMiss;
  int fsNum = (UseBoldFont) ? FsNum : FsNum - 1;

  // create default font
  char* fontname = new char[strlen(DefaultFont) + strlen(DEFAULT_FONT) + 2];
  sprintf(fontname, "%s,%s", DefaultFont, DEFAULT_FONT);
  fsDefault = XCreateFontSet(display, fontname, &miss, &nMiss, &def);
  delete [] fontname;

  if (fsDefault == NULL) {
    QvwmError("Can't load default font '%s' or any other font", DefaultFont);
    exit(1);
  }
  if (miss)
    XFreeStringList(miss);

  /*
   * Create specified FontSets, but keep times of XCreateFontSet() to a 
   * mininum because the function takes much time.
   */
  for (int i = 0; i < fsNum; i++) {
    Bool match = False;

    if (*fsSet[i].fontname == NULL ||
	strcmp(*fsSet[i].fontname, DefaultFont) == 0) {
      *fsSet[i].fs = fsDefault;
      continue;
    }

    // check if this font has already been loaded
    for (int j = 0; j < i; j++) {
      if (*fsSet[j].fontname &&
	  strcmp(*fsSet[i].fontname, *fsSet[j].fontname) == 0) {
	match = True;  // yes, it has been; so just reuse it
	*fsSet[i].fs = *fsSet[j].fs;
	break;
      }
    }

    if (!match) {  // this is the first time this font is attempted
      *fsSet[i].fs = XCreateFontSet(display, *fsSet[i].fontname, &miss, &nMiss,
				    &def);
      if (miss)
	XFreeStringList(miss);

      // XFontSet is pointer.
      if (*fsSet[i].fs == NULL) {
	QvwmError("Can't find font '%s'", *fsSet[i].fontname);
	*fsSet[i].fs = fsDefault;
      }
    }
  }
}

/*
 * IsFunction
 *   Return True if text is a function. The identifier beginning with 'QVWM_'
 *   is an internal function.
 */
Bool IsFunction(const char* text)
{
  char prefix[6];

  strncpy(prefix, text, 5);
  prefix[5] = '\0';

  if (strcmp(prefix, "QVWM_") == 0)
    return True;
  else
    return False;
}

/*
 * IsAttribute --
 *   Return True if text is an attribute.
 */
Bool IsAttribute(const char* text)
{
  for (int i = 0; i < AttrNum; i++)
    if (strcmp(text, attrSet[i].name) == 0)
      return True;

  return False;
}

#ifdef __EMX__
static char* TranslateForOS2(char* name)
{
  char* tName;

  if (name != NULL)
    if (strchr(name, '/') != NULL || strchr(name, '\\') != NULL) {
      char nom[300];

      strcpy(nom, __XOS2RedirRoot(name));
      tName = new char[strlen(nom) + 1];
      strcpy(tName, nom);

      return tName;
    }
  
  return name;
}
#endif  
