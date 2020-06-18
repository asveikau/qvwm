/*
 * property.cc
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "hash.h"
#include "tbutton.h"
#include "qvwmrc.h"
#include "util.h"
#include "desktop.h"
#include "gnome.h"
#include "image.h"
#include "dialog.h"
#include "switcher.h"
#include "session.h"
#include "tooltip.h"

Hash<AppAttr>* Qvwm::appHashTable;

#if 0
static Pixmap FixIconPixmap(Pixmap origPix, unsigned int width,
			    unsigned int height, int size, unsigned int depth,
			    unsigned long fore, unsigned long back);
#endif
static QvImage* GetDefLogo(int size);

/*
 * SetProperty --
 *   Set window property.
 */
void Qvwm::SetProperty(Atom atom)
{
  switch (atom) {
  case XA_WM_NAME:
    name = GetNameFromHint();
    toolTip->SetString(name);
    CalcShortName();
    DrawTitle(False);
    break;

  case XA_WM_ICON_NAME:
    ASSERT(tButton);
    tButton->ChangeName();
    tButton->DrawButton();
    break;

  case XA_WM_HINTS:
    if (!CheckFlags(LARGE_IMG)) {
      QvImage::Destroy(imgLarge);
      imgLarge = GetFixedIcon(32);
    }
    if (!CheckFlags(SMALL_IMG)) {
      QvImage::Destroy(imgSmall);
      imgSmall = GetFixedIcon(16);
      ASSERT(tButton);
      tButton->SetImage(imgSmall->Duplicate());
      tButton->DrawButton();
    }
    break;

  case XA_WM_NORMAL_HINTS:
    GetWindowSizeHints();
    break;

  default:
    if (atom == _XA_WM_PROTOCOLS)
      FetchWMProtocols();
    else if (atom == _XA_WM_COLORMAP_WINDOWS) {
      FetchWMColormapWindows();
      InstallWindowColormaps();
    }
  }
}

/*
 * GetWindowClassHints --
 *
 */
void Qvwm::GetWindowClassHints()
{
  AppAttr* attrs;
  int argc;
  char** argv;
  char* cmdName;

  // Get an application name
  name = GetNameFromHint();

  if (XGetClassHint(display, wOrig, &classHints) == 0) {
    classHints.res_name = NULL;
    classHints.res_class = NULL;
  }
  
  if (XGetCommand(display, wOrig, &argv, &argc) == 0 ||
      argv == NULL || argv[0] == NULL)
    cmdName = NULL;
  else {
    // get only a basename
    cmdName = strrchr(argv[0], '/');
    if (cmdName)
      cmdName++;
    else
      cmdName = argv[0];
  }

  /*
   * Set flags and pixmaps according to resource name, resource class and 
   * application name.
   */
  if ((attrs = appHashTable->GetHashItem(name)) ||
      (attrs = appHashTable->GetHashItem(classHints.res_name)) ||
      (attrs = appHashTable->GetHashItem(classHints.res_class)) ||
      (attrs = appHashTable->GetHashItem(cmdName))) {
    // give higher priority to window decorations specified by a 
    // configuration file than MWM hints (i.e. prefer unset bits)
    flags = ~(~flags | ~attrs->flags) & DECOR_MASK;
    flags |= attrs->flags & ~DECOR_MASK;

    if (!CheckFlags(BORDER))
      SetFlags(THIN_BORDER);

    /*
     * If LARGE_IMG is specified and large pixmap hasn't been created,
     * create it.
     */
    if (CheckFlags(LARGE_IMG)) {
      if (attrs->large_img == NULL) {
	attrs->large_img = CreateImageFromFile(attrs->large_file,
					       TaskSwitcher::swTimer);
	if (attrs->large_img)
	  imgLarge = attrs->large_img->Duplicate();
	else
	  ResetFlags(LARGE_IMG);
      }
      else
	imgLarge = attrs->large_img->Duplicate();
    }

    /*
     * If SMALL_IMG is specified and large pixmap hasn't been created,
     * create it.
     */
    if (CheckFlags(SMALL_IMG)) {
      if (attrs->small_img == NULL) {
	attrs->small_img = CreateImageFromFile(attrs->small_file, timer);
	if (attrs->small_img)
	  imgSmall = attrs->small_img->Duplicate();
	else
	  ResetFlags(SMALL_IMG);
      }
      else
	imgSmall = attrs->small_img->Duplicate();
    }

    if (CheckFlags(GEOMETRY))
      geom = &attrs->geom;

    if (CheckFlags(ONTOP)) {
      desktop.GetOnTopList().InsertTail(this);
      Gnome::ChangeLayer(this, WIN_LAYER_ONTOP);
    }
  }

  // free returned argv when XGetCommand succeeded
  if (cmdName)
    XFreeStringList(argv);
}

/*
 * GetWindowSizeHints --
 *   Get window size hints.
 */
