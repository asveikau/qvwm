/*
 * util.cc
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
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif
#ifndef HAVE_USLEEP
#include <sys/time.h>
#endif
#ifdef __EMX__
#include <process.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include "main.h"
#include "misc.h"
#include "qvwm.h"
#include "menu.h"
#include "util.h"
#include "qvwmrc.h"
#include "desktop.h"
#include "timer.h"
#include "callback.h"
#include "audio_wav.h"
#include "audio_au.h"
#include "exec.h"
#include "paging.h"
#include "pager.h"
#include "pixmap_image.h"
#ifdef USE_ANIM_IMAGE
#include "anim_image.h"
#endif
#ifdef USE_IMLIB
#include "extra_image.h"
#endif



/*
 * Gravity offset for frame decoration.
 */
Point GravityOffset[] = {
  Point(CENTER, CENTER),            // ForgetGravity
  Point(WEST,   NORTH),             // NorthWestGravity
  Point(CENTER, NORTH),             // NorthGravity
  Point(EAST,   NORTH),             // NorthEastGravity
  Point(WEST,   CENTER),            // WestGravity
  Point(CENTER, CENTER),            // CenterGravity
  Point(EAST,   CENTER),            // EastGravity
  Point(WEST,   SOUTH),             // SouthWestGravity
  Point(CENTER, SOUTH),             // SouthGravity
  Point(EAST,   SOUTH),             // SouthEastGravity
  Point(CENTER, CENTER),            // StaticGravity
};

List<ExecPending> ExecPending::m_pendingList;

/*
 * InRect --
 *   Return True if the point is in a given rectangle.
 */
Bool InRect(const Point& pt, const Rect& rc)
{
  if (pt.x < rc.x)
    return False;
  if (pt.x >= rc.x + rc.width)
    return False;
  if (pt.y < rc.y)
    return False;
  if (pt.y >= rc.y + rc.height)
    return False;

  return True;
}

/*
 * Intersect --
 *   Return True if two rectangles intersect each other.
 */
Bool Intersect(const Rect& rc1, const Rect& rc2)
{
  if (InRect(Point(rc1.x, rc1.y), rc2) ||
      InRect(Point(rc1.x + rc1.width - 1, rc1.y), rc2) ||
      InRect(Point(rc1.x, rc1.y + rc1.height - 1), rc2) ||
      InRect(Point(rc1.x + rc1.width - 1, rc1.y + rc1.height - 1), rc2) ||
      InRect(Point(rc2.x, rc2.y), rc1) ||
      InRect(Point(rc2.x + rc2.width - 1, rc2.y), rc1) ||
      InRect(Point(rc2.x, rc2.y + rc2.height - 1), rc1) ||
      InRect(Point(rc2.x + rc2.width - 1, rc2.y + rc2.height - 1), rc1))
    return True;

  return False;
}

/*
 * IsDoubleClick --
 *   Return True if the click is decided as double click from time and space.
 */
Bool IsDoubleClick(Time prevTime, Time clickTime, const Point& ptPrev,
		   const Point& ptClick)
{
  Rect rcPrev;

  if (clickTime - prevTime > (unsigned int)DoubleClickTime)
    return False;

  rcPrev = Rect(ptPrev.x-DoubleClickRange, ptPrev.y-DoubleClickRange,
		ptPrev.x+DoubleClickRange, ptPrev.y+DoubleClickRange);
  if (!InRect(ptClick, rcPrev))
    return False;

  return True;
}

/*
 * GetMenuItemNum --
 *   Return the number of elements in mItem.
 */
int GetMenuItemNum(const MenuElem* mItem)
{
  int num = 0;

  while (mItem) {
    num++;
    mItem = mItem->next;
  }

  return num;
}

