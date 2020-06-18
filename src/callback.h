/*
 * callback.h
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

#ifndef CALLBACK_H_
#define CALLBACK_H_

class BasicCallback {
public:
  virtual void Execute() = 0;

  virtual Bool Equal(BasicCallback* cb) = 0;
};

template <class T>
class Callback : public BasicCallback {
  typedef void (T::*FuncPtr)();

private:
  T* obj;
  FuncPtr func;

public:
  Callback(T* o, FuncPtr f) : obj(o), func(f) {}

  void Execute() { (obj->*func)(); }

  Bool Equal(BasicCallback* bcb) {
    Callback<T>* cb = (Callback<T> *)bcb;
    if (obj == cb->obj && func == cb->func)
      return True;
    return False;
  }
};

class GlobalCallback : public BasicCallback {
  typedef void (*FuncPtr)();

private:
  FuncPtr func;

public:
  GlobalCallback(FuncPtr f) : func(f) {}

  void Execute() { (*func)(); }

  Bool Equal(BasicCallback* bcb) {
    GlobalCallback* cb = (GlobalCallback *)bcb;
    if (func == cb->func)
      return True;
    return False;
  }
};

#endif // CALLBACK_H_
