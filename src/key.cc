/*
 * key.cc
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
#include <X11/keysym.h>
#include "main.h"
#include "util.h"
#include "key.h"
#include "menu.h"
#include "qvwmrc.h"

int ShortCutKey::altMask = Mod1Mask;
int ShortCutKey::metaMask = Mod1Mask;
int ShortCutKey::numLockMask = 0;

#define MOD_ALT  2
#define MOD_META 3

/* XXX */
extern KeyCode swCode;
extern unsigned int swMod;

ShortCutKey::ShortCutKey(KeyCode c, unsigned int modifier, FuncNumber fn)
: mod(modifier), func(fn), exec(NULL)
{
  code = c;
  next = NULL;
}
 
ShortCutKey::ShortCutKey(KeyCode c, unsigned int modifier, char* command)
: mod(modifier), func(Q_EXEC), exec(command)
{
  code = c;
  next = NULL;
} 

void ShortCutKey::GrabKeys(Window w)
{
  if (code != 0) {
    XGrabKey(display, code, mod, w, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, code, (mod | LockMask), w, True,
	     GrabModeAsync, GrabModeAsync);
    if (numLockMask) {
      XGrabKey(display, code, (mod | numLockMask), w, True,
	       GrabModeAsync, GrabModeAsync);
      XGrabKey(display, code, (mod | LockMask | numLockMask), w, True,
	       GrabModeAsync, GrabModeAsync);
    }
  }
}

void ShortCutKey::UngrabKeys(Window w)
{
  if (code != 0) {
    XUngrabKey(display, code, mod, w);
    XUngrabKey(display, code, (mod | LockMask), w);
    if (numLockMask) {
      XUngrabKey(display, code, (mod | numLockMask), w);
      XUngrabKey(display, code, (mod | LockMask | numLockMask), w);
    }
  }
}

Bool ShortCutKey::ExecShortCutKey(unsigned int keycode, unsigned int state,
				  Menu* menu)
{
  if (keycode == code && (state & ~(LockMask | numLockMask)) == mod) {
    switch (func) {
    case Q_EXEC:
      if (exec) {
	PlaySound(OpenSound);
	ExecCommand(exec);
      }
      return True;
      
    /* XXX - this doesn't belong here: */
    case Q_SWITCH_TASK:
    case Q_SWITCH_TASK_BACK:
      swCode = keycode;
      swMod = state;
      // FALLTHROUGH

    default:
      // YYY -- remove useless:
      //Qvwm *q = menu->GetQvwm();
      //QvwmError( "func: %d, window: '%s'", func, q->GetName() );
      QvFunction::execFunction(func, menu);
      return True;
    }
  }

  return False;
}

void ShortCutKey::SetModifier()
{
  XModifierKeymap *map = XGetModifierMapping(display);
  int i, j, k = map->max_keypermod * Mod1MapIndex;

  for (i = Mod1MapIndex; i <= Mod5MapIndex; i++) {
    for (j = 0; j < map->max_keypermod; j++) {
      if (map->modifiermap[k]) {
	KeySym sym = XKeycodeToKeysym(display, map->modifiermap[k], j);

	switch (sym) {
	case XK_Alt_L:
	case XK_Alt_R:
	  keyMod[MOD_ALT].mask = altMask = 1 << i;
	  break;

	case XK_Meta_L:
	case XK_Meta_R:
	  keyMod[MOD_META].mask = metaMask = 1 << i;
	  break;

	case XK_Num_Lock:
	  numLockMask = 1 << i;
	  break;
	}
      }
      k++;
    }
  }

  XFreeModifiermap(map);
}

SCKeyTable::SCKeyTable()
: m_scKey(NULL)
{
  XDisplayKeycodes(display, &m_minKeycode, &m_maxKeycode);
}

SCKeyTable::~SCKeyTable()
{
  ShortCutKey* pSck;

  for (ShortCutKey* sck = m_scKey; sck != NULL; sck = pSck) {
    pSck = sck->GetNext();
    delete sck;
  }
}

void SCKeyTable::AddSCKey(KeySym sym, unsigned int modifier, FuncNumber fn)
{
  ShortCutKey* sck;

  for (int i = m_minKeycode; i < m_maxKeycode; i++) {
    if (XKeycodeToKeysym(display, i, 0) == sym) {
      sck = new ShortCutKey(i, modifier, fn);
      sck->SetNext(m_scKey);
      m_scKey = sck;
    }
  }
}

void SCKeyTable::AddSCKey(KeySym sym, unsigned int modifier, char* exec)
{
  ShortCutKey* sck;

  for (int i = m_minKeycode; i < m_maxKeycode; i++) {
    if (XKeycodeToKeysym(display, i, 0) == sym) {
      sck = new ShortCutKey(i, modifier, exec);
      sck->SetNext(m_scKey);
      m_scKey = sck;
    }
  }
}

void SCKeyTable::AddSCKey(KeyCode code, unsigned int modifier, FuncNumber fn)
{
  ShortCutKey *sck;

  sck = new ShortCutKey(code, modifier, fn);
  sck->SetNext(m_scKey);
  m_scKey = sck;
}

void SCKeyTable::AddSCKey(KeyCode code, unsigned int modifier, char* exec)
{
  ShortCutKey *sck;

  sck = new ShortCutKey(code, modifier, exec);
  sck->SetNext(m_scKey);
  m_scKey = sck;
}

/*
 * GrabKeys --
 *   Grab keys of a window.
 */
void SCKeyTable::GrabKeys(Window w)
{
  for (ShortCutKey* sck = m_scKey; sck != NULL; sck = sck->GetNext())
    sck->GrabKeys(w);
}

/*
 * UngrabKeys --
 *   Ungrab keys of a window.
 */
void SCKeyTable::UngrabKeys(Window w)
{
  for (ShortCutKey* sck = m_scKey; sck != NULL; sck = sck->GetNext())
    sck->UngrabKeys(w);
}

/*
 * ExecShortCutKey --
 *   Execute the function of shortcut key.
 */
Bool SCKeyTable::ExecShortCutKey(unsigned int keycode, unsigned int state,
				 Menu* menu)
{
  for (ShortCutKey* sck = m_scKey; sck != NULL; sck = sck->GetNext())
    if (sck->ExecShortCutKey(keycode, state, menu))
      return True;

  return False;  // no short cut key
}