QvImage* CreateImageFromFile(const char* file, Timer* timer)
{
  switch (file[0]) {
  case 0: 
    return NULL;

  case '/':
    return CreateImage(file, timer);

  case '~':
  case '$':
    {
      char* filename = ExtractPathName(file);
      return CreateImage(filename, timer);
    }

  default:
    {
      QvImage* img;
      char* paths = new char[strlen(ImagePath) + 1 + strlen(IMGDIR) + 1];
      int filelen = strlen(file);

      sprintf(paths, "%s:%s", ImagePath, IMGDIR);
      char* pathname = strtok(paths, ":");

      while (pathname) {
	char* realpath = ExtractPathName(pathname);
	int len = strlen(realpath) + 1 + filelen;
	char* filename = new char[len + 1];

	sprintf(filename, "%s/%s", realpath, file);
	delete [] realpath;

	img = CreateImage(filename, timer);

	delete [] filename;

	if (img) {
	  delete [] paths;
	  return img;
	}

	pathname = strtok(NULL, ":");
      }

      delete [] paths;
      QvwmError("Can't find image '%s'", file);
      return NULL;
    }
  }
}

QvImage* CreateImage(const char* filename, Timer* timer)
{
  QvImage* img;
  struct stat st;
  int len = strlen(filename);

#ifdef __EMX__
  if (stat(__XOS2RedirRoot(filename), &st) == 0) {
#else
  if (stat(filename, &st) == 0) {
#endif

    if (strcmp(&filename[len - 4], ".ani") == 0) 
#ifdef USE_ANIM_IMAGE
      img = new AnimImage(filename, timer);
#else
      QvwmError("Can't load image, animated image support disabled: %s", filename);
#endif
    else {
#ifdef USE_IMLIB
      img = new ExtraImage(filename);
#else
      if (strcmp(&filename[len - 4], ".xpm") == 0)
	img = new PixmapImage(filename);
      else {
	QvwmError("unsupported image file: %s", filename);
	img = NULL;
      }
#endif // USE_IMLIB
    }

    if (img && img->GetError()) {
      QvImage::Destroy(img);
      img = NULL;
    }

    return img;
  }

  return NULL;
}

/*
 * GetFixName --
 */
char* GetFixName(XFontSet& fs, const char* name, int width)
{
  const int dotWidth = 11;
  int len, curWidth;
  char *str;
  XRectangle ink, log;

  len = strlen(name);

  // str is enough short
  if ((curWidth = XmbTextExtents(fs, name, len, &ink, &log)) <= width){
    str = new char[len + 1];
    strcpy(str, name);
    return str;
  }

  // can't write even "..."
  if (width <= dotWidth) {
    str = new char[1];
    *str = '\0';
    return str;
  }

  if (curWidth < (width-dotWidth) * 2) {
    while (XmbTextExtents(fs, name, len, &ink, &log) > width - dotWidth)
      if (--len == 0) {
	str = new char[1];
	*str = '\0';
	return str;
      }
  }
  else {
    len = 0;
    while (XmbTextExtents(fs, name, len, &ink, &log) < width - dotWidth)
      len++;
    len--;
  }
  
  str = new char[len + 1];
  strncpy(str, name, len);
  str[len] = '\0';

  return str;
}

/*
 * MappedNotOverride --
 *   Return True if the window attribute doesn't include override_redirect.
 */
Bool MappedNotOverride(Window w, long* state)
{
  XWindowAttributes attr;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;
  unsigned char* prop;

  if(!XGetWindowAttributes(display, w, &attr))
    return False;

  *state = NormalState;

  if (XGetWindowProperty(display, w, _XA_WM_STATE, 0, 3, False, _XA_WM_STATE,
			 &atype, &aformat, &nitems, &bytes_remain, &prop)
      ==Success) {
    if (prop != NULL) {
      *state = *(long *)prop;
      XFree(prop);
    }
  }

  return (((*state == IconicState) || (attr.map_state != IsUnmapped))
	  && !attr.override_redirect);
}

GC CreateTileGC(Drawable d)
{
  static const unsigned char data[] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
  Pixmap pattern;
  GC gcTile;

  pattern = XCreateBitmapFromData(display, d, (char*)data, 8, 8);

  gcTile = XCreateGC(display, d, 0, 0);
  XSetFillStyle(display, gcTile, FillStippled);
  XSetStipple(display, gcTile, pattern);

  XFreePixmap(display, pattern);

  return gcTile;
}

/*
 * DarkenScreen --
 *   Darken whole screen.
 */
void DarkenScreen()
{
  GC gcDark;

  gcDark = CreateTileGC(root);

  XSetSubwindowMode(display, gcDark, IncludeInferiors);
  XSetForeground(display, gcDark, black.pixel);

  XFillRectangle(display, root, gcDark,
		 0, 0,
		 DisplayWidth(display, screen),
		 DisplayHeight(display, screen));

  XFreeGC(display, gcDark);
}

/*
 * RefreshScreen --
 *   Refresh whole screen.
 */
void RefreshScreen()
{
  Window w;

  w = XCreateSimpleWindow(display, root,
			  0, 0,
			  DisplayWidth(display, screen),
			  DisplayHeight(display, screen),
			  0, black.pixel, white.pixel);
  
  XMapWindow(display, w);
  XFlush(display);
  XDestroyWindow(display, w);
}

// tcsh does not work...
#define SHELL "/bin/sh"

pid_t ExecCommand(const char* exec)
{
  pid_t pid;

  if (strncmp(exec, "EXEC ", 5) == 0) {
    // XXX dangerous buffer limits:
    char cmd[256], *argv[16];
    int i = 1;

    strcpy(cmd, &exec[5]);
    argv[0] = strtok(cmd, " ");
    while ((argv[i] = strtok(NULL, " ")) != NULL)
      i++;

    desktop.FinishQvwm(True);
    execvp(cmd, argv);
  }
  else if (strncmp(exec, "PAGE", 4) == 0) {
    char cmd[256], *comp;  // XXX buffer allocation unsafe
    Point pageoff;
    
    if (exec[4] == '[') {
      char *next;

      strcpy(cmd, &exec[5]);
      
      // x page offset
      next = strtok(cmd, ",");
      if (next == NULL) {
	QvwmError("Syntax error: first ',' is missing");
	QvwmError("  %s", exec);
	return -1;
      }
      pageoff.x = atoi(next) * DisplayWidth(display, screen);

      // y page offset
      next = strtok(NULL, ",");
      if (next == NULL) {
	QvwmError("Syntax error: second ',' is missing");
	QvwmError("  %s", exec);
	return -1;
      }
      pageoff.y = atoi(next) * DisplayHeight(display, screen);

      // compare string
      comp = strtok(NULL, "]");
    }
    else if (strncmp(&exec[4], "CUR[", 4) == 0) {
      pageoff = paging->origin;
      strcpy(cmd, &exec[8]);
      comp = strtok(cmd, "]");
    }
    else {
      QvwmError("Syntax error: %s", exec);
      return -1;
    }

    if (comp == NULL) {
      QvwmError("Syntax error: compare string is missing");
      QvwmError("  %s", exec);
      return -1;
    }

    // eliminate spaces before the string
    while (*comp == ' ')
      comp++;

    // return if the compare string is not specified
    int len = strlen(comp);
    if (len == 0) {
      QvwmError("Syntax error: compare string is missing");
      QvwmError("  %s", exec);
      return -1;
    }

    // eliminate spaces after the compare string
    char* ptr = &comp[len - 1];
    while (*ptr == ' ' && ptr > comp)
      ptr--;
    *(ptr + 1) = '\0';

    ExecPending* pending = new ExecPending(pageoff, comp);
    ExecPending::m_pendingList.InsertTail(pending);

    const char* excmd = strchr(exec, ']');
    excmd++;
    while (*excmd == ' ')
      excmd++;
    if (*excmd == '\0') {
      QvwmError("Syntax error: external command is not specified");
      QvwmError("  %s", exec);
      return -1;
    }

    exec = excmd;
  }

  char cmd[1024];

  if (strlen(exec) + 5 >= sizeof(cmd)) {
    QvwmError("too long command string: '%s'", exec);
    return -1;
  }
  sprintf(cmd, "exec %s", exec);

  if (HourGlassTime > 0) {
    XDefineCursor(display, root, cursor[BGWORK]);

    timer->ResetTimeout(new GlobalCallback(&RestoreCursor));
    timer->SetTimeout(HourGlassTime, new GlobalCallback(&RestoreCursor));
  }

#ifdef __EMX__  
  if (spawnl(P_NOWAIT, "cmd.exe", "cmd.exe", "/C", exec, NULL) == -1)
    QvwmError("Can't execute '%s'", exec);
#else
  if ((pid = vfork()) == 0) {
    if (execl(SHELL, SHELL, "-c", cmd, NULL) == -1) {
      QvwmError("Can't execute '%s'", &cmd[5]);
      _exit(1);
    }
  }
#endif    

  return (pid);
}

void RestoreCursor()
{
  XDefineCursor(display, root, cursor[SYS]);
}

#define MAX_ENV_NAME 255
#define MAX_USER_NAME 32

/*
 * Extract ~ and environment variable in a path name.
 */
char* ExtractPathName(const char* name)
{
  char* exname;

  if (name == NULL)
    return NULL;

  if (name[0] == '~') {
    if (name[1] == '/' || name[1] == '\0') {
      const char* home = getenv("HOME");
      const char* dir = &name[1];

      if (home == NULL) {
	QvwmError("Undefined environment variable HOME\n");
	exname = new char[strlen(dir) + 1];
	strcpy(exname, dir);
      }
      else {
	exname = new char[strlen(home) + strlen(dir) + 1];
	sprintf(exname, "%s%s", home, dir);
      }
    }
    else {
      char user[MAX_USER_NAME + 1];
      const char* dir;

      dir = GetNextDelim(&name[1], user, MAX_USER_NAME + 1);
      struct passwd* pw = getpwnam(user);

      if (pw == NULL) {
	if (user[0] != '\0')
	  QvwmError("No user %s\n", user);
	exname = new char[strlen(name) + 1];
	strcpy(exname, name);
      }
      else {
	exname = new char[strlen(pw->pw_dir) + strlen(dir) + 1];
	sprintf(exname, "%s%s", pw->pw_dir, dir);
      }
    }
    return exname;
  }
  else if (name[0] == '$') {
    char env[MAX_ENV_NAME + 1];
    const char* dir;
    
    dir = GetNextDelim(&name[1], env, MAX_ENV_NAME + 1);
    char* exenv = getenv(env);

    if (exenv == NULL) {
      if (env[0] != '\0')
	QvwmError("No environment variable %s\n", env);
      exname = new char[strlen(dir) + 1];
      strcpy(exname, dir);
    }
    else {
      exname = new char[strlen(exenv) + strlen(dir) + 1];
      sprintf(exname, "%s%s", exenv, dir);
    }

    return exname;
  }

  exname = new char[strlen(name) + 1];
  strcpy(exname, name);

  return exname;
}

// maxSize includes null for termination
const char* GetNextDelim(const char* path, char* name, int maxSize)
{
  int i;

  for (i = 0; path[i] != '/' && path[i] != '\0'; i++) {
    if (i >= maxSize - 1) {
      name[0] = '\0';
      QvwmError("Too long before delim: %s\n", path);
      return path;
    }
    name[i] = path[i];
  }
  name[i] = '\0';

  return &path[i];
}

#if !defined(HAVE_USLEEP) && !defined(__EMX__)

#if defined(sun) && !defined(__SVR4)
extern "C" int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif

void usleep(unsigned long usec)
{
  struct timeval tm;
  
  if (usec <= 0)
    return;
  
  tm.tv_usec = usec % 1000000;
  tm.tv_sec = usec / 1000000;
  
  select(1, NULL, NULL, NULL, &tm);
}
#endif // HAVE_USLEEP

// Get string width excluding "\B" for bold font and "\&" for underscore
int GetRealWidth(XFontSet fs, const char* str)
{
  XRectangle ink, log;
  unsigned int width;

  if (*str == '\\' && *(str + 1) == 'B')
    str += 2;

  const char* keyStr = strstr(str, "\\&");
  if (keyStr == NULL)
    width = XmbTextExtents(fs, str, strlen(str), &ink, &log);
  else {
    // length before "\&"
    width = XmbTextExtents(fs, str, keyStr - str, &ink, &log);
    // length after "\&"
    width += XmbTextExtents(fs, keyStr + 2, str + strlen(str) - (keyStr + 2),
			    &ink, &log);
  }

  return width;
}

// Draw "...\&?..." -> "...?..."
//                         ~
void DrawRealString(Window w, XFontSet fs, GC gc, int x, int y, const char* str)
{
  Bool bold = False;

  if (*str == '\\' && *(str + 1) == 'B') {
    str += 2;
    bold = True;
  }

  const char* keyStr = strstr(str, "\\&");
  XRectangle ink, log;
  int len = strlen(str);

  if (keyStr == NULL) {
    XmbDrawString(display, w, fs, gc, x, y, str, len);
    if (bold)
      XmbDrawString(display, w, fs, gc, x, y + 1, str, len);
  }
  else {
    // draw "..."
    XmbDrawString(display, w, fs, gc, x, y, str, keyStr - str);
    if (bold)
      XmbDrawString(display, w, fs, gc, x, y + 1, str, keyStr - str);

    // draw "?..."
    XmbTextExtents(fs, str, keyStr - str, &ink, &log);
    x += log.width;
    XmbDrawString(display, w, fs, gc, x, y,
		  keyStr + 2, str + len - (keyStr + 2));
    if (bold)
      XmbDrawString(display, w, fs, gc, x, y + 1,
		    keyStr + 2, str + len - (keyStr + 2));
    
    // draw underscore
    XmbTextExtents(fs, keyStr + 2, 1, &ink, &log);
    XDrawLine(display, w, gc, x, y + 1, x + log.width, y + 1);
  }
}

#define AUDIODEV "/dev/audio"

void PlaySound(const char* file, Bool sync)
{
  if (file == NULL || file[0] == '\0')
    return;
  
  if (!EnableSound)
    return;

#ifndef __EMX__
  if (!sync) {
    pid_t pid = fork();
    if (pid)
      return;
  }

  Audio* audio;
  int filelen = strlen(file);
  int ret;
    
  if (filelen > 4 && strcmp(&file[filelen - 4], ".wav") == 0)
    audio = new AudioWav(AUDIODEV);
  else if (filelen > 3 && strcmp(&file[filelen - 3], ".au") == 0)
    audio = new AudioAu(AUDIODEV);
  else {
    QvwmError("unsupported extentions of sound file: %s", file);
    if (sync)
      return;
    else
      exit(0);
  }
    
  // EnableSound may be set to False in Audio constructor
  if (!EnableSound) {
    delete audio;
    if (sync)
      return;
    else
      exit(0);
  }

  switch (file[0]) {
  case '/':
    ret = audio->Play(file);
    break;

  case '~':
  case '$':
    {
      char* filename = ExtractPathName(file);
      ret = audio->Play(filename);
      delete [] filename;
    }
    break;

  default:
    {
      char* paths = new char[strlen(SoundPath) + 1 + strlen(SNDDIR) + 1];

      sprintf(paths, "%s:%s", SoundPath, SNDDIR);
      char* pathname = strtok(paths, ":");

      while (pathname) {
	char* realpath = ExtractPathName(pathname);
	int len = strlen(realpath) + 1 + filelen;
	char* filename = new char[len + 1];

	sprintf(filename, "%s/%s", realpath, file);
	delete [] realpath;

	ret = audio->Play(filename);

	delete [] filename;

	if (ret == AUDIO_FATAL) {
	  /* do not try audio after that */
	  EnableSound = False;
	  break;
	}
	if (ret != AUDIO_NOFILE)
	  break;

	pathname = strtok(NULL, ":");
      }

      delete [] paths;
    }
    break;
  }

  if (ret == AUDIO_NOFILE)
    QvwmError("not found sound file: '%s'", file);

  delete audio;
  
  if (!sync)
    exit(0);
#endif // __EMX__
}

Bool IsPointerInWindow(const Point& ptRoot)
{
  List<Qvwm>::Iterator i(&desktop.GetQvwmList());
  
  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
    Rect rcReal = qvWm->GetRect();
    rcReal.x -= paging->origin.x;
    rcReal.y -= paging->origin.y;

    if (InRect(ptRoot, rcReal))
      return True;
  }

  if (UseTaskbar) {
    if (InRect(ptRoot, taskBar->GetRect()))
      return True;
  }

  if (UsePager) {
    if (InRect(ptRoot, pager->GetRect()))
      return True;
  }

  return False;
}

