/*
 * session.cc
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
#ifdef USE_XSMP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/SM/SMlib.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include "main.h"
#include "session.h"
#include "desktop.h"
#include "qvwm.h"
#include "list.h"

void
Session::DieCallback(SmcConn arg_connId, SmPointer client_data)
{
  ((Session*)client_data)->Die(arg_connId);
}

void
Session::SaveCompleteCallback(SmcConn arg_connId, SmPointer client_data)
{
  ((Session*)client_data)->SaveComplete(arg_connId);
}

void
Session::ShutdownCancelledCallback(SmcConn arg_connId, SmPointer client_data)
{
  ((Session*)client_data)->ShutdownCancelled(arg_connId);
}

void
Session::SaveYourselfPhase2Callback(SmcConn arg_connId, SmPointer client_data)
{
  ((Session*)client_data)->SaveYourselfPhase2(arg_connId);
}

void
Session::SaveYourselfCallback(SmcConn arg_connId,
			      SmPointer client_data,
			      int save_type,
			      Bool shutdown,
			      int interact_style,
			      Bool fast)
{
  ((Session*)client_data)->SaveYourself(arg_connId,
					save_type, shutdown,
					interact_style, fast);
}

void
Session::IceConnWatchCallback(IceConn conn,
			      IcePointer client_data,
			      Bool opening,
			      IcePointer* watch_data)
{
  ((Session*)client_data)->IceConnWatch(conn, opening, watch_data);
}

void
Session::Die(SmcConn arg_connId)
{
  desktop.FinishQvwm(False);
}

void
Session::SaveComplete(SmcConn arg_connId)
{
}

void
Session::ShutdownCancelled(SmcConn arg_connId)
{
  if (!saveDone) {
    SmcSaveYourselfDone(connId, False);
    saveDone = True;
  }
}

void
Session::SaveYourselfPhase2(SmcConn arg_connId)
{
  FILE* fp = OpenSessionFile("w");
  if (!fp) {
    if (!saveDone) {
      SmcSaveYourselfDone(connId, False);
      saveDone = True;
    }
    return;
  }

  List<Qvwm>* qvwmList = &desktop.GetQvwmList();
  List<Qvwm>::Iterator i(qvwmList);
  Qvwm* qvWm;
  int idx=0;
  for (qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
    if (!qvWm->SavePropertiesToFile(idx++, fp)) {
      //break;
    }
  }

  fclose(fp);

  if (!saveDone) {
    SmcSaveYourselfDone(connId, qvWm ? False : True);
    saveDone = True;
  }
}

void
Session::SaveYourself(SmcConn arg_connId,
		      int save_type, Bool shutdown,
		      int interact_style, Bool fast)
{
  if (!SmcRequestSaveYourselfPhase2(connId,
				    &SaveYourselfPhase2Callback,
				    (SmPointer)this)) {
    SmcSaveYourselfDone(connId, False);
    saveDone = True;
  } else {
    saveDone = False;
  }
}

void
Session::IceConnWatch(IceConn conn,
		      Bool opening,
		      IcePointer* watch_data)
{
  if (opening) {
    if (iceConnFd < 0) {
      iceConnFd = IceConnectionNumber(conn);
    }
  } else if (IceConnectionNumber(conn) == iceConnFd) {
    iceConnFd = -1;
  }
}

void
Session::InitializeProperties()
{
  // SmClone
  int argv0_len = strlen(qvArgv[0]);
  SmPropValue cloneCommandValue[1] = {
    {argv0_len,qvArgv[0]}
  };
  SmProp cloneCommand = {
    SmCloneCommand,
    SmLISTofARRAY8,
    1,
    cloneCommandValue
  };

  // SmDiscardCommand
  int path_rm_len = strlen(PATH_RM);
  int session_file_len = strlen(sessionFile);
  SmPropValue discardCommandValue[3] = {
    {path_rm_len, (void *)PATH_RM},
    {3, (void *)"-f"},
    {session_file_len, sessionFile}
  };
  SmProp discardCommand = {
    SmDiscardCommand,
    SmLISTofARRAY8,
    3,
    discardCommandValue
  };

  // SmProgram
  SmPropValue programValue[1] = {
    {argv0_len,qvArgv[0]}
  };
  SmProp program = {
    SmProgram,
    SmARRAY8,
    1,
    programValue
  };

  // SmRestartCommand
  char* client_id_arg = "-clientId";
  int client_id_len = strlen(clientId);
  SmPropValue restartCommandValue[3] = {
    {argv0_len, qvArgv[0]},
    {9, client_id_arg},
    {client_id_len, clientId}
  };
  SmProp restartCommand = {
    SmRestartCommand,
    SmLISTofARRAY8,
    3,
    restartCommandValue
  };

  // UserID
  char* user_id = getenv("USER");
  int user_id_len = strlen(user_id);
  SmPropValue userIdValue[1] = {
    {user_id_len, user_id}
  };
  SmProp userId = {
    SmUserID,
    SmARRAY8,
    1,
    userIdValue
  };

  char val = 30;
  SmPropValue gsmPriorityValue[1] = {
      {1, &val}
  };
  SmProp gsmPriority = {
    "_GSM_Priority",
    SmCARD8,
    1,
    gsmPriorityValue
  };

  SmProp *properties[6] = {
    &cloneCommand,
    &discardCommand,
    &program,
    &restartCommand,
    &userId,
    &gsmPriority
  };
  
  SmcSetProperties(connId, 6, properties);
}

Bool
Session::ConnectSessionManager(char* arg_previousId)
{
  SmcCallbacks callbacks;
  char errorStringRet[256];

  callbacks.save_yourself.callback = &SaveYourselfCallback;
  callbacks.save_yourself.client_data = (SmPointer)this;
  callbacks.die.callback = &DieCallback;
  callbacks.die.client_data = (SmPointer)this;
  callbacks.save_complete.callback = &SaveCompleteCallback;
  callbacks.save_complete.client_data = (SmPointer)this;;
  callbacks.shutdown_cancelled.callback = &ShutdownCancelledCallback;
  callbacks.shutdown_cancelled.client_data = (SmPointer)this;;

  connId = SmcOpenConnection(NULL, NULL, 1, 0,
			     // the 1.0 spec requires support of all procs
			     SmcSaveYourselfProcMask |
			     SmcDieProcMask |
			     SmcSaveCompleteProcMask |
			     SmcShutdownCancelledProcMask,
			     &callbacks,
			     arg_previousId,
			     &clientId,
			     sizeof(errorStringRet),
			     errorStringRet);
  return (connId == NULL) ? False : True;
}

Session::Session(char* arg_previousId)
{
  status = False;
  iceConnFd = -1;
  saveDone = False;
  sessionFile[0] = '\0';
  connId = NULL;

  if (!getenv("SESSION_MANAGER"))
    return;
  if (!IceAddConnectionWatch(&IceConnWatchCallback, (IcePointer)this))
    return;
  if (!ConnectSessionManager(arg_previousId))
    return;
  iceConn = SmcGetIceConnection(connId);
  if (!getenv("HOME") ||
      strlen(getenv("HOME")) + strlen(clientId) + 12 > sizeof sessionFile)
    return;
  sprintf(sessionFile, "%s/.qvwm/%s.ini", getenv("HOME"), clientId);
  InitializeProperties();
  status = True;
}

Session::~Session()
{
  if (status)
    SmcCloseConnection(connId, 0, NULL);
}

#if 0
void
Session::debug(char* fmt_string, ...)
{
  va_list ap;
  va_start(ap, fmt_string);
  FILE* fp = fopen("/tmp/qvwm-error", "a");
  if (fp == NULL)
    return;
  vfprintf(fp, fmt_string, ap);
  fclose(fp);
  va_end(ap);
}
#endif

void
Session::CloseConnection()
{
  SmcCloseConnection(connId, 0, NULL);
  iceConn = NULL;
  iceConnFd = -1;
}

Bool
Session::RequestQuit()
{
  if (!status)
    return False;
  SmcRequestSaveYourself(connId,
			 SmSaveBoth,
			 True,	// shutdown is taking place
			 SmInteractStyleAny,
			 False,	// not fast
			 True);	// global
  return True;
}

FILE*
Session::OpenSessionFile(const char* mode)
{
  char* dir;
  dir = new char[strlen(getenv("HOME")) + 7];
  if (dir == NULL) {
    return NULL;
  }
  sprintf(dir, "%s/.qvwm", getenv("HOME"));

  struct stat st;
  if (stat(dir, &st) != 0) {
      if (errno != ENOENT) {       // ENOENT == 'dir' does not exist.
	return NULL;
      }
      mkdir(dir, 0755);
  }
  delete [] dir;
  return fopen(sessionFile, mode);
}

Bool
Session::ReadVector(char** p, int* x, int* y)
{
  *x = 0;
  *y = 0;

  while (**p && isdigit(**p))
    *x = *x * 10 + (*(*p)++ - '0');
  if (*(*p)++ != ',')
    return False;
  while (**p && isdigit(**p))
    *y = *y * 10 + (*(*p)++ - '0');
  return True;
}

Bool
Session::LoadSessionFile()
{
  if (!status)
    return False;
  FILE* fp = OpenSessionFile("r");
  if (!fp) {
    return True;
  }
  char buf[1024];
  size_t l;
  int idx = -1;
  int cur_idx;
  char *p, *key;
  int x, y;
  QvwmProto* qp = NULL;
  while (feof(fp) == 0) {
    if (fgets(buf, sizeof buf, fp) == NULL)
      break;
    l = strlen(buf);
    if (buf[l - 1] == '\n') // chomp!
      buf[--l] = '\0';
    p = buf;
    if (!isdigit(*p))
      continue;
    cur_idx = 0;
    while (isdigit(*p))
      cur_idx = cur_idx * 10 + (*p++ - '0');
    if (cur_idx != idx) {
      // new hint.
      if (qp) {
	qvwmProtoList.InsertTail(qp);
	qp = NULL;
      }
      idx = cur_idx;
      qp = new QvwmProto();
    }
    if (*p != ',')
      continue;
    key = ++p;
    while (*p && *p != '=') 
      p++;
    if (!*p)
      continue;
    *p++ = '\0';
    if (strcmp(key, "id") == 0) {
      qp->SetId(p);
    } else if (strcmp(key, "class") == 0) {
      qp->SetRClass(p);
    } else if (strcmp(key, "rname") == 0) {
      qp->SetRName(p);
    } else if (strcmp(key, "name") == 0) {
      qp->SetName(p);
    } else if (strcmp(key, "pos") == 0) {
      if (ReadVector(&p, &x, &y))
	qp->SetPos(x, y);
    } else if (strcmp(key, "size") == 0) {
      if (ReadVector(&p, &x, &y))
	qp->SetSize(x, y);
#if 0
    } else if (strcmp(key, "flags") == 0) {
      unsigned long f = 0;
      // read something..
      qp->SetFlags(f);
#endif
    } else {
      // do nothing
    }
  }
  if (qp) {
    qvwmProtoList.InsertTail(qp);
  }
  fclose(fp);
  return True;
}

#if 0
void
QvwmProto::dump(const char* label)
{
  Session::debug("  QP: %s: (%s,%s,%s,%s,%s,x=%d,y=%d,w=%d,h=%d)\n",
		 label, id, role, res_class, res_name, name,
		 x, y,
		 width, height);
}
#endif

Bool
Session::SearchProto(const unsigned char* smid,
		     const unsigned char* wm_role, 
		     const char* rclass, const char* rname,
		     const char* name,
		     int* x, int* y, int* width, int* height /*,int* flags*/)
{
  if (!status)
    return False;

  List<QvwmProto>* qlist = &qvwmProtoList;
  List<QvwmProto>::Iterator i(qlist);
  QvwmProto* qp;
  Bool found = False;
  for (qp = i.GetHead(); qp; qp = i.GetNext()) {
    const char* qsmid = qp->GetId();
    const char* qrole = qp->GetRole();
    if ((!smid && qsmid) ||
	(smid && !qsmid) ||
	(smid && qsmid && strcmp((const char*)smid, (const char*)qsmid))) {
      continue;
    }
    if (wm_role || qrole) {
      if (wm_role && qrole &&
	  strcmp((const char*)wm_role, (const char*)qrole) == 0) {
	found = True;
	break;
      }
    }

    const char* qrclass = qp->GetRClass();
    const char* qrname = qp->GetRName();
    const char* qname = qp->GetName();
    if (!rclass || !rname || !qrclass || !qrname) {
      continue;
    }
    // compare WM_CLASS and WM_NAME to identify the window
    if (strcmp(rclass, qrclass) == 0 && strcmp(rname, qrname) == 0 &&
	strcmp(name, qname) == 0) {
      if (qsmid) {
	found = True;
	break;
      }
      // SM_CLIENT_ID is not set. Check other attributes to indetify.
      // (but not implemented)
    }
  }
  if (found) {
    *x = qp->GetX();
    *y = qp->GetY();
    *width = qp->GetWidth();
    *height = qp->GetHeight();
    //*flags = qp->GetFlags();
    qvwmProtoList.Remove(qp);
    return True;
  }
  return False;
}

// end of session.cc

#endif // USE_XSMP
