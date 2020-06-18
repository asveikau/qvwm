/*
 * qvwm.h
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

#ifndef _QVWM_H_
#define _QVWM_H_

#include "main.h"
#include "misc.h"
#include "util.h"
#include "menu.h"
#include "hash.h"
#include "tbutton.h"
#include "list.h"

/*
 * Frame attributes
 */
#define TITLE             (1L << 0)
#define BORDER            (1L << 1)
#define CTRL_MENU         (1L << 2)
#define BUTTON1           (1L << 3)	// minimize button
#define BUTTON2           (1L << 4)	// maximize (restore) button
#define BUTTON3           (1L << 5)	// exit button
#define BORDER_EDGE       (1L << 6)	// 2 dot line inside border
#define THIN_BORDER       (1L << 7)	// 1 dot border surrounding frame
#define SHAPED            (1L << 8)	// shaped window
#define TRANSIENT         (1L << 9)	// transient window
#define STICKY            (1L << 10)	// always display the window
#define NO_FOCUS          (1L << 11)	// has no focus
#define SKIP_FOCUS        (1L << 12)    // skip a window in task switcher
#define FOCUS_ON_CLICK    (1L << 13)    // activate a window only clicking
#define NO_TBUTTON        (1L << 14)    // has no taskbar button
#define ONTOP             (1L << 15)    // always be on top
#define CLOSE_SOON        (1L << 16)    // close a window without displaying
#define INIT_MAXIMIZE     (1L << 17)    // maximized on initializing
#define INIT_MINIMIZE     (1L << 18)    // minimized on intilalizing
#define WM_DELETE_WINDOW  (1L << 19)	// intercept close
#define WM_TAKE_FOCUS     (1L << 20)
#define SMALL_IMG         (1L << 21)	// has particular small pixmap (16x16)
#define LARGE_IMG         (1L << 22)	// has particular large pixmap (32x32)
#define FIXED_POS         (1L << 23)	// locate in fixed position
#define GEOMETRY          (1L << 24)	// has specified geometry

#define DECOR_MASK        0xff		// mask for window decorations

/*
 * Window status
 */
#define MINIMIZE_WINDOW   (1 << 0)
#define MAXIMIZE_WINDOW   (1 << 1)
#define PRESS_FRAME       (1 << 2)

/*
 * Expand directions
 */
#define EXPAND_LEFT    (1 << 0)
#define EXPAND_RIGHT   (1 << 1)
#define EXPAND_UP      (1 << 2)
#define EXPAND_DOWN    (1 << 3)

class FrameButton;
class TaskbarButton;
class Miniature;
class QvImage;
class Tooltip;

/*
 * Qvwm class
 */
class Qvwm {
friend class TaskSwitcher;

private:
  enum { F_LEFT = 0, F_RIGHT, F_TOP, F_BOTTOM };
  enum { F_TLEFT = 0, F_TRIGHT, F_BLEFT, F_BRIGHT };

private:
  Window wOrig;                  // Original window 
  Rect rcOrig;                   // Original window position
  unsigned int bwOrig;           // Original border width 

  Window frame;                  // Frame window (child of root window) 
  Window parent;                 // Parent window of original window 
  Window corner[4];              // Corner window 
  Window side[4];                // Side border window 
  Window edge[4];                // Border edge
  Window title;                  // Title window 
  Window ctrl;                   // Control menu window 
  Rect rc;                       // Frame position
  Rect rcTitle;
  Rect rcParent;
  Rect rcCorner[4];
  Rect rcSide[4];
  Rect rcEdge[4];
  unsigned long flags;           // Frame attributes (with title etc.) 
  unsigned int status;           // Window status (maximize etc.) 
  Bool mapped;                   // Flag if this window is mapped
  char* name;                    // Window name 
  char* shortname;               // Window name fitted to titlebar size
  QvImage* imgSmall;         // Small icon image for control menu
  QvImage* imgLarge;         // Large icon image for task switcher
  InternGeom* geom;              // specified geometry
  XSizeHints hints;
  XWMHints* wmHints;
  XClassHint classHints;
#ifdef USE_XSMP
  unsigned char* wmRole;
  unsigned char* smClientId;
#endif

  Pixmap pixGrad;                // titlebar gradation pixmap
  Pixmap pixActiveGrad;          // active titlebar gradation pixmap