/*
 * CreateColor --
 *   Allocate color from red, green and blue.
 */
int CreateColor(unsigned short red, unsigned short green,
		unsigned short blue, XColor* color, XColor* substitute,
		const char* comment)
{
  int status;

  color->red = red;
  color->green = green;
  color->blue = blue;

  status = XAllocColor(display, colormap, color);
  if (status == 0) {
    if (comment)
      QvwmError("cannot allocate color: %s", comment);

    if (substitute)
      *color = *substitute;
    else
      color->pixel = 0;
  }

  return status;
}

// require is a set of indispensable parameters
Bool SetGeometry(char* str, InternGeom* geom)
{
  if (geom == NULL)
    return False;

  // initialize
  geom->rc = Rect(0, 0, 0, 0);

  geom->bitmask = XParseGeometry(str, &geom->rc.x, &geom->rc.y,
				 (unsigned int *)&geom->rc.width,
				 (unsigned int *)&geom->rc.height);


  if (geom->bitmask & XNegative)    // EastGravity
    geom->gravity.x = EAST;
  else if (geom->bitmask & XValue)  // WestGravity
    geom->gravity.x = WEST;
  else                              // CenterGravity
    geom->gravity.x = CENTER;

  if (geom->bitmask & YNegative)    // SouthGravity
    geom->gravity.y = SOUTH;
  else if (geom->bitmask & YValue)  // NorthGravity
    geom->gravity.y = NORTH;
  else                              // CenterGravity
    geom->gravity.y = CENTER;

  return True;
}

