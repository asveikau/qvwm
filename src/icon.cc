/*
 * icon.cc
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
#include <unistd.h>
#ifdef __EMX__
#include <process.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#ifdef USE_SHAPE
#include <X11/extensions/shape.h>
#endif
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "icon.h"
#include "util.h"
#include "event.h"
#include "startmenu.h"
#include "qvwmrc.h"
#include "desktop.h"
#include "pixmap_image.h"
#include "callback.h"
#include "tooltip.h"
#include "bitmaps/icon32.xpm"

Icon*		Icon::focusIcon = NULL;
XContext	Icon::context;
QvImage*	Icon::imgIcon;
IconMenu*	Icon::ctrlMenu = NULL;

// image must be passed as a duplicated object
Icon::Icon(QvImage* image, const char* iconname, const char* execname, int x, int y)
: img(image), exec(execname)
{
  isBuiltin = False;

  init(iconname, x, y);
}

// image must be passed as a duplicated object
Icon::Icon(QvImage* image, const char* iconname, FuncNumber func, int x, int y)
: img(image), fn(func)
{
  isBuiltin = True;
  
  init(iconname, x, y);
}

void Icon::init(const char* iconname, int x, int y)
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  rcVirt.x = x;
  rcVirt.y = y;

  dragging = False;
  enableDrop = True;

  /*
   * Compute icon text format and icon text size/position
   */
  AlterIconName(iconname);

  rc.width = IconSize + IconHorizontalSpacing;
  rc.height = IconSize + IconVerticalSpacing;
  if (rc.height < 2 + IconSize + 4 + rcText.height)
    rc.height = 2 + IconSize + 4 + rcText.height;
  
  /*
   * Create an icon image window.
   */
  attributes.background_pixel = DesktopColor.pixel;
  attributes.override_redirect = True;
  valueMask = CWBackPixel | CWOverrideRedirect;

  frame = XCreateWindow(display, root,
			0, 0, IconSize, IconSize,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  /*
   * Create an icon text window.
   */
  attributes.event_mask = ExposureMask;
  valueMask = CWBackPixel | CWOverrideRedirect | CWEventMask;

  text = XCreateWindow(display, root,
		       0, 0, rcText.width, rcText.height,
		       0, CopyFromParent, InputOutput, CopyFromParent,
		       valueMask, &attributes);

  /*
   * create a shadow window (for dragging)
   */
  shadow = XCreateWindow(display, root,
			 0, 0, rc.width, rc.height,
			 0, CopyFromParent, InputOutput, CopyFromParent,
			 valueMask, &attributes);

  /*
   * Create an icon wapper window.
   */
  attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
                          Button1MotionMask | PointerMotionMask |
                          EnterWindowMask | LeaveWindowMask;
  valueMask = CWOverrideRedirect | CWEventMask;

  wrap = XCreateWindow(display, root,
		       0, 0, rc.width, rc.height,
		       0, CopyFromParent, InputOnly, CopyFromParent,
		       valueMask, &attributes);
  
  XSaveContext(display, text, context, (caddr_t)this);
  XSaveContext(display, wrap, context, (caddr_t)this);
  XSaveContext(display, shadow, context, (caddr_t)this);

#ifdef USE_SHAPE
  shapeMask = XCreatePixmap(display, frame, IconSize, IconSize, 1);
  gcShape = XCreateGC(display, shapeMask, 0, 0);
#endif

  SetIconImage();

  img->SetDisplayCallback(new Callback<Icon>(this, &Icon::SetIconImage));

  toolTip = new Tooltip(iconname, &fsIcon);
}

Icon::~Icon()
{
  delete [] name;

  XDeleteContext(display, text, context);
  XDeleteContext(display, wrap, context);

  XDestroyWindow(display, frame);
  XDestroyWindow(display, text);
  XDestroyWindow(display, wrap);

  XFreePixmap(display, shapeMask);
  XFreeGC(display, gcShape);

  if (this == focusIcon)
    focusIcon = NULL;

  delete toolTip;
  QvImage::Destroy(img);
}

void Icon::SetIconImage()
{
  img->SetBackground(frame);

  CreateShapedWindow();

  XClearWindow(display, frame);
}

/*
 * Create an window shaped by icon image.
 */
