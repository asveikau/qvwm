/*
 * main.h
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

#ifndef _MAIN_H_
#define _MAIN_H_

#include "message.h"

class Qvwm;
class Rect;
class Event;
class FocusMgr;
class Desktop;
class QvImage;

extern char*		displayName;
extern Display*		display;
extern int		screen;
extern unsigned int	depth;
extern Colormap		colormap;
extern Window		root;
extern Qvwm*		rootQvwm;
extern Rect		rcScreen;
extern GC		gc, gcXor, gcTile, gcDash;
extern QvImage		*imgLogo, *imgLargeLogo;
extern Cursor		cursor[9];
extern XFontSet		fsDefault;
extern XFontSet		fsTitle, fsTaskbar, fsBoldTaskbar, fsTaskbarClock, fsIcon;
extern XFontSet		fsCtrlMenu, fsCascadeMenu, fsStartMenu, fsDialog;
extern XColor		black, white;
extern XColor		gray, darkGray, grey, darkGrey, blue, lightBlue;
extern XColor		royalBlue, yellow, lightYellow;
extern XColor		gray95, darkGray95, lightGray95, grey95, blue95;
extern XColor		lightBlue95, green95, yellow95;
extern Bool		shapeSupport;
extern char**           qvArgv;
extern Bool		start;
extern Bool restart;
extern Bool noParse;
extern Event event;
extern FocusMgr focusMgr;
extern Desktop desktop;

extern Bool enableTaskbar, enablePager;

extern unsigned long* gradPattern;
extern unsigned long* gradActivePattern;

#ifdef USE_XSMP
class Session;
extern Session* session;
#endif

#ifdef USE_SS
class ScreenSaver;
extern ScreenSaver* scrSaver;
#endif

#ifdef ALLOW_RMTCMD
class RemoteCommand;
extern RemoteCommand* remoteCmd;
#endif

extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_DELETE_WINDOW;
extern Atom _XA_WM_DESKTOP;
extern Atom _XA_WM_ICON_SIZE;
#ifdef USE_XSMP
extern Atom _XA_WM_CLIENT_LEADER;
extern Atom _XA_SM_CLIENT_ID;
extern Atom _XA_WM_WINDOW_ROLE;
#endif

class Taskbar;
class StartMenu;
class TaskSwitcher;
class Paging;
class Pager;
class Menu;
class ExitDialog;
class ConfirmDialog;
class Timer;
class InfoDisplay;

extern Taskbar*		taskBar;
extern StartMenu*	startMenu;
extern TaskSwitcher*	taskSwitcher;
extern Paging*		paging;
extern Pager*		pager;
extern Menu*		ctrlMenu;
extern ExitDialog*	exitDlg;
extern ConfirmDialog*	confirmDlg;
extern Timer*           timer;
extern InfoDisplay*     infoDisp;

extern void FinishQvwm();
extern void RestartQvwm(Bool minimumRestart = False, int count = 0,
			Bool cleanup = True);

extern Pixmap CreateGradPixmap(unsigned long* pat, int width, Window win);
extern void CreateGradPattern();

#endif // _MAIN_H_