void Qvwm::GetWindowSizeHints()
{
  long supplied;

  if (!XGetWMNormalHints(display, wOrig, &hints, &supplied))
    hints.flags = 0;
  
  if (hints.flags & PResizeInc) {
    if (hints.width_inc == 0)
      hints.width_inc = 1;
    if (hints.height_inc == 0)
      hints.height_inc = 1;
  }
  else {
    hints.width_inc = 1;
    hints.height_inc = 1;
  }
  
  if (!(hints.flags & PBaseSize)) {
    if (hints.flags & PMinSize) {
      hints.base_width = hints.min_width;
      hints.base_height = hints.min_height;
    }
    else {
      hints.base_width = 0;
      hints.base_height = 0;
    }
  }
  
  if (!(hints.flags & PMinSize)) {
    hints.min_width = hints.base_width;
    hints.min_height = hints.base_height;
  }
    
  if (!(hints.flags & PMaxSize)) {
    hints.max_width = MAX_WINDOW_WIDTH;
    hints.max_height = MAX_WINDOW_HEIGHT;
  }
    
  if (!(hints.flags & PWinGravity)) {
    hints.win_gravity = NorthWestGravity;
    hints.flags |= PWinGravity;
  }
}

/*
 * GetGravityOffset --
 *   Get the offset needed in the gravity.
 */
Point& Qvwm::GetGravityOffset()
{
  int gr;

  if (CheckFlags(GEOMETRY) && !CheckFlags(FIXED_POS))
    return geom->gravity;

  if (hints.flags & PWinGravity)
    gr = hints.win_gravity;
  else
    gr = NorthWestGravity;

  if (gr < ForgetGravity || gr > StaticGravity)
    return GravityOffset[NorthWestGravity];
  else
    return GravityOffset[gr];
}
  
/*
 * FetchWMProtocols --
 */
void Qvwm::FetchWMProtocols()
{
  Atom *protocols, *ap;
  int i, n;
  Atom atype;
  int aformat;
  unsigned long bytes_remain, nitems;
  
  if (XGetWMProtocols(display, wOrig, &protocols, &n)) {
    ap = protocols;
    for (i = 0; i < n; i++) {
      if (*ap == (Atom)_XA_WM_DELETE_WINDOW)
	SetFlags(WM_DELETE_WINDOW);
      if (*ap == (Atom)_XA_WM_TAKE_FOCUS)
	SetFlags(WM_TAKE_FOCUS);
      ap++;
    }
    if (protocols)
      XFree(protocols);
  }
  else {
    if ((XGetWindowProperty(display, wOrig, _XA_WM_PROTOCOLS, 0L, 10L, False,
			    _XA_WM_PROTOCOLS, &atype, &aformat, &nitems,
			    &bytes_remain, (unsigned char **)&protocols))
	== Success) {
      ap = protocols;
      for (i = 0; i < (int)nitems; i++) {
	if (*ap == (Atom)_XA_WM_DELETE_WINDOW)
	  SetFlags(WM_DELETE_WINDOW);
	if (*ap == (Atom)_XA_WM_TAKE_FOCUS)
	  SetFlags(WM_TAKE_FOCUS);
	ap++;
      }
      if (protocols)
	XFree(protocols);
    }
  }
}

void Qvwm::GetTransientForHint()
{
  Window tWin;

  if (XGetTransientForHint(display, wOrig, &tWin)) {
    if (XFindContext(display, tWin, context, (caddr_t*)&qvMain) == XCSUCCESS) {
      qvMain->trList.InsertTail(this);
      if (qvMain->CheckFlags(ONTOP) && !CheckFlags(ONTOP)) {
	SetFlags(ONTOP);
	desktop.GetOnTopList().InsertTail(this);
	Gnome::ChangeLayer(this, WIN_LAYER_ONTOP);
      }
    }
    else
      qvMain = NULL;

    SetFlags(TRANSIENT);
    ResetFlags(CTRL_MENU | BORDER_EDGE | BUTTON1 | BUTTON2 | BUTTON3);
  }
  else {
    qvMain = NULL;           // meaning this window itself is a main window
    ResetFlags(TRANSIENT);
  }
}

/*
 * GetFixedIcon --
 *   Get icon whose size becomes 16 or 32.
 */
QvImage* Qvwm::GetFixedIcon(int size)
{
  QvImage* img;

// FixIconPixmap() is unstable.
#if 0
  Window junkRoot;
  Rect rect;
  unsigned int junkBW, pixDepth;
  QvImage* img;

  if (wmHints == NULL || !(wmHints->flags & IconPixmapHint) ||
      wmHints->icon_pixmap == None)
    img = GetDefLogo(size);
  else {
    if (XGetGeometry(display, wmHints->icon_pixmap, &junkRoot,
		     &rect.x, &rect.y,
		     (unsigned int *)&rect.width, (unsigned int *)&rect.height,
		     &junkBW, &pixDepth) == 0) {
      img = GetDefLogo(size);
      XFree(wmHints);
      return img;
    }
	
    if (rect.width == 0 || rect.height == 0) {
      img = GetDefLogo(size);
      XFree(wmHints);
      return img;
    }

    pix = FixIconPixmap(wmHints->icon_pixmap, rect.width, rect.height, size,
			depth, IconForeColor.pixel, IconBackColor.pixel);
    if (pix == None) {
      img = GetDefLogo(size);
      XFree(wmHints);
      return;
    }

    if ((wmHints->flags & IconMaskHint) && wmHints->icon_mask != None) {
      mask = FixIconPixmap(wmHints->icon_mask, rect.width, rect.height, size,
			   1, 1, 0);
      gcv.clip_mask = mask;
      gc = XCreateGC(display, root, GCClipMask, &gcv);
    }
    else
      gc = XCreateGC(display, root, 0, 0);

    img = new QvImage(pix, gc, Dim(size, size));
  }
#else
  img = GetDefLogo(size);
#endif // 0  

  return img;
}

