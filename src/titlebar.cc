/*
 * titlebar.cc
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
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "qvwmrc.h"
#include "event.h"
#include "fbutton.h"
#include "tooltip.h"

QvImage* Qvwm::imgTitlebar;
QvImage* Qvwm::imgActiveTitlebar;

unsigned long* gradPattern;
unsigned long* gradActivePattern;

/*
 * CreateTitle --
 *   Create title window, which is the child of frame window.
 */
void Qvwm::CreateTitle(const Rect& rect)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                          EnterWindowMask | LeaveWindowMask |
			  Button1MotionMask | PointerMotionMask;
  attributes.background_pixel = darkGray.pixel;
  valueMask = CWBackPixel | CWEventMask;

  title = XCreateWindow(display, frame,
			rect.x, rect.y, rect.width, rect.height,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);
  XSaveContext(display, title, context, (caddr_t)this);

  rcTitle = rect;
  CalcShortName();

  toolTip = new Tooltip();
  toolTip->SetString(name);

  if (GradTitlebar) {
    pixGrad = None;
    pixActiveGrad = None;
  }

  if (TitlebarImage) {
    imgTitle = imgTitlebar->Duplicate();
    imgTitle->SetBackground(title);
  }
  if (TitlebarActiveImage)
    imgActiveTitle = imgActiveTitlebar->Duplicate();
}

char* Qvwm::GetNameFromHint()
{
  XTextProperty xtp;
  char** cl;
  int n;

  delete [] name;

  if (XGetWMName(display, GetWin(), &xtp) != 0) {
    XmbTextPropertyToTextList(display, &xtp, &cl, &n);
    if (cl) {
      name = new char[strlen(cl[0]) + 1];
      strcpy(name, cl[0]);
      XFreeStringList(cl);
    }
    else {
      name = new char[9];
      strcpy(name, "Untitled");
    }
  }
  else {
    name = new char[9];
    strcpy(name, "Untitled");
  }

  return name;
}

void Qvwm::CalcShortName()
{
  delete [] shortname;

  int nameWidth = rcTitle.width - 2;  // 2 is the space between name and button
  int num = 0;

  if (CheckFlags(CTRL_MENU))
    nameWidth -= TaskbarButton::SYMBOL_SIZE + 4;
  else
    nameWidth -= 4;

  if (CheckFlags(BUTTON1))
    num++;
  if (CheckFlags(BUTTON2))
    num++;
  if (CheckFlags(BUTTON3))
    num++;

  nameWidth -= BUTTON_WIDTH * num + 4;
  if (num == 1)
    nameWidth += 2;

  shortname = GetFixName(fsTitle, name, nameWidth);
}

/*
 * DrawTitle --
 *   Draw title.
 */
void Qvwm::DrawTitle(Bool focusChange)
{
  Point ptTitle;
  XRectangle ink, log;

  if (this == rootQvwm || !CheckFlags(TITLE))
    return;

  if (focusChange) {
    if (CheckFocus()) {
      if (TitlebarImage)
	imgTitle->SetBackground(None);

      if (TitlebarActiveImage)
	imgActiveTitle->SetBackground(title);
      else if (GradTitlebar) {
	if (pixActiveGrad == None)
	  pixActiveGrad = CreateGradPixmap(gradActivePattern, rcTitle.width,
					   title);
	XSetWindowBackgroundPixmap(display, title, pixActiveGrad);
      }
      else
	XSetWindowBackground(display, title, TitlebarActiveColor.pixel);
    }
    else {
      if (TitlebarActiveImage)
	imgActiveTitle->SetBackground(None);

      if (TitlebarImage)
	imgTitle->SetBackground(title);
      else if (GradTitlebar) {
	if (pixGrad == None)
	  pixGrad = CreateGradPixmap(gradPattern, rcTitle.width, title);
	XSetWindowBackgroundPixmap(display, title, pixGrad);
      }
      else
	XSetWindowBackground(display, title, TitlebarColor.pixel);
    }
  }
    
  XClearWindow(display, title);

  DrawCtrlMenuMark();
  
  /*
   * Draw the title text.
   */
  XmbTextExtents(fsTitle, shortname, strlen(shortname), &ink, &log);
  if (CheckFlags(CTRL_MENU))
    ptTitle.x = TaskbarButton::SYMBOL_SIZE + 4;
  else
    ptTitle.x = 4;
  ptTitle.y = (TITLE_HEIGHT - log.height)/2 - log.y;
  
  if (CheckFocus())
    XSetForeground(display, gc, TitleStringActiveColor.pixel);
  else
    XSetForeground(display, gc, TitleStringColor.pixel);
  
  XmbDrawString(display, title, fsTitle, gc, ptTitle.x, ptTitle.y,
		shortname, strlen(shortname));

  if (strlen(name) > strlen(shortname)) {
    XRectangle xr[3] =
    {{ptTitle.x + log.width + 2, ptTitle.y - 2, 1, 2},
     {ptTitle.x + log.width + 5, ptTitle.y - 2, 1, 2},
     {ptTitle.x + log.width + 8, ptTitle.y - 2, 1, 2}};
    
    XFillRectangles(display, title, gc, xr, 3);
  }
}

