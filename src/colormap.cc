/*
 * colormap.cc
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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "util.h"
#include "desktop.h"

void Qvwm::ChangeColormap(const XColormapEvent& ev)
{
  XWindowAttributes attr;
  Bool reInstall = False;
  XEvent xev;
  Qvwm* qvWm;

  if (ev.c_new) {
    XGetWindowAttributes(display, ev.window, &attr);
    if (this == desktop.GetCmapInstalled() && GetNumCmapWins() == 0)
      desktop.SetCurrentCmap(attr.colormap);
    reInstall = True;
  }
  else if (ev.state == ColormapUninstalled &&
	   desktop.GetCurrentCmap() == ev.colormap)
    reInstall = True;
  
  while (XCheckTypedEvent(display, ColormapNotify, &xev)) {
    XColormapEvent* cev = (XColormapEvent *)&xev;

    if (XFindContext(display, cev->window, Qvwm::context, (caddr_t *)&qvWm)
	== XCSUCCESS) {
      ASSERT(qvWm);

      if (cev->c_new) {
	XGetWindowAttributes(display, qvWm->GetWin(), &attr);
	if (qvWm == desktop.GetCmapInstalled() && qvWm->GetNumCmapWins() == 0)
	  desktop.SetCurrentCmap(attr.colormap);
	reInstall = True;
      }
      else if (cev->state == ColormapUninstalled &&
	       desktop.GetCurrentCmap() == cev->colormap)
	reInstall = False;
    }
  }
  
  if (reInstall) {
    if (desktop.GetCurrentCmap()) {
      XSync(display, 0); /* XXX */
      XInstallColormap(display, desktop.GetCurrentCmap());
    }
#ifdef DEBUG
    else
      printf("curCmap is None\n");
#endif
  }
}

/*
 * InstallWindowColormaps --
 *   Install window colormap.
 */
void Qvwm::InstallWindowColormaps()
{
  XWindowAttributes attr;
  Bool isThisWin = False;

  desktop.SetCmapInstalled(this);

  if (nCmapWins > 0) {
    for (int i = nCmapWins - 1; i >= 0; i--) {
      if (cmapWins[i] == wOrig)
	isThisWin = True;
      XGetWindowAttributes(display, cmapWins[i], &attr);

      if (desktop.GetCurrentCmap() != attr.colormap) {
	desktop.SetCurrentCmap(attr.colormap);
	XInstallColormap(display, attr.colormap);
      }
    }
  }
  
  if (!isThisWin) {
    XGetWindowAttributes(display, wOrig, &attr);
    if (desktop.GetCurrentCmap() != attr.colormap) {
      desktop.SetCurrentCmap(attr.colormap);
      XInstallColormap(display, attr.colormap);
    }
  }
}

/*
 * FetchWMColormapWindows --
 *
 */
void Qvwm::FetchWMColormapWindows()
{
  if(XGetWMColormapWindows(display, wOrig, &cmapWins, &nCmapWins) == 0) {
    cmapWins = NULL;
    nCmapWins = 0;
  }
}
