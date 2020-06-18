/*
 * main.cc
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
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/Xresource.h>
#include <X11/Xproto.h>
#ifdef USE_XSMP
#include <X11/SM/SMlib.h>
#endif
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include "misc.h"
#include "qvwm.h"
#include "fbutton.h"
#include "tbutton.h"
#include "taskbar.h"
#include "menu.h"
#include "util.h"
#include "event.h"
#include "parse.h"
#include "main.h"
#include "qvwmrc.h"
#include "icon.h"
#include "switcher.h"
#include "paging.h"
#include "pager.h"
#include "mini.h"
#include "timer.h"
#include "key.h"
#include "debug.h"
#include "ver.h"
#include "focus_mgr.h"
#include "desktop.h"
#include "dialog.h"
#include "indicator.h"
#include "mwm.h"
#include "gnome.h"
#include "info_display.h"
#include "pixmap_image.h"
#include "tooltip.h"
#include "accessory.h"
#include "startmenu.h"
#include "radio_button.h"
#ifdef USE_IMLIB
#include "extra_image.h"
#endif
#include "session.h"
#include "screen_saver.h"
#include "remote_cmd.h"
#include "function.h"
#include "bitmaps/logo16.xpm"
#include "bitmaps/logo32.xpm"
#include "bitmaps/cursor_arrow.xbm"
#include "bitmaps/csmask_arrow.xbm"
#include "bitmaps/cursor_size_h.xbm"
#include "bitmaps/csmask_size_h.xbm"
#include "bitmaps/cursor_size_v.xbm"
#include "bitmaps/csmask_size_v.xbm"
#include "bitmaps/cursor_size_l.xbm"
#include "bitmaps/csmask_size_l.xbm"
#include "bitmaps/cursor_size_r.xbm"
#include "bitmaps/csmask_size_r.xbm"
#include "bitmaps/cursor_busy.xbm"
#include "bitmaps/csmask_busy.xbm"
#include "bitmaps/cursor_move.xbm"
#include "bitmaps/csmask_move.xbm"
#include "bitmaps/cursor_wait.xbm"
#include "bitmaps/csmask_wait.xbm"
#include "bitmaps/cursor_no.xbm"
#include "bitmaps/csmask_no.xbm"

static char*	rcFileName = NULL;     /* Rc file name */
char*		displayName = NULL;    /* Display name */
Display*	display;               /* Display structure */
int		screen;                /* Default screen */
unsigned int	depth;                 /* Default depth */
Colormap	colormap;              /* Default colormap */
Window		root;                  /* Root window */
Qvwm*		rootQvwm;              /* Virtual qvwm for root window */
Rect		rcScreen;              /* Screen size (not include taskbar) */
GC		gc, gcXor, gcDash;
QvImage		*imgLogo, *imgLargeLogo;
Cursor		cursor[9];
XFontSet	fsDefault;
XFontSet	fsTitle, fsTaskbar, fsBoldTaskbar, fsTaskbarClock, fsIcon;
XFontSet	fsCtrlMenu, fsCascadeMenu, fsStartMenu, fsDialog;
XColor		black, white;
XColor		gray, darkGray, grey, darkGrey, blue, lightBlue, royalBlue;
XColor		yellow, lightYellow;
XColor		gray95, darkGray95, lightGray95, grey95, blue95, lightBlue95;
XColor		green95, yellow95;
Bool		synch = False;
Bool		shapeSupport;
char**          qvArgv;
Bool		start = False;
Bool restart = False;  // ignore [Startup] section
Bool noParse = False;  // not parse ~/.qvwmrc and system.qvwmrc
Event event;
FocusMgr focusMgr;
Desktop desktop;
Pixmap pixWallPaper;
int failCount = 0;
#ifdef USE_XSMP
char* previousId = NULL; // The client Id from the previous session
Session* session = NULL;
#endif
Bool enableTaskbar = True, enablePager = True;

/*
 * Internal atoms.
 */
