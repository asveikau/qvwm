/*
 * remote_cmd.h
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

#ifndef REMOTE_CMD_H_
#define REMOTE_CMD_H_

#ifdef ALLOW_RMTCMD

#include <stdio.h>

class RemoteCommand {
private:
  char* m_fifoName;
  int m_fd;

private:
  void interpret(char* cmd);

public:
  RemoteCommand();
  ~RemoteCommand();

  int getRmtCmdFd() const { return m_fd; }

  void process();
};

#endif // REMOTE_CMD_H_

#endif // ALLOW_RMTCMD
