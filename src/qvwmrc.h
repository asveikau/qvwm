/*
 * qvwmrc.h
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

#ifndef _QVWMRC_H_
#define _QVWMRC_H_

#include "misc.h"
#include "util.h"
#include "taskbar.h"
#include "desktop.h"
#include "qvwm.h"

const int HashTableSize = 50;
extern const int VarTableSize;
extern const int FsNum;
extern const int AttrNum;
extern const int KeyModNum;
extern const int MenuKeyNum;

class VariableTable;
class SCKeyTable;
struct SCKeyEntry;

extern int DoubleClickTime;
extern int DoubleClickRange;
extern Bool TitlebarMotion;
extern int TitlebarMotionSpeed;
extern int MenuDelayTime;
extern int MenuDelayTime2;
extern int PagingResistance;
extern int PagingMovement;
extern int PagingBeltSize;
extern Point TopLeftPage;
extern Dim PagingSize;
extern int PagingSpeed;
extern InternGeom PagerGeometry;
extern int EdgeResistance;
extern int SnappingMove;
extern int SnappingEdges;
extern Bool SmartPlacement;
extern Bool NoResizeOverTaskbar;
extern Bool UseTaskbar;
extern Taskbar::TaskbarPos TaskbarPosition;
extern unsigned int TaskbarRows;
extern Bool OnTopTaskbar;
extern int AutoRaiseDelay;
extern const char* WallPaper;
extern const char* LocaleName;
extern Bool UseBoldFont;
extern Bool UseExitDialog;
extern Bool UseConfirmDialog;
extern Bool UsePager;
extern Bool OpaqueMove;
extern Bool OpaqueResize;
extern Bool FullOpaque;
extern Bool ClickToFocus;
extern Bool FocusOnMap;
extern Bool ClickingRaises;
extern Bool NoDesktopFocus;
extern Bool AutoRaise;
extern Bool UseClock;
extern const char* ClockLocaleName;
extern const char* ImagePath;
extern const char* SoundPath;
extern Bool TaskbarAutoHide;
extern Bool RestoreMinimize;
extern Bool RestartOnFailure;
extern Bool GradMenuMap;
extern int GradMenuMapSpeed;
extern Bool GradTaskbarMotion;
extern int GradTaskbarMotionSpeed;
extern Bool GradTitlebar;
extern int GradTitlebarColors;
extern Bool OnTopPager;
extern double ShiftMoveRatio;
extern double CtrlMoveRatio;
extern int HourGlassTime;
extern int UseInfoDisplay;
extern Qvwm::GradStyle GradWindowMapStyle;
extern int GradWindowMapSpeed;
extern Bool UseDebugger;
extern Bool ImageAnimation;
extern int TooltipDelayTime;
extern int TooltipMotionSpeed;
extern int TooltipDisplayTime;
extern unsigned int NoFocusChangeMask;
extern unsigned int NoSnappingMask;
extern int TaskbarShowDelay;
extern int TaskbarHideDelay;
extern Bool LockDragState;
extern const char* ScreenSaverProg;
extern int ScreenSaverDelay;
extern Bool TaskbarButtonInScr;
extern Bool EnableSound;
extern Bool EnableAlsa;
extern Bool EnableEsd;
extern Bool AllowRemoteCmd;
extern Bool DisableDesktopChange;
extern Bool DisableTaskbarDragging;

extern const char* StartButtonTitle;
extern const char* StartButtonMessage;
extern const char* MinimizeButtonMessage;
extern const char* MaximizeButtonMessage;
extern const char* CloseButtonMessage;
extern const char* RestoreButtonMessage;
extern const char* ClockFormat;
extern const char* ClockMessageFormat;

extern int IconSize;
extern int FrameTitleHeight;
extern int FrameBorderWidth;
extern int TaskbarButtonHeight;
extern int IndicatorSize;
extern int IconVerticalSpacing;
extern int IconHorizontalSpacing;
extern int MenuItemMinimalHeight;

extern const char* TitlebarImage;
extern const char* TitlebarActiveImage;
extern const char* FrameImage;
extern const char* FrameActiveImage;
extern const char* TaskbarImage;
extern const char* MenuImage;
extern const char* MenuActiveImage;
extern const char* PagerImage;
extern const char* DialogImage;
extern const char* SwitcherImage;
extern const char* StartMenuLogoImage;

extern const char* SystemStartSound;
extern const char* SystemExitSound;
extern const char* SystemRestartSound;
extern const char* MaximizeSound;
extern const char* MinimizeSound;
extern const char* RestoreUpSound;
extern const char* RestoreDownSound;
extern const char* ExpandSound;
extern const char* MenuPopupSound;
extern const char* MenuCommandSound;
extern const char* OpenSound;
extern const char* CloseSound;
extern const char* PagerSound;
extern const char* PagingSound;

extern const char* OpeningSound;
extern const char* EndingSound;
extern const char* RestartSound;
extern const char* RestoreSound;

extern const char* DefaultIcon;
extern const char* DefaultMenuItemIcon;
extern const char* DefaultFolderIcon;
extern const char* DefaultLargeIcon;
extern const char* DefaultShortcutIcon;

extern XColor IconForeColor, IconBackColor;
extern XColor IconStringColor, IconStringActiveColor;
extern XColor MiniatureColor, MiniatureActiveColor;
extern XColor TitlebarColor, TitlebarActiveColor;
extern XColor TitlebarColor2, TitlebarActiveColor2;
extern XColor TitleStringColor, TitleStringActiveColor;
extern XColor MenuColor, MenuActiveColor;
extern XColor MenuStringColor, MenuStringActiveColor;
extern XColor DialogColor, DialogStringColor;
extern XColor SwitcherColor, SwitcherActiveColor, SwitcherStringColor;
extern XColor FrameColor, FrameActiveColor;
extern XColor PagerColor, PagerActiveColor;
extern XColor ButtonColor, ButtonActiveColor;
extern XColor ButtonStringColor, ButtonStringActiveColor;
extern XColor TaskbarColor;
extern XColor ClockStringColor;
extern XColor DesktopColor, DesktopActiveColor;
extern XColor StartMenuLogoColor;
extern XColor CursorColor;
extern XColor TooltipColor, TooltipStringColor;

extern const char *DefaultFont;
extern const char *TitleFont, *TaskbarFont, *TaskbarBoldFont, *TaskbarClockFont;
extern const char *CascadeMenuFont, *CtrlMenuFont, *StartMenuFont, *IconFont, *DialogFont;

extern MenuElem* StartMenuItem;
extern MenuElem* CtrlMenuItem;
extern MenuElem* DesktopMenuItem;
extern MenuElem* IconMenuItem;
extern MenuElem* TaskbarMenuItem;
extern MenuElem* ExitDlgItem;
extern MenuElem* ShortCutItem;
extern SCKeyTable* scKey;
extern SCKeyTable* menuKey;

extern SCKeyEntry scMenuKey[];

extern VariableTable varTable[];
extern FsNameSet fsSet[];
extern AttrNameSet attrSet[];
extern KeyMod keyMod[];

#endif // _QVWMRC_H_