Atom _XA_WM_CHANGE_STATE;
Atom _XA_WM_STATE;
Atom _XA_WM_COLORMAP_WINDOWS;
Atom _XA_WM_PROTOCOLS;
Atom _XA_WM_TAKE_FOCUS;
Atom _XA_WM_DELETE_WINDOW;
Atom _XA_WM_DESKTOP;
Atom _XA_WM_ICON_SIZE;
#ifdef USE_XSMP
Atom _XA_WM_WINDOW_ROLE;
Atom _XA_WM_CLIENT_LEADER;
Atom _XA_SM_CLIENT_ID;
#endif //USE_XSMP

Taskbar*	taskBar      = NULL;
StartMenu*	startMenu    = NULL;
TaskSwitcher*	taskSwitcher = NULL;
Paging*		paging;
Pager*		pager        = NULL;
Menu*		ctrlMenu;
ExitDialog*	exitDlg      = NULL;
ConfirmDialog*	confirmDlg   = NULL;
Timer*		timer;
InfoDisplay*    infoDisp;
#ifdef USE_SS
ScreenSaver*    scrSaver = NULL;
#endif
#ifdef ALLOW_RMTCMD
RemoteCommand*  remoteCmd = NULL;
#endif

/*
 * Local function prototype declaration.
 */
void AnalizeOptions(int argc, char** argv);
void Usage();
void DisplayVersion();
void DisplayConfig();
void InitQvwm();
void SetSignal();
void SetGC();
void SetColor();
void CreateLogoMarkPixmap();
void CreateCursor();
void CreateInternAtom();
int CatchRedirectError(Display* display, XErrorEvent *ev);
int CatchFatalError(Display *display);
int QvwmErrorHandler(Display* display, XErrorEvent* ev);
void TrapSignal(int sig);
void TrapChild(int sig);
void TrapAlarm(int sig);
void TrapHup(int sig);

int main(int argc, char** argv)
{
  qvArgv = argv;

  AnalizeOptions(argc, argv);

  /*
   * Open the display.
   */
  if ((display = XOpenDisplay(displayName)) == NULL) {
    QvwmError("Can't open display %s", XDisplayName(displayName));
    exit(1);
  }

  if (displayName)
    SetDisplayEnv(displayName);

  /*
   * Synchronize X events.
   */
  if (synch) {
    XSynchronize(display, True);
    QvwmError("Synchronize mode");
  }

  /*
   * Set system defaults.
   */
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);
  depth = DefaultDepth(display, screen);
  colormap = DefaultColormap(display, screen);
  rcScreen = Rect(0, 0,
		  DisplayWidth(display, screen),
		  DisplayHeight(display, screen));

  /*
   * Select inputs for window manager.
   */
  XSetErrorHandler(CatchRedirectError);
  XSetIOErrorHandler((XIOErrorHandler)CatchFatalError);
  XSelectInput(display, root, LeaveWindowMask | EnterWindowMask |
	       PropertyChangeMask | SubstructureRedirectMask | KeyPressMask |
	       ButtonPressMask | ButtonReleaseMask | SubstructureNotifyMask);
  XSync(display, 0);
  XSetErrorHandler(NULL);
  
#ifdef USE_SHAPE
  int shapeErrorBase;

  shapeSupport = XShapeQueryExtension(display, &event.shapeEventBase,
				      &shapeErrorBase);
#endif

  SetSignal();

  /*
   * Cleanup leftovers from previous session (after restart)
   */
  if (restart)
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;

  /*
   * Set resources for qvwm.
   */
  CreateInternAtom();
  ShortCutKey::SetModifier();
  Indicator::Initialize();
  QvFunction::initialize();

  SetColor();
  SetGC();

  // used in ExecCommand for indicators
  timer = new Timer();

  scKey = new SCKeyTable();
  menuKey = new SCKeyTable();

  desktop.CreateTopWindow();

  ParseQvwmrc(rcFileName);

  if (XSupportsLocale() == False)
    QvwmError("X does not support the locale");

  InitQvwm();
  start = True;

  event.MainLoop();
}

/*
 * AnalizeOptions --
 *   Analize command line options.
 */