/*
 * GetDefLogo --
 *   Get default logo pixmap.
 */
static QvImage* GetDefLogo(int size)
{
  QvImage* img;
  
  if (size == 16)
    img = imgLogo->Duplicate();
  else if (size == 32)
    img = imgLargeLogo->Duplicate();
  else
    img = NULL;

  return img;
}

#if 0
/*
 * FixIconPixmap --
 *   Adjust icon pixmap size.
 */
static Pixmap FixIconPixmap(Pixmap origPix, unsigned int width,
			    unsigned int height, int size, unsigned int depth,
			    unsigned long fore, unsigned long back)
{
  XImage *srcImage, *destImage;
  char* data = new char[(size + 7) / 8 * size * depth];
  Point pt;
  unsigned long pixel;
  Pixmap pix;
  GC gc;

  srcImage = XGetImage(display, origPix, 0, 0, width, height, AllPlanes,
		       XYPixmap);
  destImage = XCreateImage(display, DefaultVisual(display, screen), depth,
			   XYPixmap, 0, data, size, size, 8, 0);

  for (int x = 0; x < size; x++) {
    pt.x = (int)((double)width / size * x);
    for (int y = 0; y < size; y++) {
      pt.y = (int)((double)height / size * y);

      pixel = XGetPixel(srcImage, pt.x, pt.y);
      XPutPixel(destImage, x, y, pixel ? fore : back);
    }
  }

  pix = XCreatePixmap(display, root, size, size, depth);
  gc = XCreateGC(display, pix, 0, 0);
  XPutImage(display, pix, gc, destImage, 0, 0, 0, 0, size, size);

  XFreeGC(display, gc);
  XDestroyImage(srcImage);
  XFree(destImage);
  delete [] data;

  return pix;
}
#endif // 0

void Qvwm::SetStateProperty(int state)
{
  unsigned long data[2];
  
  ASSERT(tButton);

  data[0] = (unsigned long)state;
  data[1] = (unsigned long)tButton->GetFrameWin();
  
  XChangeProperty(display, wOrig, _XA_WM_STATE, _XA_WM_STATE, 32, 
		  PropModeReplace, (unsigned char *)data, 2);
}

#ifdef USE_XSMP
void Qvwm::GetSMClientId()
{
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;

  if (smClientId) {
    XFree(smClientId);
    smClientId = NULL;
  }
  // WM_CLIENT_LEADER
  //  type: WINDOW
  //  format: 8
  //  -- the window ID of the client leader window
  unsigned char* prop = NULL;
  if (XGetWindowProperty(display, wOrig,
			 _XA_WM_CLIENT_LEADER, 0, 1, False,
			 XA_WINDOW, &atype, &aformat,
			 &nitems, &bytes_remain, &prop) != Success) {
    return;
  }
  Window leader = None;
  if (atype == XA_WINDOW && aformat == 32 &&
      nitems == 1 && bytes_remain == 0) {
    leader = ((Window *)prop)[0];
  }
  if (prop)
    XFree(prop);
  if (leader == None)
    return;

  // SM_CLIENT_ID
  //   type: STRING
  //   format: 8
  //   XPCS, ISO 8859-1
  //   -- the client leader window must have this prop.
  XTextProperty text_prop;
  if (XGetTextProperty(display, leader, &text_prop, _XA_SM_CLIENT_ID)) {
    if (text_prop.encoding == XA_STRING &&
	text_prop.format == 8 && text_prop.nitems != 0) {
      smClientId = text_prop.value;
    }
  }
}

void Qvwm::GetWMRole()
{
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;

  // WM_WINDOW_ROLE
  //  type: STRING
  //  format: 8
  //  XPCS, ISO 8859-1
  //  -- a string that uniquely identifies that window amaong
  //     all windows that have the same client leader window.
  unsigned char* wm_role = NULL;
  if (XGetWindowProperty(display, wOrig,
			 _XA_WM_WINDOW_ROLE, 0, 256, False,
			 XA_STRING, &atype, &aformat,
			 &nitems, &bytes_remain, &wm_role) != Success) {
    wm_role = NULL;
  }
  if(wmRole)
    XFree(wmRole);
  if (wm_role)
    wmRole = wm_role;
  else
    wmRole = NULL;
}
#endif // USE_XSMP
