/*
 * session.h
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

/* -*- c++ -*- */
#ifndef SESSION_H_
#define SESSION_H_

#ifdef USE_XSMP

#include <X11/SM/SMlib.h>
#include <X11/ICE/ICElib.h>
#include <sys/param.h>

#include "list.h"

class QvwmProto {
  char* id;
  char* role;
  char* res_class;
  char* res_name;
  char* name;
  int x, y;
  int width, height;
  unsigned long flags;
public:
  QvwmProto() {
    id = role = res_class = res_name = name = NULL;
    x = y = width = height = 0;
    flags = 0;
  }
  ~QvwmProto() {
    delete id;
    delete role;
    delete res_class;
    delete res_name;
    delete name;
  }
  void SetId(char* aid) { id = strdup(aid); }
  void SetRole(char* arole) { role = strdup(arole); }
  void SetRClass(char* arclass) { res_class = strdup(arclass); }
  void SetRName(char* arname) { res_name = strdup(arname); }
  void SetName(char* aname) { name = strdup(aname); }
  void SetPos(int ax, int ay) { x = ax; y = ay; }
  void SetSize(int aw, int ah) { width = aw; height = ah; }
  void SetFlags(unsigned long af) { flags = af; }

  // <@_@>
  const char* GetId() const { return id; }
  const char* GetRole() const { return role; }
  const char* GetRClass() const { return res_class; }
  const char* GetRName() const { return res_name; }
  const char* GetName() const { return name; }
  int GetX() const { return x; }
  int GetY() const { return y; }
  int GetWidth() const { return width; }
  int GetHeight() const { return height; }
  unsigned long GetFlags() const { return flags; }

  //  void dump(const char* label);
};

class Session {
private:
  static void DieCallback(SmcConn arg_connId, SmPointer client_data);
  static void SaveCompleteCallback(SmcConn arg_connId, SmPointer client_data);
  static void ShutdownCancelledCallback(SmcConn arg_connId,
					SmPointer client_data);
  static void SaveYourselfPhase2Callback(SmcConn arg_connId,
					 SmPointer client_data);
  static void SaveYourselfCallback(SmcConn arg_connId,
				   SmPointer client_data,
				   int save_type,
				   Bool shutdown,
				   int interact_style,
				   Bool fast);
  static void IceConnWatchCallback(IceConn conn, IcePointer client_data,
				   Bool opening, IcePointer* watch_data);

  FILE* OpenSessionFile(const char* mode);
  void Die(SmcConn arg_connId);
  void SaveComplete(SmcConn arg_connId);
  void ShutdownCancelled(SmcConn arg_connId);
  void SaveYourselfPhase2(SmcConn arg_connId);
  void SaveYourself(SmcConn arg_connId,
		    int save_type, Bool shutdown, int interact_style,
		    Bool fast);
  void IceConnWatch(IceConn conn, Bool opening, IcePointer* watch_data);
  void InitializeProperties();

  Bool ConnectSessionManager(char* arg_previousId);
  Bool ReadVector(char** p, int* x, int* y);

  SmcConn connId;
  char* previousId;
  char* clientId;

  char sessionFile[MAXPATHLEN];

  int iceConnFd;
  IceConn iceConn;
  Bool status;
  Bool saveDone;

  List<QvwmProto> qvwmProtoList; // Qvwm prototype list

public:
  Session(char* arg_previousId);
  virtual ~Session();
  void CloseConnection();
  Bool LoadSessionFile();
  Bool SearchProto(const unsigned char* smid,
		   const unsigned char* wm_role, 
		   const char* rclass, const char* rname,
		   const char* name,
		   int* x, int* y, int* width, int* height);
    
  //static void debug(char* fmt_string, ...);

  int GetIceConnFd() const { return iceConnFd; }
  IceConn GetIceConn() const { return iceConn; }
  Bool GetStatus() const { return status; }

  Bool RequestQuit();
};

extern Session* session;

#endif // USE_XSMP

#endif // SESSION_H_