void AnalizeOptions(int argc, char** argv)
{
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {  // long-style options
	if (strcmp(&argv[i][2], "display") == 0)
	  displayName = argv[++i];
	else if (strcmp(&argv[i][2], "rcfile") == 0)
	  rcFileName = argv[++i];
	else if (strcmp(&argv[i][2], "noparse") == 0)
	  noParse = True;
	else if (strcmp(&argv[i][2], "restart") == 0)
	  restart = True;
#ifdef USE_XSMP
	else if (strcmp(&argv[i][2], "clientid") == 0)
	  previousId = argv[++i];
#endif
	else if (strcmp(&argv[i][2], "synch") == 0)
	  synch = True;
	else if (strcmp(&argv[i][2], "version") == 0)
	  DisplayVersion();
	else if (strcmp(&argv[i][2], "config") == 0)
	  DisplayConfig();
	else if (strcmp(&argv[i][2], "help") == 0)
	  Usage();
	else {
	  printf("invalid option: %s\n", argv[i]);
	  Usage();
	}
      }
      else {
	if (strcmp(&argv[i][1], "d") == 0 ||
	    strcmp(&argv[i][1], "display") == 0)
	  displayName = argv[++i];
	else if (strcmp(&argv[i][1], "f") == 0)
	  rcFileName = argv[++i];
	else if (strcmp(&argv[i][1], "n") == 0)
	  noParse = True;
	else if (strcmp(&argv[i][1], "r") == 0)
	  restart = True;
#ifdef USE_XSMP
	else if (strcmp(&argv[i][1], "id") == 0 ||
		 strcmp(&argv[i][1], "clientId") == 0)
	  previousId = argv[++i];
#endif
	else if (strcmp(&argv[i][1], "s") == 0 ||
		 strcmp(&argv[i][1], "synch") == 0)
	  synch = True;
	else if (strcmp(&argv[i][1], "c") == 0)
	  failCount = atoi(argv[++i]);
	else if (strcmp(&argv[i][1], "v") == 0)
	  DisplayVersion();
	else if (strcmp(&argv[i][1], "cfg") == 0)
	  DisplayConfig();
	else
	  Usage();
      }
    }
    else
      Usage();
  }
}

void Usage()
{
  printf("Usage:\n");
  printf("  qvwm [-cfg] [-d display_name] [-f rcfile_name] [-id client_id] [-n] [-r]\n");
  printf("       [-s] [-v] [long-style options]\n");
  printf("Long-style options:\n");
  printf("  --clientid client_id    specify a client ID for session manager\n");
  printf("  --config                display compilation options\n");
  printf("  --display display_name  specify a display name\n");
  printf("  --help                  display this help\n");
  printf("  --noparse               parse no configuration files\n");
  printf("  --rcfile rcfile_name    specify a qvwmrc file\n");
  printf("  --restart               start without executing [Startup] section\n");
  printf("  --synch                 run qvwm in synchronized mode\n");
  printf("  --version               display version information\n");
  exit(0);
}

void DisplayVersion()
{
  printf("qvwm version %s (%s)\n", Version, Codename);
  printf("Copyright (C) 1995-2001 Kenichi Kourai.\n");
  printf("E-mail: kourai@qvwm.org\n");
  printf("URL: http://www.qvwm.org/\n");
  exit(0);
}

void DisplayConfig()
{
  printf("Config:\n");
  printf("  qvwm directory: %s\n", QVWMDIR);
  printf("  image directory: %s\n", IMGDIR);
  printf("  sound directory: %s\n", SNDDIR);
  printf("  options:");
#ifdef DEBUG
  printf(" [DEBUG]");
#endif
#ifdef USE_ANIM_IMAGE
  printf(" [ANIM_IMAGE]");
#endif
#ifdef USE_IMLIB
  printf(" [IMLIB]");
#endif
#ifdef USE_SHAPE
  printf(" [SHAPE]");
#endif
#ifdef USE_XSMP
  printf(" [XSMP]");
#endif
#ifdef USE_ALSA
  printf(" [ALSA]");
#endif
#ifdef USE_ESD
  printf(" [ESD]");
#endif
#ifdef USE_SS
  printf(" [SS]");
#ifdef USE_SSEXT
  printf(" [SSEXT]");
#endif
#endif /* USE_SS */
#ifdef ALLOW_RMTCMD
  printf(" [RMTCMD]");
#endif
  printf("\n");
  exit(0);
}

/*
 * Initialize qvwm.
 */
