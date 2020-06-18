/*
 * debug.cc
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include "main.h"
#include "debug.h"
#ifdef __EMX__
#include <io.h>
#include <string.h>
#endif

#if defined(sun) && !defined(__SVR4)
extern "C" int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
extern "C" int bzero(char *, int);
#endif

Bool Debug::stackTraceDone;

#define PIPE_READ  0
#define PIPE_WRITE 1

void Debug::TraceStack(char* debugger, char* program)
{
  char buf[16];
  char *args[4];

  sprintf(buf, "%d", (int)getpid());

  args[0] = debugger;
  args[1] = program;
  args[2] = buf;
  args[3] = NULL;

  /*
   * Create dual pipe.
   */
  int fdQvwmToDbg[2], fdDbgToQvwm[2];

  if (pipe(fdQvwmToDbg) == -1 || pipe(fdDbgToQvwm) == -1) {
    QvwmError("Can't open pipe");
    return;
  }

  stackTraceDone = False;
  signal(SIGCHLD, TraceStackSigChld);

  pid_t pid;

  if ((pid = fork()) == -1) {
    QvwmError("Can't fork process");
    return;
  }
  else if (pid == 0) {
    /*
     * Redirect stdin/stdout/stderr to pipe.
     */
    close(0);
    dup(fdQvwmToDbg[PIPE_READ]);  // stdin
    close(1);
    dup(fdDbgToQvwm[PIPE_WRITE]);  // stdout
    close(2);
    dup(fdDbgToQvwm[PIPE_WRITE]);  // stderr

    /*
     * Execute the debugger.
     */
    if (execvp(args[0], args) == -1) {
      char str[256];
      sprintf(str, "execv: %s", args[0]);
      perror(str);
    }
    exit(0);
  }

  write(fdQvwmToDbg[PIPE_WRITE], "backtrace\n", 10);
  write(fdQvwmToDbg[PIPE_WRITE], "quit\n", 5);

  while (1) {
    fd_set fds;
    struct timeval tm;
    int s;
    char c;

    FD_ZERO(&fds);
    FD_SET(fdDbgToQvwm[PIPE_READ], &fds);

    tm.tv_sec = 1;
    tm.tv_usec = 0;

#if defined(__hpux) && !defined(_XPG4_EXTENDED)
    s = select(FD_SETSIZE, (int *)&fds, NULL, NULL, &tm);
#else
    s = select(FD_SETSIZE, &fds, NULL, NULL, &tm);
#endif
    if (s <= 0) {
      if (stackTraceDone)
	break;
    }
    else {
      if (FD_ISSET(fdDbgToQvwm[PIPE_READ], &fds))
	if (read(fdDbgToQvwm[PIPE_READ], &c, 1))
	  fprintf(stdout, "%c", c);
    }
  }

  close(fdQvwmToDbg[PIPE_READ]);
  close(fdQvwmToDbg[PIPE_WRITE]);
  close(fdDbgToQvwm[PIPE_READ]);
  close(fdDbgToQvwm[PIPE_WRITE]);

  return;
}

void Debug::TraceStackSigChld(int sig)
{
  stackTraceDone = True;

  wait(NULL);
}