void Icon::CreateShapedWindow()
{
#ifdef USE_SHAPE
  // First, make the whole transparent (pixel value 0 is transparent).
  XSetForeground(display, gcShape, 0);
  XFillRectangle(display, shapeMask, gcShape, 0, 0, IconSize, IconSize);

  XSetForeground(display, gcShape, 1);
  XSetBackground(display, gcShape, 0);

  // Opaque icon pixmap.
  if (img->GetMask()) {
    Dim size = img->GetSize();
    XCopyPlane(display, img->GetMask(), shapeMask, gcShape,
	       0, 0, size.width, size.height, 0, 0, 1);
  }
  else
    XFillRectangle(display, shapeMask, gcShape, 0, 0, IconSize, IconSize);

  if (this == focusIcon) {
    // create semi-transparent mask for icon image
    GC gcTile = CreateTileGC(shapeMask);
    XFillRectangle(display, shapeMask, gcTile, 0, 0, IconSize, IconSize);
    XFreeGC(display, gcTile);
  }

  XShapeCombineMask(display, frame, ShapeBounding, 0, 0, shapeMask, ShapeSet);
#endif // USE_SHAPE
}

void Icon::CreateShapedShadowWindow()
{
#ifdef USE_SHAPE
  Pixmap shadowMask;
  GC gcShadow;

  shadowMask = XCreatePixmap(display, shadow, rc.width, rc.height, 1);
  gcShadow = XCreateGC(display, shadowMask, 0, 0);

  XSetForeground(display, gcShadow, 0);
  XFillRectangle(display, shadowMask, gcShadow, 0, 0, rc.width, rc.height);
  
  XSetForeground(display, gcShadow, 1);
  XSetBackground(display, gcShadow, 0);

  if (img->GetMask())
    XCopyPlane(display, img->GetMask(), shadowMask, gcShadow,
	       0, 0, IconSize, IconSize, IconHorizontalSpacing / 2, 2, 1);
  else
    XFillRectangle(display, shadowMask, gcShadow,
		   IconHorizontalSpacing / 2, 2, IconSize, IconSize);

  // create semi-transparent mask for icon image
  GC gcTile = CreateTileGC(shadowMask);
  XFillRectangle(display, shadowMask, gcTile,
		 IconHorizontalSpacing / 2, 2, IconSize, IconSize);
  XFreeGC(display, gcTile);

  // create shaped icon name
  XSetForeground(display, gcShadow, 0);  // transparent
  XFillRectangle(display, shadowMask, gcShadow,
		 rcText.x, rcText.y, rcText.width, rcText.height);

  XSetForeground(display, gcShadow, 1);
  DrawText(shadowMask, gcShadow, rcText);

  XShapeCombineMask(display, shadow, ShapeBounding, 0, 0, shadowMask,
		    ShapeSet);
#endif // USE_SHAPE
}

/*
 * MapIcon --
 *   Map icon itself and the wrapper.
 */
void Icon::MapIcon()
{
  XLowerWindow(display, wrap);
  XMapWindow(display, wrap);

  XLowerWindow(display, frame);
  XMapWindow(display, frame);
  XLowerWindow(display, text);
  XMapWindow(display, text);
}

/*
 * unmapIcon --
 *   Unmap icon itself and the wrapper.
 */
void Icon::UnmapIcon()
{
  XUnmapWindow(display, frame);
  XUnmapWindow(display, text);

  XUnmapWindow(display, wrap);
}

void Icon::SetFocus()
{
  focusIcon = this;
  CreateShapedWindow();
  DrawIcon(True);
}

void Icon::ResetFocus()
{
  focusIcon = NULL;
  CreateShapedWindow();
  DrawIcon(False);
}

/*
 * DrawIcon --
 *   Draw icon name.
 */
void Icon::DrawIcon(Bool focus)
{
  if (dragging && img)
    img->Display(shadow, Point(IconHorizontalSpacing / 2, 2));

  if (focus)
    XSetForeground(display, gc, DesktopActiveColor.pixel);
  else
    XSetForeground(display, gc, DesktopColor.pixel);
  
  XFillRectangle(display, text, gc, 0, 0, rcText.width, rcText.height);
    
  if (focus)
    XSetForeground(display, gc, IconStringActiveColor.pixel);
  else
    XSetForeground(display, gc, IconStringColor.pixel);
    
  Rect range(0, 0, rcText.width, rcText.height);
  DrawText(text, gc, range);

  if (dragging) {
    XSetForeground(display, gc, black.pixel);
    range = Rect(rcText.x, rcText.y, rcText.width, rcText.height);
    DrawText(shadow, gc, range);
  }

  if (focus) {
    XSetForeground(display, gcDash, yellow.pixel);
    XDrawRectangle(display, text, gcDash,
		   0, 0, rcText.width - 1, rcText.height - 1);
  }
}

/*
 * Draw each line of the text.
 */