void InitQvwm()
{
  XSetErrorHandler(QvwmErrorHandler);

#ifdef USE_IMLIB
  ExtraImage::Initialize();
#endif
  Qvwm::Initialize();
  TaskbarButton::Initialize();
  Taskbar::Initialize();
  Menu::Initialize();
  Button::Initialize();
  Dialog::Initialize();
  TaskSwitcher::Initialize();
  Miniature::Initialize();
  Icon::Initialize();
  Tooltip::Initialize();
  FrameButton::Initialize();
  StartMenu::Initialize();
  RadioButton::Initialize();

  CreateLogoMarkPixmap();
  CreateCursor();
  if (GradTitlebar)
    CreateGradPattern();

  rootQvwm = new Qvwm(root);

  ctrlMenu = new Menu(CtrlMenuItem, fsCtrlMenu, NULL, NULL);

  Icon::ctrlMenu = new IconMenu(IconMenuItem, fsCtrlMenu, NULL, NULL);

  desktop.SetWallPaper();

#ifdef USE_SS
  // this must be before PlaySound() because it causes SIGCHILD
  scrSaver = new ScreenSaver(ScreenSaverProg, ScreenSaverDelay);
#endif

  if (restart)
    PlaySound(SystemRestartSound);
  else
    PlaySound(SystemStartSound);

  paging = new Paging(TopLeftPage, PagingSize);

  if (UsePager) {
    pager = new Pager(PagerGeometry);
    pager->MapPager();
  }
  
  infoDisp = new InfoDisplay();

#ifdef ALLOW_RMTCMD
  if (AllowRemoteCmd)
    remoteCmd = new RemoteCommand();
#endif

  Mwm::Init();
  Gnome::Init();
  Gnome::SetGnomeCompliant(desktop.GetTopWindow());

  taskBar->MoveTaskbar(TaskbarPosition);

#ifdef USE_XSMP
  session = new Session(previousId);
  session->LoadSessionFile();
#endif

  if (UseTaskbar)
    taskBar->MapTaskbar();

  desktop.CaptureAllWindows();

  desktop.CreateIcons();
  desktop.RedrawAllIcons();

  desktop.CreateAccessories();

  /*
   * Key bind for menu.
   */
  for (int i = 0; i < MenuKeyNum; i++)
    menuKey->AddSCKey(scMenuKey[i].sym, scMenuKey[i].mod, scMenuKey[i].func);
}

/*
 * SetSignal --
 *   Set signal handlers.
 */
void SetSignal()
{
  signal(SIGINT, TrapSignal);
  signal(SIGQUIT, TrapSignal);
  signal(SIGTERM, TrapSignal);
  signal(SIGSEGV, TrapSignal);
  signal(SIGBUS, TrapSignal);
  signal(SIGFPE, TrapSignal);
//#ifndef USE_XSMP
  signal(SIGCHLD, TrapChild);
//#endif
  signal(SIGHUP, TrapHup);
}

/*
 * SetColor --
 *   Allocate mininum colors for qvwm.
 */