/*
 * MotionTitlebar --
 *   Move the titlebar to dest slowly.
 */
void Qvwm::MotionTitlebar(const Rect& rcSrc, const Rect& rcDest)
{
  Window motionBarWin;
  Rect rcNew;
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  QvImage* imgIcon = GetIconImage();

  if (!TitlebarMotion)
    return;

  XSync(display, 0);

  attributes.save_under = DoesSaveUnders(ScreenOfDisplay(display, screen));
  attributes.background_pixel = TitlebarActiveColor.pixel;
  valueMask = CWSaveUnder | CWBackPixel;

  /*
   * Create a motion title bar.
   */
  motionBarWin = XCreateWindow(display, root, rcSrc.x, rcSrc.y,
			       rcSrc.width, TITLE_HEIGHT,
			       0, CopyFromParent, InputOutput, CopyFromParent, 
			       valueMask, &attributes);

  XMapRaised(display, motionBarWin);

  /*
   * Move the titlebar slowly.
   */
  XSetForeground(display, gc, white.pixel);

  for (int i = 1; i < TitlebarMotionSpeed; i++) {
    rcNew.x = rcSrc.x + (rcDest.x - rcSrc.x) * i / TitlebarMotionSpeed;
    rcNew.y = rcSrc.y + (rcDest.y - rcSrc.y) * i / TitlebarMotionSpeed;
    rcNew.width = rcSrc.width + (rcDest.width - rcSrc.width) * i
      / TitlebarMotionSpeed;

    XMoveResizeWindow(display, motionBarWin,
		      rcNew.x, rcNew.y, rcNew.width, TITLE_HEIGHT);
    imgIcon->Display(motionBarWin, Point(2, 1));

    /*
     * If you don't have this function, motion title bar may be underneath
     * main window.
     */
    XRaiseWindow(display, motionBarWin);

    XFlush(display);
    usleep(10000);
  }

  XDestroyWindow(display, motionBarWin);
}

void Qvwm::TitleButton1Press(Time clickTime, const Point& ptRoot)
{
  /*
   * When double click.
   */
  if (!CheckFlags(TRANSIENT) &&
      IsDoubleClick(titleClickTime, clickTime, event.ptPrevRoot, ptRoot)) {
    ASSERT(fButton[1]);
    fButton[1]->SetState(Button::NORMAL);
    if (CheckStatus(MAXIMIZE_WINDOW)) {
      /*
       * Restore if maximun window.
       */
      fButton[1]->ChangeImage(FrameButton::MAXIMIZE);
      RestoreWindow();
    }
    else {
      /*
       * Maximize if normal window.
       */
      fButton[1]->ChangeImage(FrameButton::RESTORE);
      MaximizeWindow();
    }
  }
  titleClickTime = clickTime;
}

void Qvwm::CreateTitlebarPixmap()
{
  if (TitlebarImage) {
    imgTitlebar = CreateImageFromFile(TitlebarImage, timer);
    if (imgTitlebar == None) {
      delete [] TitlebarImage;
      TitlebarImage = NULL;
    }
  }
  if (TitlebarActiveImage) {
    imgActiveTitlebar = CreateImageFromFile(TitlebarActiveImage, timer);
    if (imgActiveTitlebar == None) {
      delete [] TitlebarActiveImage;
      TitlebarActiveImage = NULL;
    }
  }
}

