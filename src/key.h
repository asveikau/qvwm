/*
 * key.h
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

#ifndef _KEY_H_
#define _KEY_H_

/*
 * ShortCutKey.
 */
class ShortCutKey {
private:
  KeyCode code;			// keycode
  unsigned int mod;		// modifier
  FuncNumber func;		// function
  char* exec;			// command when func is Q_EXEC
  ShortCutKey* next;

  static int altMask;
  static int metaMask;
  static int numLockMask;

public:
  ShortCutKey(KeyCode c, unsigned int modifier, FuncNumber fn);
  ShortCutKey(KeyCode c, unsigned int modifier, char* command);
  ~ShortCutKey() {}

  void SetNext(ShortCutKey* nSck) { next = nSck; }
  ShortCutKey* GetNext() const { return next; }

  void GrabKeys(Window w);
  void UngrabKeys(Window w);
  Bool ExecShortCutKey(unsigned int keycode, unsigned int state,
		       Menu* menu);

  static void SetModifier();
  static int getAltMask() { return altMask; }
  static int getMetaMask() { return metaMask; }
  static int getAltMetaMask() { return altMask | metaMask; }
  static int getNumLockMask() { return numLockMask; }
};

class SCKeyTable {
private:
  int m_minKeycode;
  int m_maxKeycode;

  ShortCutKey* m_scKey;

public:
  SCKeyTable();
  ~SCKeyTable();

  void AddSCKey(KeySym sym, unsigned int modifier, FuncNumber fn);
  void AddSCKey(KeySym sym, unsigned int modifier, char* exec);
  void AddSCKey(KeyCode code, unsigned int modifier, FuncNumber fn);
  void AddSCKey(KeyCode code, unsigned int modifier, char* exec);
  
  void GrabKeys(Window w);
  void UngrabKeys(Window w);
  Bool ExecShortCutKey(unsigned int keycode, unsigned int state,
		       Menu* menu);
};

struct SCKeyEntry {
  KeySym sym;
  unsigned int mod;
  FuncNumber func;
};

#endif // _KEY_H_