void SetColor()
{
  if (CreateColor(0x0000, 0x0000, 0x0000, &black) == 0) {
    QvwmError("cannot allocate color: black");
    exit(1);
  }
  if (CreateColor(0xffff, 0xffff, 0xffff, &white) == 0) {
    QvwmError("cannot allocate color: white");
    exit(1);
  }

  CreateColor(0xd4d4, 0xd0d0, 0xc8c8, &gray, &white, "gray");
  CreateColor(0x8080, 0x8080, 0x8080, &darkGray, &black, "dark gray");
  CreateColor(0xc0c0, 0xc0c0, 0xc0c0, &grey, &gray, "grey");
  CreateColor(0x4040, 0x4040, 0x4040, &darkGrey, &black, "dark grey");
  CreateColor(0x0a0a, 0x2424, 0x6a6a, &blue, &black, "blue");
  CreateColor(0xa6a6, 0xcaca, 0xf0f0, &lightBlue, &blue, "light blue");
  CreateColor(0x3a3a, 0x6e6e, 0xa5a5, &royalBlue, &blue, "royal blue");
  CreateColor(0xf5f5, 0xdbdb, 0x9595, &yellow, &white, "yellow");
  CreateColor(0xffff, 0xffff, 0xe1e1, &lightYellow, &white, "lightYellow");

  // backward compatibility
  gray95.pixel = 0;
  darkGray95.pixel = 0;
  lightGray95.pixel = 0;
  grey95.pixel = 0;
  blue95.pixel = 0;
  lightBlue95.pixel = 0;
  green95.pixel = 0;
  yellow95.pixel = 0;

  IconForeColor = black;
  IconBackColor = white;
  IconStringColor = white;
  IconStringActiveColor = white;
  MiniatureColor = black;
  MiniatureActiveColor = white;
  TitlebarColor = darkGray;
  TitlebarColor2 = grey;
  TitlebarActiveColor = blue;
  TitlebarActiveColor2 = lightBlue;
  TitleStringColor = gray;
  TitleStringActiveColor = white;
  MenuColor = gray;
  MenuActiveColor = blue;
  MenuStringColor = black;
  MenuStringActiveColor = white;
  DialogColor = gray;
  DialogStringColor = black;
  SwitcherColor = gray;
  SwitcherActiveColor = blue;
  SwitcherStringColor = black;
  FrameColor = gray;
  FrameActiveColor = gray;
  PagerColor = gray;
  PagerActiveColor = darkGray;
  ButtonColor = gray;
  ButtonActiveColor = gray;
  ButtonStringColor = black;
  ButtonStringActiveColor = black;
  TaskbarColor = gray;
  ClockStringColor = black;
  DesktopColor = royalBlue;
  DesktopActiveColor = blue;
  StartMenuLogoColor = black;
  CursorColor = white;
  TooltipColor = lightYellow;
  TooltipStringColor = black;
}

/*
 * SetGC --
 *   Set GC for normal, xor and dash.
 */
void SetGC()
{
  XGCValues gcv;
  unsigned long gcm;
  char dash[] = {1, 1};
  
  gc = XCreateGC(display, root, 0, 0);

  gcv.function = GXxor;
  gcv.line_width = 4;
  gcv.foreground = darkGray.pixel;
  gcv.subwindow_mode = IncludeInferiors;
  gcm = GCFunction | GCLineWidth | GCForeground | GCSubwindowMode;
  gcXor = XCreateGC(display, root, gcm, &gcv);

  gcDash = XCreateGC(display, root, 0, 0);
  XSetDashes(display, gcDash, 0, dash, 2);
  XSetLineAttributes(display, gcDash, 1, LineOnOffDash, CapButt, JoinMiter);
}

/*
 * CreateCursor --
 *   Create cursors.
 */
void CreateCursor()
{
  Pixmap pixCursor, maskCursor;
  char* curbits[9] = { cursor_arrow_bits, cursor_size_h_bits,
		       cursor_size_v_bits, cursor_size_l_bits,
		       cursor_size_r_bits, cursor_busy_bits,
		       cursor_move_bits, cursor_wait_bits,
		       cursor_no_bits };
  char* maskbits[9] = { csmask_arrow_bits, csmask_size_h_bits,
			csmask_size_v_bits, csmask_size_l_bits,
			csmask_size_r_bits, csmask_busy_bits,
			csmask_move_bits, csmask_wait_bits,
			csmask_no_bits };
  Dim size[9] = { Dim(cursor_arrow_width, cursor_arrow_height),
		  Dim(cursor_size_h_width, cursor_size_h_height),
		  Dim(cursor_size_v_width, cursor_size_v_height),
		  Dim(cursor_size_l_width, cursor_size_l_height),
		  Dim(cursor_size_r_width, cursor_size_r_height),
		  Dim(cursor_busy_width, cursor_busy_height),
		  Dim(cursor_move_width, cursor_move_height),
		  Dim(cursor_wait_width, cursor_wait_height),
		  Dim(cursor_no_width, cursor_no_height) };
  Point spot[9] = { Point(10, 6), Point(15, 15), Point(14, 15),
		    Point(14, 15), Point(14, 14), Point(15, 15),
		    Point(16, 16), Point(4, 5), Point(17, 17) };

  for (int i = 0; i < 9; i++) {
    pixCursor = XCreateBitmapFromData(display, root, curbits[i],
				      size[i].width, size[i].height);
    maskCursor = XCreateBitmapFromData(display, root, maskbits[i],
				       size[i].width, size[i].height);
    cursor[i] = XCreatePixmapCursor(display, pixCursor, maskCursor,
				    &black, &CursorColor,
				    spot[i].x, spot[i].y);

    XFreePixmap(display, pixCursor);
    XFreePixmap(display, maskCursor);
  }

  XDefineCursor(display, root, cursor[SYS]);
}