void Icon::DrawText(Drawable d, GC dGc, Rect rect)
{
  char *str, *start, *end;
  XRectangle ink, log;

  start = end = name;

  while (*end) {
    // Find delimiter, and save it
    while (*end && *end != '\n')
      end++;

    char prev = *end;
    if (prev)
      *end = '\0';
    
    // Center this line of text
    str = GetFixName(fsIcon, start, IconSize + IconHorizontalSpacing);
    XmbTextExtents(fsIcon, str, strlen(str), &ink, &log);
    
    // Draw it
    XmbDrawString(display, d, fsIcon, dGc,
		  rect.x + (rect.width - log.width) / 2,
		  rect.y - log.y,
		  str, strlen(str));
    if (strlen(start) > strlen(str)) {
      XRectangle xr[3] = {
	{(short)(rect.x + log.width + 2), (short)(rect.y - log.y - 2), 1, 2},
	{(short)(rect.x + log.width + 5), (short)(rect.y - log.y - 2), 1, 2},
	{(short)(rect.x + log.width + 8), (short)(rect.y - log.y - 2), 1, 2}
      };

      XFillRectangles(display, d, dGc, xr, 3);
    }

    delete [] str;
    
    // Onto the next
    rect.y += rcText.height / textLines;
    if (prev)
      *end++ = prev;
    start = end;
  }
}

void Icon::MoveIcon(const Point& pt)
{
  XMoveWindow(display, frame, pt.x + IconHorizontalSpacing / 2, pt.y + 2);
  XMoveWindow(display, text, pt.x + rcText.x, pt.y + rcText.y);
  XMoveWindow(display, wrap, pt.x, pt.y);
}

/*
 * Button1Press --
 *   Process press of button1 (mouse left button)
 */
void Icon::Button1Press(Time clickTime, const Point& ptRoot)
{
  /*
   * Take off the focus of short cut icon, if any.
   */
  if (Icon::focusIcon != NULL)
    Icon::focusIcon->ResetFocus();

  toolTip->Disable();

  /*
   * Unmap all menus and move focus to root window.
   */
  Menu::UnmapAllMenus();

  rootQvwm->SetFocus();

  /*
   * If double click, execute the function.
   */
  if (IsDoubleClick(iconClickTime, clickTime, event.ptPrevRoot, ptRoot)) {
    if (isBuiltin)
      QvFunction::execFunction(fn);
    else {
      PlaySound(OpenSound);
      ExecCommand(exec);
    }
    return;
  }
  else
    SetFocus();

  iconClickTime = clickTime;
}

void Icon::Button1Motion(const Point& ptRoot)
{
  XEvent ev;
  Point ptOld, ptNew, ptCur;
  Bool pointer = False;

  if (DisableDesktopChange)
    return;

  ptCur = Point(rc.x, rc.y);
  ptOld = ptRoot;

  /*
   * Process icon movement.
   */
  while (1) {
    XMaskEvent(display,
	       ExposureMask | Button1MotionMask | ButtonReleaseMask |
	       ButtonPressMask | PointerMotionMask,
	       &ev);
    switch (ev.type) {
    case Expose:
      event.DispatchEvent(ev);
      break;

    case MotionNotify:
      if (!dragging) {
	dragging = True;
	CreateShapedShadowWindow();
	XMoveWindow(display, shadow, ptCur.x, ptCur.y);
	XMapRaised(display, shadow);
      }

      ptNew = Point(ev.xbutton.x_root, ev.xbutton.y_root);
      ptCur.x += ptNew.x - ptOld.x;
      rcVirt.x += ptNew.x - ptOld.x;
      ptCur.y += ptNew.y - ptOld.y;
      rcVirt.y += ptNew.y - ptOld.y;

      ptOld = ptNew;
      XMoveWindow(display, shadow, ptCur.x, ptCur.y);

      if (IsPointerInWindow(ptNew)) {
	if (enableDrop) {
	  XDefineCursor(display, root, cursor[DISALLOW]);
	  enableDrop = False;
	}
      }
      else {
	if (!enableDrop) {
	  XDefineCursor(display, root, cursor[SYS]);
	  enableDrop = True;
	}
      }
      break;

    case ButtonRelease:
      if (LockDragState) {
	pointer = True;
	XGrabPointer(display, root, True, ButtonPressMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, root, None, CurrentTime);
	break;
      }
      else
	goto decide;

    case ButtonPress:
      if (pointer) {
	XUngrabPointer(display, CurrentTime);
	goto decide;
      }
      return;
    }
  }

decide:
  if (dragging) {
    XUnmapWindow(display, shadow);
    if (enableDrop) {
      MoveIcon(Point(ptCur.x, ptCur.y));
      rc.x = ptCur.x;
      rc.y = ptCur.y;
    }
    else {
      XDefineCursor(display, root, cursor[SYS]);
      enableDrop = True;
    }
    dragging = False;
    DrawIcon(True);
  }
}

/*
 * Button3Press --
 *   Process press of button3.
 */