// titlebar general routines
Pixmap CreateGradPixmap(unsigned long* pat, int width, Window win)
{
  Pixmap pix = XCreatePixmap(display, win, width, 4, depth);
  GC gc = XCreateGC(display, pix, 0, 0);
  XImage* image = XGetImage(display, pix, 0, 0, width, 4, AllPlanes, XYPixmap);
  int* xIndex = new int[width];
  int level = (GradTitlebarColors - 1) / 3;
  int k = 0, sw;

  // calculate the position of base pattern
  for (int i = 0; i < level; i++) {
    int base = i * 28;
    for (int j = 0; j < 28;) {
      switch (j) {
	// # of pattern is 2
      case 0: case 6: case 8: case 10: case 16: case 18: case 20: case 26:
	sw = 0;
	for (; k < (base + j + 2) * width / (level * 28) - 1; k++) {
	  xIndex[k] = base + j + sw;
	  sw = (sw + 1) & 1;
	}
	j += 2;
	break;

	// # of pattern is 4
      case 2: case 12: case 22:
	sw = 0;
	for (; k < (base + j + 4) * width / (level * 28) - 1; k++) {
	  xIndex[k] = base + j + sw;
	  sw = (sw + 1) & 3;
	}
	j += 4;
	break;
      }
    }
  }
  for (; k < width; k++)
    xIndex[k] = level * 28 - 1;

  for (int x = 0; x < width; x++)
    for (int y = 0; y < 4; y++)
      XPutPixel(image, x, y, pat[xIndex[x] * 4 + y]);

  XPutImage(display, pix, gc, image, 0, 0, 0, 0, width, 4);
  XDestroyImage(image);
  XFreeGC(display, gc);
  delete [] xIndex;

  return pix;
}

static unsigned long* CreatePattern(XColor base1, XColor base2);
static void SetPattern(unsigned long* pat, int offset, unsigned long p[]);

void CreateGradPattern()
{
  if (GradTitlebarColors == 1) {
    GradTitlebar = False;
    return;
  }

  gradPattern = CreatePattern(TitlebarColor, TitlebarColor2);
  gradActivePattern = CreatePattern(TitlebarActiveColor,
				    TitlebarActiveColor2);
}

static unsigned long* CreatePattern(XColor base1, XColor base2)
{
  int level = (GradTitlebarColors - 1) / 3;

  // need 4 colors at minimum
  if (level == 0) {
    GradTitlebarColors = 4;  // XXX
    level = 1;
  }

  unsigned long* pat = new unsigned long[28 * level * 4];
  XColor* cMajor = new XColor[level + 1];
  int diffRed = base2.red - base1.red;
  int diffGreen = base2.green - base1.green;
  int diffBlue = base2.blue - base1.blue;
  int i, j;

  // major color allocation
  for (i = 0; i < (level + 1) / 2; i++) {
    XColor* sub = (i == 0) ? NULL : &cMajor[i - 1];

    CreateColor(base1.red   + diffRed   * i / level,
		base1.green + diffGreen * i / level,
		base1.blue  + diffBlue  * i / level,
		&cMajor[i], sub, "titlebar gradation");
  }
  for (j = level; j >= i; j--) {
    XColor* sub = (j == level) ? NULL : &cMajor[j + 1];

    CreateColor(base1.red   + diffRed   * j / level,
		base1.green + diffGreen * j / level,
		base1.blue  + diffBlue  * j / level,
		&cMajor[j], sub, "titlebar gradation");
  }

  // minor color allocation
  for (i = 0; i < level; i++) {
    double scale1 = (i + 0.33) / level;
    double scale2 = (i + 0.66) / level;
    XColor cMinor[2];
    unsigned long p[4];

    CreateColor(base1.red   + int(diffRed   * scale1),
		base1.green + int(diffGreen * scale1),
		base1.blue  + int(diffBlue  * scale1),
		&cMinor[0], &cMajor[i]);

    CreateColor(base1.red   + int(diffRed   * scale2),
		base1.green + int(diffGreen * scale2),
		base1.blue  + int(diffBlue  * scale2),
		&cMinor[1], &cMajor[i + 1]);

    p[0] = cMajor[i].pixel;
    p[1] = cMinor[0].pixel;
    p[2] = cMinor[1].pixel;
    p[3] = cMajor[i + 1].pixel;

    SetPattern(pat, 28 * i, p);
  }

  delete [] cMajor;
  
  return pat;
}

static void SetPattern(unsigned long* pat, int offset, unsigned long p[])
{
  static int table[4][28] =
    {{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 2,
       1, 2, 2, 2, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3 },
     { 0, 0, 2, 0, 1, 0, 2, 0, 2, 0, 2, 0, 2, 1,
       2, 1, 3, 1, 3, 1, 3, 1, 3, 2, 3, 1, 3, 3 },
     { 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 2,
       1, 2, 2, 2, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3 },
     { 0, 0, 1, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 1,
       2, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 2, 3, 3 }};
  
  for (int x = 0; x < 28; x++)
    for (int y = 0; y < 4; y++)
      pat[(x + offset) * 4 + y] = p[table[y][x]];
}
