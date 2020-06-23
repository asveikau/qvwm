/*
 * yaccsrc.yy
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

%{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "parse.h"
#include "util.h"

extern int line;
extern char filename[256];
#if defined(__cplusplus)
extern "C" {
#endif
extern int yylex();
extern int yyerror(const char* error);
#if defined(__cplusplus)
}
#endif

#define YYDEBUG 0
%}

%union {
  char* str;
  MenuElem* mItem;
  AttrStream* aStream;
  unsigned int modifier;
}

%token <str> VARIABLE MENU SHORTCUT APP KEY IND EXITDLG STARTUP ACC
%token <str> VAR STRING FUNC PLUS MINUS

%type <str> session sessions ind inds cmd cmds acc accs
%type <mItem> item items sc scs dlgitem dlgitems
%type <aStream> stream
%type <modifier> mod
%type <sck> keys key

%%

qvwmrc:	  sessions			{ DoAllSetting(); }
	|				{ DoAllSetting(); }
	;

sessions: session sessions
	| session
	;

session:  VARIABLE vars
	| VARIABLE
	| APP apps
	| APP
	| MENU items			{ CompleteMenu($1, $2); }
	| MENU
	| SHORTCUT scs			{ CompleteMenu($1, $2); }
	| SHORTCUT
	| KEY keys
        | KEY
	| IND inds
	| IND
	| EXITDLG dlgitems		{ CompleteMenu($1, $2); }
	| EXITDLG
	| STARTUP cmds
	| STARTUP
	| ACC accs
	| ACC
	;

vars:	  var vars
	| var
	;

var:	  VAR '=' STRING		{ AssignVariable($1, $3); }
	| VAR '=' VAR			{ AssignVariable($1, $3); }
	;

items:	  item items			{ $$ = ChainMenuItem($1, $2); }
	| item				{ $$ = ChainMenuItem($1, NULL); }
	;

item:	  STRING STRING STRING		{ $$ = MakeExecItem($1, $2, $3); }
	| STRING STRING FUNC		{ $$ = MakeFuncItem($1, $2, $3); }
	| STRING STRING PLUS items MINUS { $$ = MakeDirItem($1, $2, $4); }
	;

scs:	  sc scs			{ $$ = ChainMenuItem($1, $2); }
	| sc				{ $$ = ChainMenuItem($1, NULL); }
	;

sc:	  STRING STRING STRING VAR ',' VAR
			{ $$ = MakeDesktopItem($1, $2, $3, $4, $6); }
	| STRING STRING STRING VAR
			{ $$ = MakeDesktopItem($1, $2, $3, $4, NULL); }
	| STRING STRING STRING ',' VAR
			{ $$ = MakeDesktopItem($1, $2, $3, NULL, $5); }
	| STRING STRING STRING
			{ $$ = MakeDesktopItem($1, $2, $3, NULL, NULL); }
	| STRING STRING FUNC VAR ',' VAR
			{ $$ = MakeDesktopFuncItem($1, $2, $3, $4, $6); }
	| STRING STRING FUNC VAR
			{ $$ = MakeDesktopFuncItem($1, $2, $3, $4, NULL); }
	| STRING STRING FUNC ',' VAR
			{ $$ = MakeDesktopFuncItem($1, $2, $3, NULL, $5); }
	| STRING STRING FUNC
			{ $$ = MakeDesktopFuncItem($1, $2, $3, NULL, NULL); }
	;
	
apps:	  app apps
	| app
	;

app:	  STRING stream			{ CreateAppHash($1, $2); }
	;

stream:   VAR ',' stream		{ $$ = MakeStream($1, NULL, $3); }
	| VAR				{ $$ = MakeStream($1, NULL, NULL); }
	| VAR '=' STRING ',' stream	{ $$ = MakeStream($1, $3, $5); }
	| VAR '=' STRING		{ $$ = MakeStream($1, $3, NULL); }
	;

dlgitems: dlgitem dlgitems		{ $$ = ChainMenuItem($1, $2); }
	| dlgitem			{ $$ = ChainMenuItem($1, NULL); }
	;

dlgitem:  VAR STRING FUNC		{ $$ = MakeDlgFuncItem($1, $2, $3); }
	| VAR STRING STRING		{ $$ = MakeDlgItem($1, $2, $3); }
	| VAR STRING			{ $$ = MakeDlgFuncItem($1, $2, NULL); }
	;

keys:	  key keys
	| key
	;

key:	  VAR mod STRING		{ CreateSCKey($1, $2, $3); }
	| VAR mod FUNC			{ CreateSCKeyFunc($1, $2, $3); }
	;

mod:	  VAR '|' mod			{ $$ = MakeModifier($1, $3); }
	| VAR				{ $$ = MakeModifier($1, 0); }
	;

inds:	  ind inds
	| ind
	;

ind:	  STRING STRING			{ CreateIndicator($1, $2); }
	;

cmds:	  cmd cmds
	| cmd
	;

cmd:	  STRING			{ if (!restart) ExecCommand($1); }
	;

accs:	  acc accs
	| acc
	;

acc:	  STRING VAR VAR		{ CreateAccessory($1, $2, $3); }
	;

%%
int yyerror(const char* error)
{
  QvwmError("%s: %d: %s", filename, line, error);
  QvwmError("Restarting with the minimum configuration...");

  // restart without parsing any configuration files
  RestartQvwm(True);
}