/*
 * CreateInternAtom --
 *   Create internal atoms of window manager.
 */
void CreateInternAtom()
{
  _XA_WM_CHANGE_STATE = XInternAtom(display, "WM_CHANGE_STATE", False);
  _XA_WM_STATE = XInternAtom(display, "WM_STATE", False);
  _XA_WM_COLORMAP_WINDOWS = XInternAtom(display, "WM_COLORMAP_WINDOWS", False);
  _XA_WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", False);
  _XA_WM_TAKE_FOCUS = XInternAtom(display, "WM_TAKE_FOCUS", False);
  _XA_WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
  _XA_WM_DESKTOP = XInternAtom(display, "WM_DESKTOP", False);
  _XA_WM_ICON_SIZE = XInternAtom(display, "WM_ICON_SIZE", False);
#ifdef USE_XSMP
  _XA_WM_WINDOW_ROLE = XInternAtom(display, "WM_WINDOW_ROLE", False);
  _XA_WM_CLIENT_LEADER = XInternAtom(display, "WM_CLIENT_LEADER", False);
  _XA_SM_CLIENT_ID = XInternAtom(display, "SM_CLIENT_ID", False);
#endif
}

/*
 * CreateLogoMarkPixmap --
 *   Create pixmaps for logo mark.
 */
void CreateLogoMarkPixmap()
{
  if (DefaultIcon == NULL)
    imgLogo = new PixmapImage(logo16);
  else {
    imgLogo = CreateImageFromFile(DefaultIcon, timer);
    if (imgLogo == NULL)
      imgLogo = new PixmapImage(logo16);
  }

  if (DefaultLargeIcon == NULL)
    imgLargeLogo = new PixmapImage(logo32);
  else {
    imgLargeLogo = CreateImageFromFile(DefaultLargeIcon,
				       TaskSwitcher::swTimer);
    if (imgLargeLogo == NULL)
      imgLargeLogo = new PixmapImage(logo32);
  }
}  

/*
 * CatchRedirectError --
 *   Jump here when another window manager is running.
 */
int CatchRedirectError(Display* display, XErrorEvent *ev)
{
  QvwmError("Another window manager is running");
  exit(1);
}

/*
 * CatchFatalError --
 *   Jump here when an fatal I/O error happens in X server.
 */
int CatchFatalError(Display *display)
{
  XCloseDisplay(display);
  exit(1);
}

/*
 * QvwmErrorHandler --
 *   Jump here when an error happens in X server.
 */
int QvwmErrorHandler(Display *display, XErrorEvent* ev)
{
#ifndef DEBUG
  if (ev->error_code == BadWindow ||
      ev->error_code == BadDrawable || 
      ev->request_code == X_GetGeometry ||
      ev->request_code == X_GrabButton ||
      ev->request_code == X_SetInputFocus ||
      ev->request_code == X_ChangeWindowAttributes ||
      ev->request_code == X_InstallColormap)
    return 0;
#endif

  char msg[128], number[32], request[128];

  XGetErrorText(display, ev->error_code, msg, 128);
  sprintf(number, "%d", ev->request_code);
  XGetErrorDatabaseText(display, "XRequest", number, "", request, 128);
  if (*request == '\0')
    sprintf(request, "Unknown_Code_%d", ev->request_code);
  QvwmError("%s (0x%lx): %s", request, ev->resourceid, msg);

  return 0;
}

#define DEBUGGER "gdb"

/*
 * TrapSignal --
 *   General signal handler.
 */