  Tooltip* toolTip;

  QvImage* imgTitle;
  QvImage* imgActiveTitle;
  QvImage* imgSide[4];
  QvImage* imgCorner[4];
  QvImage* imgActiveSide[4];
  QvImage* imgActiveCorner[4];

  Qvwm* qvMain;                  // main window of this transient window
  List<Qvwm> trList;             // transient window list of this window
  Bool trMapped;                 // keep mapping info for transient window

  Window* cmapWins;              // Colormap window list
  int nCmapWins;                 // # of elements of the list

  // Temporary
  Time menuClickTime;            // Previous left click time of menu button 
  Time titleClickTime;           // Previous left click time of titlebar 
  unsigned long flagsBak;        // Frame attributes (backup when maximize)
  Rect rcOrigBak;                // Original window position backup
  Bool isFirstMapped;            // True if window has not been mapped
  
  Dim origWinSize;

  static Bool umReserved;        // Ctrl menu unmap reserved flag

public:
  FrameButton* fButton[3];       // Frame button (minimize etc.) 
  TaskbarButton* tButton;        // Taskbar button 
  Menu* ctrlMenu;                // Control popup menu 
  Miniature* mini;               // Miniture window used by pager

  static Qvwm* focusQvwm;        // focus Qvwm (not always activeQvwm)
  static Qvwm* activeQvwm;       // active Qvwm (raised window)
  static XContext context;
  static Hash<AppAttr>* appHashTable;
  static int initPos;
  static QvImage* imgTitlebar;
  static QvImage* imgActiveTitlebar;
  static QvImage* imgFrame;
  static QvImage* imgActiveFrame;

  static const int BUTTON_WIDTH = 16;      // frame button width
  static const int BUTTON_HEIGHT = 14;     // frame button height
  static const int MOVEMENT = 13;

  static int TITLE_HEIGHT;
  static int BORDER_WIDTH;		// left, right and bottom border
  static int TOP_BORDER;		// top border is special
  static const int TITLE_EDGE = 3;      // the part of border below title

  enum GradStyle {
    Normal, TopToBottom, LeftToRight, CenterToTopBottom, CenterToLeftRight,
    CenterToAll
  };
  
private:
  void CreateFrame(const Rect& rect);
  void CreateParent(const Rect& rect);
  void CreateSides(const Rect rect[]);
  void CreateCorners(const Rect rect[]);  
  void CreateEdges(const Rect rect[]);  

  void GetWindowAttrs(Bool usrPos);
  void ChangeOrigWindow();
  void SetTransient();

  void CalcFramePos(const Rect& rcFrame, Rect* rcParent, Rect* rcTitle,
		    Rect rcCorner[], Rect rcSide[], Rect rcEdge[],
		    Rect* rcCtrl, Rect rcButton[]);

  void MapWindow(Bool mapTransient);
  void UnmapWindow(Bool unmapTransient);
  void StackWindows(Window* wins, Miniature** minis, int *nwindows);
  void AnimateWindow(Bool map);
  void RaiseSoon();

  void FetchWMColormapWindows();
  void GetWindowClassHints();
  void GetWindowSizeHints();
  Point& GetGravityOffset();
  void GetTransientForHint();
  QvImage* GetFixedIcon(int size);

#ifdef USE_XSMP
  void GetSMClientId();
  void GetWMRole();
#endif

public:
  Qvwm(Window w, Bool usrPos);
  Qvwm(Window root);
  ~Qvwm();