void SetDisplayEnv(const char* displayName)
{
  static char envStr[256];

  snprintf(envStr, 256, "DISPLAY=%s", displayName);

  putenv(envStr);
}

int GetXColorFromName(const char* str, XColor* color)
{
  if (strcmp(str, "qvgray") == 0)
    *color = gray;
  else if (strcmp(str, "qvdarkgray") == 0)
    *color = darkGray;
  else if (strcmp(str, "qvgrey") == 0)
    *color = grey;
  else if (strcmp(str, "qvdarkgrey") == 0)
    *color = darkGrey;
  else if (strcmp(str, "qvblue") == 0)
    *color = blue;
  else if (strcmp(str, "qvlightblue") == 0)
    *color = lightBlue;
  else if (strcmp(str, "qvroyalblue") == 0)
    *color = royalBlue;
  else if (strcmp(str, "qvyellow") == 0)
    *color = yellow;
  else if (strcmp(str, "qvlightyellow") == 0)
    *color = lightYellow;
  else if (strcmp(str, "qvgray95") == 0) {
    if (gray95.pixel == 0)
      CreateColor(0xbefb, 0xbefb, 0xbefb, &gray95, &white, "qvgray95");
    *color = gray95;
  }
  else if (strcmp(str, "qvdarkgray95") == 0) {
    if (darkGray95.pixel == 0)
      CreateColor(0x79e7, 0x7df7, 0x79e7, &darkGray95, &black,
		  "qvdarkgray95");
    *color = darkGray95;
  }
  else if (strcmp(str, "qvlightgray95") == 0) {
    if (lightGray95.pixel == 0)
      CreateColor(0xdf7d, 0xdf7d, 0xdf7d, &lightGray95, &gray95,
		  "qvlightgray95");
    *color = lightGray95;
  }
  else if (strcmp(str, "qvgrey95") == 0) { // not qvgray!
    if (grey95.pixel == 0)
      CreateColor(0xb6da, 0xb2ca, 0xb6da, &grey95, &gray95, "qvgrey95");
    *color = grey95;
  }
  else if (strcmp(str, "qvblue95") == 0) {
    if (blue95.pixel == 0)
      CreateColor(0x0000, 0x0000, 0x79e7, &blue95, &black, "qvblue95");
    *color = blue95;
  }
  else if (strcmp(str, "qvlightblue95") == 0) {
    if (lightBlue95.pixel == 0)
      CreateColor(0x0820, 0x8207, 0xcf3c, &lightBlue95, &blue95,
		  "qvlightblue95");
    *color = lightBlue95;
  }
  else if (strcmp(str, "qvgreen95") == 0) {
    if (green95.pixel == 0)
      CreateColor(0x0000, 0x7df7, 0x79e7, &green95, &blue95,
		  "qvgreen95");
    *color = green95;
  }
  else if (strcmp(str, "qvyellow95") == 0) {
    if (yellow95.pixel == 0)
      CreateColor(0xffff, 0xffff, 0xdf7d, &yellow95, &white,
		  "qvyellow95");
    *color = yellow95;
  }
  else if (XParseColor(display, colormap, str, color) != 0)
    XAllocColor(display, colormap, color);
  else
    return -1;

  return 0;
}