void TrapSignal(int sig)
{
  if (sig == SIGSEGV || sig == SIGBUS) {
    if (sig == SIGSEGV) {
      signal(SIGSEGV, SIG_DFL);
      QvwmError("Segmentation fault");
    }
    else {
      signal(SIGBUS, SIG_DFL);
      QvwmError("Bus error");
    }
    
    if (UseDebugger) {
      Debug debug;
      QvwmError("Starting %s...", DEBUGGER);
      debug.TraceStack(DEBUGGER, qvArgv[0]);
    }

    // terminate accessory processes
    //   This is done in FinishQvwm, but it is not called from here...
    List<Accessory>::Iterator i(&desktop.GetAccList());
    for (Accessory* acc = i.GetHead(); acc; acc = i.Remove())
      delete acc;

    if (RestartOnFailure) {
      timer->ResetAllTimeouts();

      XSetInputFocus(display, root, RevertToNone, CurrentTime);
      if (start)
	paging->PagingProc(Point(0, 0));

      // Exit qvwm if failure coutinues
      if (failCount < MAX_FAILURE_COUNT) {
	QvwmError("Automatically restarting...");
	RestartQvwm(False, failCount + 1, False);
      }
      else {
	QvwmError("Failure count reaches maximum.  Start qvwm again.");
	XCloseDisplay(display);
	exit(1);
      }
    }
    else {
      XSetInputFocus(display, root, RevertToNone, CurrentTime);
      if (start)
	paging->PagingProc(Point(0, 0));
      XCloseDisplay(display);

      abort();
    }
  }

  FinishQvwm();
}

/*
 * TrapChild --
 *   Signal handler for child termination.
 */
void TrapChild(int sig)
{
  pid_t pid;

  while (1) {
    pid = waitpid(-1, NULL, WNOHANG);
    if (pid <= 0)
      break;
#ifdef USE_SS
    if (scrSaver && scrSaver->NotifyDeadPid(pid))
      continue;
#endif
    if (Indicator::NotifyDeadPid(pid))
      continue;
  }

  signal(SIGCHLD, TrapChild);
}

void TrapHup(int sig)
{
  RestartQvwm();
}

#ifdef HAVE_VPRINTF
/*
 * QvwmError --
 *   Display an error in qvwm.
 */
void QvwmError(const char* fmt, ...)
{
  va_list args;
  char buf[256];
  
  va_start(args, fmt);
  
  sprintf(buf, "qvwm: %s\n", fmt);
  vfprintf(stderr, buf, args);

  va_end(args);
}
#endif // HAVE_VPRINTF
  
void FinishQvwm()
{
#ifdef USE_XSMP
  if (session->GetStatus()) {
    if (!session->RequestQuit())
      desktop.FinishQvwm(False);
  }
  else
#endif // USE_XSMP
    desktop.FinishQvwm(False);
}

void RestartQvwm(Bool minimumRestart, int count, Bool cleanup)
{
  char *p, *argv[20], buf[8];
  int i, j;

  if (cleanup)
    desktop.FinishQvwm(True);

  // Fvwm family sets "fvwm" to argument 0 of qvwm.
  // I think that it is a bug of fvwm...
  for (p = qvArgv[0]; *p; p++)
    ;
  while (--p >= qvArgv[0] && *p != '/')
    ;

  // Allows qvwm* (e.g. "qvwm", "qvwm-1.0b11a")
  // If you want another name (e.g. "Qvwm"), use "EXEC ..." instead.
  if (strncmp(++p, "qvwm", 4) != 0) {
    QvwmError("qvwm's 1st argument is not 'qvwm*' but '%s'", p);
    QvwmError("restart 'qvwm'");
    qvArgv[0] = "qvwm";
  }

  for (i = 0, j = 0; qvArgv[i] != NULL && j < 16; i++) {
    char *arg = qvArgv[i];
    // read through a restart option
    if ( (strcmp(arg, "-r") == 0)
         || (strcmp(arg, "-n") == 0) )
      continue;
    // read through a failure count option
    if (strcmp(arg, "-c") == 0) {
      i++;
      continue;
    }

    argv[j++] = arg;
  }

  if (minimumRestart)
    argv[j++] = "-n";
  else {
    argv[j++] = "-r";

    // add an option if from FailureOnRestart
    if (count > 0) {
      argv[j++] = "-c";
      sprintf(buf, "%d", count);
      argv[j++] = buf;
    }
  }

  argv[j] = NULL;
  
  execvp(argv[0], argv);
}