  Window GetFrameWin() const { return frame; }
  Window GetWin() const { return wOrig; }
  Bool CheckStatus(unsigned long kind) const { return status & kind; }
  void SetStatus(unsigned long kind) { status |= kind; }
  void ResetStatus(unsigned long kind) { status &= ~kind; }
  Bool CheckFlags(unsigned int kind) const { return flags & kind; }
  void SetFlags(unsigned int kind) { flags |= kind; }
  void ResetFlags(unsigned int kind) { flags &= ~kind; }
  unsigned int GetOrigBW() const { return bwOrig; }
  char* GetNameFromHint();
  void CalcShortName();
  char* GetName() const { return name; }
  Bool CheckMapped() const { return mapped; }
  void ResetMapped() { mapped = False; }
  Bool CheckMenuMapped() const { return ctrlMenu->CheckMapped(); }
  Bool CheckFocus() const { return (this == focusQvwm); }
  QvImage* GetIconImage() const { return tButton->GetImage(); }
  Rect GetRect() const { return rc; }
  void SetRect(Rect& rect) { rc = rect; }
  Rect GetOrigRect() const { return rcOrig; }
  int GetNumCmapWins() const { return nCmapWins; }
  XWMHints* GetWMHints() const { return wmHints; }
  const XClassHint& GetClassHint() const { return classHints; }
  const XSizeHints& GetSizeHints() const { return hints; }
#ifdef USE_XSMP
  Bool SavePropertiesToFile(int idx, FILE* fp);
#endif

  void ChangeFrameFocus();
  void DrawFrame(Window win);
  void RecalcWindow();
  void RedrawWindow(Bool delay = False);
  void CalcWindowPos(const Rect& base);
  static void CreateFramePixmap();

  void MapWindows(Bool mapTransient = False);
  void UnmapWindows(Bool unmapTransient = False);
  void RaiseWindow(Bool soon);
  void LowerWindow();
  void CloseWindow() { SendMessage(_XA_WM_DELETE_WINDOW); }
  void SetShape();

  void UnmapMenu() {
    ASSERT(ctrlMenu);
    ctrlMenu->UnmapMenu();
  }

  void SetFocus();
  void YieldFocus();
  static void SetFocusToActiveWindow();

  void MoveWindow(const Point& ptPress, Bool mouseMove);
  void FixWindowPos(Rect& rcShown);

  void ResizeWindow(unsigned int pos, Bool mouseResize);
  void FixWindowShape(Rect& rcNew, unsigned int pos, const Point& ptBase,
		      const Point& ptNew, const Dim& dNeed);

  void MaximizeWindow(Bool motion = True, Bool mapTransient = True);
  void MinimizeWindow(Bool motion = True, Bool unmapTransient = True);
  void RestoreWindow(Bool motion = True, Bool mapTransient = True);

  void ExpandWindow(int direction =
		    EXPAND_LEFT | EXPAND_RIGHT | EXPAND_UP | EXPAND_DOWN,
		    Bool motion = True);
  void FixExpandedShape(int directions, const RectPt& rpTest,
			const RectPt& rpOrig, RectPt& rpNew);

  void SendMessage(Atom atom);
  void KillClient() {
    XKillClient(display, wOrig);
  }

  void SetProperty(Atom atom);
  void FetchWMProtocols();
  void SetStateProperty(int state);

  void Configure(const XConfigureRequestEvent* cre);
  void Circulate(int place);
  void ConfigureNewRect(const Rect& rcNew, Bool delay = False);
  void SendConfigureEvent();
  void GetBorderAndTitle(int& borderWidth, int& topBorder, int& titleHeight,
			 int& titleEdge);
  Rect GetFixSize(const Rect& rc, int max_width, int max_height, int width_inc,
		  int height_inc, int base_width, int base_height);

  void AdjustPage();

  void Exposure(Window win);
  void Button1Press(Window win, Time clickTime, const Point& ptRoot,
		    unsigned int state);
  void Button1Release();
  void Button2Press(Window win);
  void Button2Release(Window win);
  void Button3Press();
  void Button3Release(Window win, const Point& ptRoot);
  void Button1Motion(Window win, const Point& ptRoot);
  void Button2Motion(Window win, const Point& ptRoot);
  void Enter(Window win, unsigned int state);
  void Leave(Window win);
  void PointerMotion(Window win);

  void ChangeColormap(const XColormapEvent& ev);
  void InstallWindowColormaps();

  static void Initialize();

  // titlebar
  void CreateTitle(const Rect& rect);
  void DrawTitle(Bool focusChange);
  void MotionTitlebar(const Rect& rcSrc, const Rect& rcDest);
  void TitleButton1Press(Time clickTime, const Point& ptRoot);
  static void CreateTitlebarPixmap();

  // contrl button
  void CreateCtrlButton(const Rect& rect);
  void CtrlButton1Press(Time clickTime, const Point& ptRoot,
			unsigned int state);
  void DrawCtrlMenuMark();
};

#endif // _QVWM_H_
