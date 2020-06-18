/*
 * lexsrc.ll
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

%option noyywrap

/*
 * the "incl" state is used for picking up the name of an include file
 */
%x incl	

%{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "parse.h"
#include "yaccsrc.h"

/* global state */
int line;
char filename[256];

#define MAX_INCLUDE_DEPTH 8
#define MAX_FILE_LEN 255

/* saved state stack by including */
struct bufstate {
  char filename[MAX_FILE_LEN + 1];
  int line;
  YY_BUFFER_STATE buffer;
} incl_stack[MAX_INCLUDE_DEPTH];
int incl_stack_ptr = 0;
%}

%%

[ \t]+		;
\n		{ line++; }

;.*		;  /* comment */

include		BEGIN(incl);

\+[ \t]		{ return PLUS; }
\+\n		{ line++;  return PLUS; }
\-[ \t]		{ return MINUS; }
\-\n		{ line++;  return MINUS; }

\"([^\\"]*\\\")*[^"]*\" {
		  int len = strlen(yytext) - 2;
		  yylval.str = new char[len + 1];
		  int j = 0;
		  for (int i = 1; i < len + 1; i++, j++) {
		    if (yytext[i] == '\\' && yytext[i + 1] == '\"')
		      i++;
		    yylval.str[j] = yytext[i];
		  }
		  yylval.str[j] = '\0';
		  return STRING; }
\[[^]]*\]	{ int len = strlen(yytext) - 2;
		  yylval.str = new char[len + 1];
		  strncpy(yylval.str, &yytext[1], len);
		  yylval.str[len] = '\0';
		  if (strcmp(yylval.str, "Variables") == 0)
		    return VARIABLE;
		  else if (strcmp(yylval.str, "Shortcuts") == 0)
		    return SHORTCUT;
		  else if (strcmp(yylval.str, "Applications") == 0)
		    return APP;
		  else if (strcmp(yylval.str, "ShortCutKeys") == 0)
		    return KEY;
		  else if (strcmp(yylval.str, "Indicators") == 0)
		    return IND;
		  else if (strcmp(yylval.str, "ExitDialog") == 0)
		    return EXITDLG;
		  else if (strcmp(yylval.str, "Startup") == 0)
		    return STARTUP;
		  else if (strcmp(yylval.str, "Accessories") == 0)
		    return ACC;
		  else
		    return MENU; }
[^=,| \t\n]+	{ yylval.str = new char[strlen(yytext) + 1];
		  strcpy(yylval.str, yytext);
		  if (IsFunction(yylval.str))
		    return FUNC;
		  else
		    return VAR; }
	
<incl>[ \t]*	;  /* eat the whitespace */
<incl>[^ \t\n]+	{ /* got the include file name */
                  if (incl_stack_ptr >= MAX_INCLUDE_DEPTH)
		    QvwmError("Includes nested too deeply, ignoring file \"%s\"", yytext);
		  else {
		    char* name = ExtractPathName(yytext);

		    FILE* fd = fopen(name, "r");

		    if (fd == NULL)
		      QvwmError("Cannot read from include file \"%s\".", name);
		    else {
		      yyin = fd;

		      /* save current state */
		      strcpy(incl_stack[incl_stack_ptr].filename, filename);
		      incl_stack[incl_stack_ptr].line = line;
		      incl_stack[incl_stack_ptr].buffer = YY_CURRENT_BUFFER;
		      incl_stack_ptr++;

		      /* setup new state */
		      strncpy(filename, yytext, MAX_FILE_LEN);
		      filename[MAX_FILE_LEN] = '\0';
		      line = 1;
		      yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
		    }

		    delete [] name;
		  }
		  BEGIN(INITIAL);		  
		}
     
<<EOF>>		{ /* get back to from where we were included */
                  if (--incl_stack_ptr < 0)
		    yyterminate();
		  else {
		    fclose(yyin);
		    yy_delete_buffer(YY_CURRENT_BUFFER);

		    /* restore previous state */
		    strcpy(filename, incl_stack[incl_stack_ptr].filename);
		    line = incl_stack[incl_stack_ptr].line;
		    yy_switch_to_buffer(incl_stack[incl_stack_ptr].buffer);
		  }
                }


.		{ return yytext[0]; }

%%