void Icon::Button3Release(const Point& ptRoot)
{
  Point pt;
  int dir;

  /*
   * Take off the focus of short cut icon, if any.
   */
  if (Icon::focusIcon != NULL)
    Icon::focusIcon->ResetFocus();

  Menu::UnmapAllMenus();
  rootQvwm->SetFocus();

  SetFocus();

  toolTip->Disable();

  pt = ctrlMenu->GetFixedMenuPos(ptRoot, dir);
  ctrlMenu->SetIcon(this);
  ctrlMenu->MapMenu(pt.x, pt.y, dir);
}

void Icon::Enter()
{
  toolTip->SetTimer();
}

void Icon::Leave()
{
  toolTip->Disable();
}

void Icon::PointerMotion()
{
  if (!toolTip->IsMapped())
    toolTip->ResetTimer();
}

void Icon::Initialize()
{
  Icon::context = XUniqueContext();

  if (DefaultShortcutIcon == NULL)
    imgIcon = new PixmapImage(icon32);
  else {
    imgIcon = CreateImageFromFile(DefaultShortcutIcon, timer);
    if (imgIcon == NULL)
      imgIcon = new PixmapImage(icon32);
  }
}
  
/*
 * AlterIconName --
 *   checks the icon name's size and splits it on several lines if necessary.
 */
void Icon::AlterIconName(const char* iconname)
{
  XRectangle ink, log;
  char *start, *end;
  int nblanks, len, i;
  int* sizes;
  char space;
  int spaceSize;
  int lineHeight;

  // Make a copy of the string
  name = new char[strlen(iconname) + 1];
  strcpy(name, iconname);
  
  // Compute the width of a space, and the height of a line
  space = ' ';
  XmbTextExtents(fsIcon, &space, 1, &ink, &log);
  spaceSize = log.width;
  lineHeight = log.height;

  // Compute the width of the name
  XmbTextExtents(fsIcon, name, strlen(name), &ink, &log);
  
  // If it fits on a sigle line, no problem.
  if (log.width <= IconSize + IconHorizontalSpacing) {
    rcText.width = log.width;
    rcText.height = log.height;
    textLines = 1;
  }
  // Else try to split the name into several lines
  else {
    rcText.width = 0;
    rcText.height = lineHeight;
    textLines = 1;

    // First check for blanks.
    end = name;
    nblanks = 0;
    while (*end++)
      if (*end == ' ')
	nblanks++;

    // If the text has blanks, we can break the text
    if (nblanks) {
      // Compute every word's size
      sizes = new int[nblanks + 1];
      start = end = name;
      for (i = 0; i < nblanks; i++) {
        while (*end++ != ' ')
	  ;
        *(--end) = 0;
        XmbTextExtents(fsIcon, start, (int)(end - start), &ink, &log);
        sizes[i] = log.width;
        *end = ' ';
	start = ++end;
      }
      XmbTextExtents(fsIcon, end, strlen(end), &ink, &log); // last word
      sizes[nblanks] = log.width;
    
      // Insert line breaks in the text.
      len = i = 0;
      start = end = name;
      while (i < nblanks + 1) {
        len += sizes[i];
        while (*end && *end != ' ')
	  end++;
        // If too large, insert a break before the word
        if (len > IconSize + IconHorizontalSpacing) {
          if (start != name) { // can't break before first word !
            len -= sizes[i];
            if (len > rcText.width)
	      rcText.width = len;
            *start = '\n';
            len = sizes[i] + spaceSize;
            rcText.height += lineHeight;
            textLines++;
          }
        }
        else
          len += spaceSize;
        start = end++;
	i++;
      }
      len -= spaceSize; // no extra space at the end
      if (len > rcText.width)
	rcText.width = len;
    }
    else {
      rcText.width = IconSize + IconHorizontalSpacing;
      rcText.height = log.height;
    }
  }

  rcText.x = (IconSize + IconHorizontalSpacing - rcText.width) / 2;
  rcText.y = 2 + IconSize + 4;

  if (rcText.width == 0)
    rcText.width = 1;
  if (rcText.height == 0)
    rcText.height = 1;
}

void IconMenu::ExecFunction(FuncNumber fn, int i)
{
  switch (fn) {
  case Q_EXEC_ICON:
    if (icon) {
      if (icon->IsBuiltin())
	QvFunction::execFunction(icon->GetFunc());
      else {
	PlaySound(OpenSound);
	ExecCommand(icon->GetExec());
      }
    }
    break;

  case Q_DELETE_ICON:
    desktop.GetIconList().Remove(icon);
    delete icon;
    icon = NULL;
    break;

  default:
    QvwmError("cannot execute a specified function");
    break;
  }
}
