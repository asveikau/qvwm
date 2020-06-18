/*
 * remote_cmd.cc
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

#ifdef ALLOW_RMTCMD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "remote_cmd.h"
#include "function.h"
#include "hash.h"
#include "qvwmrc.h"

#define TMP_DIR "/tmp/"
#define FIFO_PREFIX "qvwm-"

RemoteCommand::RemoteCommand()
{
  char *dispName = DisplayString(display);
  int dispLen = strlen(dispName);
  int dirLen = strlen(TMP_DIR);
  int prefixLen = strlen(FIFO_PREFIX);

  m_fifoName = (char *)malloc(dirLen + prefixLen + dispLen + 1);
  sprintf(m_fifoName, "%s%s%s", TMP_DIR, FIFO_PREFIX, dispName);

  if (mknod(m_fifoName, S_IFIFO | 0600, 0) == -1) {
    if (errno != EEXIST) {
      QvwmError("RemoteCommand: mknod: %s", strerror(errno));
      m_fd = -1;
      return;
    }

    if (unlink(m_fifoName) == -1) {
      QvwmError("'%s' exists but cannot be removed: %s",
		m_fifoName, strerror(errno));
      m_fd = -1;
      return;
    }

    QvwmError("removed '%s'", m_fifoName);

    if (mknod(m_fifoName, S_IFIFO | 0600, 0) == -1) {
      QvwmError("mknod '%s': %s", m_fifoName, strerror(errno));
      m_fd = -1;
      return;
    }
  }


  if ((m_fd = open(m_fifoName, O_RDWR | O_NONBLOCK)) == -1) {
    QvwmError("open '%s': %s", m_fifoName, strerror(errno));
    return;
  }

}

RemoteCommand::~RemoteCommand()
{
  if (m_fd != -1) {
    close(m_fd);
    if (unlink(m_fifoName) == -1)
      QvwmError("RemoteCommand: unlink '%s': %s", m_fifoName, strerror(errno));
    free(m_fifoName);
  }
}

#define BUF_SIZE 256

void RemoteCommand::process()
{
  char buf[BUF_SIZE];
  char *cmd, *ptr;
  int len;

  if ((len = read(m_fd, buf, BUF_SIZE)) < 0) {
    if (errno != EAGAIN)
      QvwmError("RemoteCommand: read: %s", strerror(errno));
    return;
  }

  cmd = ptr = buf;

  while (len > 0) {
    if (*ptr == '\n') {
      *ptr = '\0';
      interpret(cmd);

      cmd = ptr + 1;  // point to the next command
    }

    ptr++;
    len--;
  }      

  if (cmd != ptr) {
    if (cmd == buf)
      QvwmError("too long remote command; command must be terminated by '\n'");
    else
      QvwmError("incomplete remote command is discarded");
  }
}

void RemoteCommand::interpret(char* cmd)
{
  FuncNumber *fn;
  char *ptr = cmd, *args=NULL;
  
  while (*ptr != '\0') {
    if (*ptr == ' ') {
      args = ptr + 1;
      break;
    }
    ptr++;
  }      

  *ptr = '\0';

  if ((fn = QvFunction::funcHashTable->GetHashItem(cmd)) == NULL) {
    QvwmError("invalid function name: '%s'", cmd);
    return;
  }

  switch (*fn) {
  case Q_NONE:
    return;

  case Q_EXIT:
    {
      Bool bakUseExitDialog = UseExitDialog;
      Bool bakUseConfirmDialog = UseConfirmDialog;

      // change to non-interructive mode
      UseExitDialog = False;
      UseConfirmDialog = False;

      QvFunction::execFunction(Q_EXIT);

      UseExitDialog = bakUseExitDialog;
      UseConfirmDialog = bakUseConfirmDialog;
    }
    break;

  case Q_RESTART:
    // window focus
  case Q_CHANGE_WINDOW:
  case Q_CHANGE_WINDOW_BACK:
  case Q_CHANGE_WINDOW_INSCR:
  case Q_CHANGE_WINDOW_BACK_INSCR:
  case Q_DESKTOP_FOCUS:
    // window rearrangement
  case Q_OVERLAP:
  case Q_OVERLAP_INSCR:
  case Q_TILE_HORZ:
  case Q_TILE_HORZ_INSCR:
  case Q_TILE_VERT:
  case Q_TILE_VERT_INSCR:
  case Q_MINIMIZE_ALL:
  case Q_MINIMIZE_ALL_INSCR:
  case Q_RESTORE_ALL:
  case Q_RESTORE_ALL_INSCR:
  case Q_CLOSE_ALL:
  case Q_CLOSE_ALL_INSCR:
    // menu
  case Q_POPUP_START_MENU:
  case Q_POPUP_DESKTOP_MENU:
  case Q_POPDOWN_ALL_MENU:
    // paging
  case Q_LEFT_PAGING:
  case Q_RIGHT_PAGING:
  case Q_UP_PAGING:
  case Q_DOWN_PAGING:
    // taskbar
  case Q_BOTTOM:
  case Q_TOP:
  case Q_LEFT:
  case Q_RIGHT:
  case Q_TOGGLE_AUTOHIDE:
  case Q_ENABLE_AUTOHIDE:
  case Q_DISABLE_AUTOHIDE:
  case Q_TOGGLE_TASKBAR:
  case Q_ENABLE_TASKBAR:
  case Q_DISABLE_TASKBAR:
  case Q_SHOW_TASKBAR:
  case Q_HIDE_TASKBAR:
    // pager
  case Q_TOGGLE_PAGER:
  case Q_ENABLE_PAGER:
  case Q_DISABLE_PAGER:
    // icon
  case Q_LINEUP_ICON:
    QvFunction::execFunction(*fn);
    break;

  case Q_MINIMIZE:
  case Q_MAXIMIZE:
  case Q_RESTORE:
  case Q_EXPAND:
  case Q_EXPAND_LEFT:
  case Q_EXPAND_RIGHT:
  case Q_EXPAND_UP:
  case Q_EXPAND_DOWN:
  case Q_RAISE:
  case Q_LOWER:
  case Q_CLOSE:
  case Q_KILL:
  case Q_TOGGLE_ONTOP:
  case Q_ENABLE_ONTOP:
  case Q_DISABLE_ONTOP:
  case Q_TOGGLE_STICKY:
  case Q_ENABLE_STICKY:
  case Q_DISABLE_STICKY:
  case Q_FOCUS:
  case Q_RAISE_FOCUS:
    {
	  List<Qvwm>* qvwmList = &desktop.GetQvwmList();
          List<Qvwm>::Iterator i(qvwmList);
          Qvwm* qvWm;
          int idx=0;
          XClassHint wm_class;        // STRING

          for (qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
                XGetClassHint(display, qvWm->GetWin(), &wm_class);
		if ((!strncmp(args, wm_class.res_name, strlen(args))) ||
                    (!strncmp(args, wm_class.res_class, strlen(args)))) {
    			QvFunction::execFunction(*fn, qvWm);
		}
          }
          break;
    }

  default:
    QvwmError("not supported command '%s' as a remote command", cmd);
    break;
  }
}

#endif // ALLOW_RMTCMD
