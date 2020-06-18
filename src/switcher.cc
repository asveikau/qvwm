/*
 * switcher.cc
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "main.h"
#include "qvwm.h"
#include "event.h"
#include "switcher.h"
#include "qvwmrc.h"
#include "timer.h"
#include "key.h"
#include "focus_mgr.h"
#include "timer.h"

Timer* TaskSwitcher::swTimer;

#define TS_ITEM_HEIGHT 36
#define TS_ITEM_MARGIN 4
#define TS_BORDER 6
#define TS_ITEM_WIDTH 316
#define TS_WIDTH (TS_ITEM_WIDTH+TS_BORDER*2)
#define TS_ITEM_TEXT_LEFT_POSITION 36


TaskSwitcher::TaskSwitcher()
{
  XSetWindowAttributes attributes;
  unsigned long valueMask;
  
  /*
   * Create frame window.
   */
  attributes.background_pixel = SwitcherColor.pixel;
  attributes.override_redirect = True;
  attributes.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask;
  valueMask = CWBackPixel | CWOverrideRedirect | CWEventMask;

  frame = XCreateWindow(display, root, 0, 0, 1, 1,
			0, CopyFromParent, InputOutput, CopyFromParent,
			valueMask, &attributes);

  valueMask = CWBackPixel;


  if (SwitcherImage) {
    imgSwitcher = CreateImageFromFile(SwitcherImage, swTimer);
    if (imgSwitcher) {
      imgSwitcher->SetBackground(frame);
      imgTitle = NULL;
    }
    else {
      delete [] SwitcherImage;
      SwitcherImage = NULL;
    }
  }

  qvWm = NULL;
  winNum = 0;
  iterMap = new List<Qvwm>::Iterator(&focusMgr.GetMapList());
  iterUnmap = new List<Qvwm>::Iterator(&focusMgr.GetUnmapList());
}

TaskSwitcher::~TaskSwitcher()
{
  
  XDestroySubwindows(display, frame);
  XDestroyWindow(display, frame);

  if (SwitcherImage) {
    QvImage::Destroy(imgSwitcher);
    QvImage::Destroy(imgTitle);
  }

  delete iterMap;
  delete iterUnmap; 
}

void TaskSwitcher::MapSwitcher( )
{

  int height = winNum * TS_ITEM_HEIGHT + TS_BORDER*2;

  itemwin = new Window[winNum];

  ASSERT(rootQvwm);
  Rect rcRoot = rootQvwm->GetRect();

  rc = Rect((rcRoot.width - TS_WIDTH) / 2,
	    (rcRoot.height - height) / 2,
	    TS_WIDTH, height);

  int rootHt = rcRoot.height;
  if ( height > rootHt ) {
    // problem: window is too high to be fully visible on the screen!
    rc.y = TS_ITEM_HEIGHT;
    scroll = True;
    scroll_to = new int[winNum];
  } else {
    scroll = False;
  }

  XMoveResizeWindow(display, frame, rc.x, rc.y, rc.width, rc.height);

  // create a window for each task item (as in menu)
  XSetWindowAttributes attributes;
  unsigned long valueMask;

  attributes.background_pixel = SwitcherColor.pixel;
  attributes.override_redirect = True;
  attributes.override_redirect = False;
  attributes.event_mask = ButtonPressMask | ButtonReleaseMask;
  valueMask = CWBackPixel | CWEventMask;

  int last_y = TS_BORDER;

  for ( int i=0; i < winNum; i++ ) {
    itemwin[i] = XCreateWindow(display, frame, 
                               TS_BORDER, last_y, TS_ITEM_WIDTH, TS_ITEM_HEIGHT,
                               0, CopyFromParent, InputOutput, CopyFromParent,
                               valueMask, &attributes);
    XMapRaised(display, itemwin[i]);

    last_y += TS_ITEM_HEIGHT;

    if ( scroll ) {
      if ( last_y < rootHt/2 )
        scroll_to[i] = TS_ITEM_HEIGHT;
      else 
        scroll_to[i] = 0 - (last_y - rootHt/2) + TS_ITEM_HEIGHT/2;
    }

  }

  XMapRaised(display, frame);
}


void TaskSwitcher::Unmap () {
  XUnmapWindow(display, frame);
  for ( int i=0; i < winNum; i++ ) {
    XUnmapWindow( display, itemwin[i] );
    XDestroyWindow( display, itemwin[i]);
  }
  delete [] itemwin;
}


/*
 * SwitchTask --
 *   Display task switcher and switch focus window.
 *   This routine must be called by modifier + code.
 *   mod is with LockMask.
 */
void TaskSwitcher::SwitchTask(Bool forward, KeyCode code, unsigned int mod)
{
  winNum = focusMgr.GetAllNum();
  
  if (winNum == 0)
    return;
  else if (winNum == 1) {
    qvWm = focusMgr.Top();
    if (qvWm == NULL)
      qvWm = focusMgr.TopUnmapList();

    qvWm->SetFocus();
    if (qvWm->CheckMapped())
      qvWm->RaiseWindow(True);
    else
      qvWm->RestoreWindow();
    qvWm->AdjustPage();
    return;
  }

  direct = forward;

  MapSwitcher();
  XSetInputFocus(display, frame, RevertToParent, CurrentTime);

  if ( !qvWm ) {
    qvWm = GetFirstFocus();
    if (Qvwm::focusQvwm == rootQvwm)
      focusWin = 0;
    else
      focusWin = direct ? 1 : winNum - 1;
  }

  EventLoop(code, mod);
}

void TaskSwitcher::EventLoop( KeyCode code, unsigned int mod )
{

  XEvent ev;
  int fd = ConnectionNumber(display);
  fd_set fds;
  struct timeval tm;
  KeySym released;

  KeySym release_key[2];
  if (mod & ShiftMask) {
    release_key[0] = XK_Shift_L;
    release_key[1] = XK_Shift_R;
    
  } else if (mod & ControlMask) {
    release_key[0] = XK_Control_L;
    release_key[1] = XK_Control_R;

  } else if (mod & ShortCutKey::getAltMask()) {
    release_key[0] = XK_Alt_L;
    release_key[1] = XK_Alt_R;
    
  } else if (mod & ShortCutKey::getMetaMask()) {
    release_key[0] = XK_Meta_L;
    release_key[1] = XK_Meta_R;

  } else {
    release_key[0] = release_key[1] = 0;
  }

//  QvwmError( "EventLoop: Code %d, mod %d", code, mod );
  

  while (1) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    while (XPending(display) != 0) {
      XNextEvent(display, &ev);

      // check if the event that has arrived is for us or not
      Bool ourevent = False;
      XAnyEvent *any = (XAnyEvent*) &ev;
      if ( any->window == frame ) {
        ourevent = True;

      } else {
        // are we the parent of the destination window?
        Window _root, parent, *childrenw = NULL;
        unsigned int childrennum = 0;
        XQueryTree( display, any->window, &_root, &parent, &childrenw, &childrennum );
        if ( parent == frame ) { 
          ourevent = True; 
        }
        if ( childrennum && childrenw ) { XFree( childrenw ); }
      }

/*
// debugging
      if ( ourevent ) {
        QvwmError( "Event receiver: switcher (%d, t:%d)",       any->window, any->type );
      } else {
        QvwmError( "Event receiver: another window (%d, t:%d)", any->window, any->type );        
      }
*/

      switch (ev.type) {
      case Expose:
	DrawTaskSwitcher();
	ASSERT(qvWm);
        DrawSwitcherContents( focusWin );
    
        while (XCheckWindowEvent(display, ev.xexpose.window, ExposureMask,&ev))
          ;
	break;
    
      case KeyPress:
	if (ev.xkey.keycode == code && ev.xkey.state == mod) {
          DrawItem( focusWin, qvWm, False );          
	  qvWm = direct ? GetNextFocus() : GetPrevFocus();
	  ASSERT(qvWm);
	  
	  if (direct) {
	    focusWin++;
	    if (focusWin == winNum)
	      focusWin = 0;
	  }
	  else {
	    focusWin--;
	    if (focusWin == -1)
	      focusWin = winNum - 1;
	  }
          DrawItem( focusWin, qvWm, True );          
	}

	if (XKeycodeToKeysym(display, ev.xkey.keycode, 0) == XK_Escape) {
          // quit without moving focus
          Unmap();
  
	  if (ClickToFocus)
	    XSetInputFocus(display, qvWm->GetWin(), RevertToParent,
			   CurrentTime);
	  else
	    XSetInputFocus(display, root, RevertToParent, CurrentTime);
          qvWm = NULL;
	  return;
	}

	if ( XKeycodeToKeysym(display, ev.xkey.keycode, 0) == XK_Return ) {
          SwitchToAndUnmap();
          return;
        }

	break;


      case KeyRelease:
        released = XKeycodeToKeysym(display, ev.xkey.keycode, 0);

        if ( released == release_key[0] || released == release_key[1] ) {
          // ok! switch the task, unmap and exit
          SwitchToAndUnmap();
          return;

        } else {
          // do nothing
          break;
        }
     
      case CreateNotify:
      case DestroyNotify:
        if ( !ourevent ) {
          int newWinNum = focusMgr.GetAllNum();

          event.DispatchEvent( ev );

          if ( newWinNum != winNum ) {
            // should restart, to reflect the changes in the list of windows / applications
            // this is not perfect solution XXX
            Restart( code, mod );
            return;
          }
        }
        break;

      default:
        if ( !ourevent ) {
          event.DispatchEvent( ev );
        }
        break;

      }

      swTimer->CheckTimeout(&tm);
    }
  
    if (!swTimer->CheckTimeout(&tm)) {
      tm.tv_sec = 1;
      tm.tv_usec = 0;
	
      XFlush(display);
    }
  
#if defined(__hpux) && !defined(_XPG4_EXTENDED)
    if (select(fd + 1, (int *)&fds, 0, 0, &tm) == 0) {    // timeout
#else
    if (select(fd + 1,        &fds, 0, 0, &tm) == 0) {    // timeout
#endif
      swTimer->CheckTimeout(&tm);
    }
  }
}


/*
 *  Find the application (qvWm) in the focus stack.
 */
Bool TaskSwitcher::FindApplication( Qvwm* qvWm ) {

    List<Qvwm>::Iterator *iterator;
    iterator = iterMap;

    int num = 0;
    for ( int pass = 0; pass < 2; pass++ ) {

      Qvwm *item = iterator ->GetHead();
      while ( item ) {
        if ( item == qvWm ) {
          focusWin = num;
          return True;
        }
        num ++;
        item = iterator ->GetNext();
      }

      if ( pass == 1 ) {
        iterator = iterUnmap;
      }
    }   

    iterMap -> GetHead();
    return False;
}


/*
 *  Switch to the user-chosen task and quit. 
 */
 
void TaskSwitcher::SwitchToAndUnmap() {
   Unmap();
      
   if (qvWm->CheckMapped()) {
     qvWm->SetFocus();
     /*
      * If a new active window has already have a focus, XSetInputFocus
      * doesn't call and the window with a mouse pointer gets an input
      * focus.  To avoid this problem, qvwm calls XSetInputFocus below.
      */
     XSetInputFocus(display, qvWm->GetWin(), RevertToParent, CurrentTime);
     qvWm->RaiseWindow(True);

   } else {
     qvWm->RestoreWindow();
   }

   qvWm->AdjustPage();
   qvWm = NULL;
}


/*
 *  Restart the task switcher.
 */
void TaskSwitcher::Restart( KeyCode code, unsigned int mod ) {
    Unmap();
    winNum = focusMgr.GetAllNum();
    delete iterMap;
    delete iterUnmap;
    iterMap   = new List<Qvwm>::Iterator(&focusMgr.GetMapList());
    iterUnmap = new List<Qvwm>::Iterator(&focusMgr.GetUnmapList());

    if ( !FindApplication( qvWm ) ) {
      qvWm = NULL;
    }
        
    SwitchTask( direct, code, mod );
}




/*
 * Called firstly once
 */
Qvwm* TaskSwitcher::GetFirstFocus()
{
  Qvwm* qvWm;

  qvWm = iterMap->GetHead();
  if (qvWm) {
    LookMapList = True;
    if (Qvwm::focusQvwm == qvWm)
      qvWm = direct ? GetNextFocus() : GetPrevFocus();
  }
  else {
    qvWm = iterUnmap->GetHead();
    ASSERT(qvWm);
    LookMapList = False;
  }

  return qvWm;
}

/*
 * Get next window in the focus stack, including unmapped windows
 */
Qvwm* TaskSwitcher::GetNextFocus()
{
  Qvwm* qvWm;

  if (LookMapList) {
    qvWm = iterMap->GetNext();
    if (qvWm == NULL) {
      qvWm = iterUnmap->GetHead();
      if (qvWm)
	LookMapList = False;
      else {
	qvWm = iterMap->GetHead();
	ASSERT(qvWm);
      }
    }
  }
  else {
    qvWm = iterUnmap->GetNext();
    if (qvWm == NULL) {
      qvWm = iterMap->GetHead();
      if (qvWm)
	LookMapList = True;
      else {
	qvWm = iterUnmap->GetHead();
	ASSERT(qvWm);
      }
    }
  }

  return qvWm;
}

/*
 * Get prev window in the focus stack, including unmapped windows
 */
Qvwm* TaskSwitcher::GetPrevFocus()
{
  Qvwm* qvWm;

  if (LookMapList) {
    qvWm = iterMap->GetPrev();
    if (qvWm == NULL) {
      qvWm = iterUnmap->GetTail();
      if (qvWm)
	LookMapList = False;
      else {
	qvWm = iterMap->GetTail();
	ASSERT(qvWm);
      }
    }
  }
  else {
    qvWm = iterUnmap->GetPrev();
    if (qvWm == NULL) {
      qvWm = iterMap->GetTail();
      if (qvWm)
	LookMapList = True;
      else {
	qvWm = iterUnmap->GetTail();
	ASSERT(qvWm);
      }
    }
  }

  return qvWm;
}




/*
 * Draw Switcher Contents --
 *   items of the task menu
 */
void TaskSwitcher::DrawSwitcherContents( int current )
{
  int num = 0;

  // draw windows in the map list
  List<Qvwm>::Iterator i(&focusMgr.GetMapList());

  for (Qvwm* qvWm = i.GetHead(); qvWm; qvWm = i.GetNext()) {
    ASSERT(num < winNum);
    DrawItem( num, qvWm, (num == current) ? True : False ); 
    num++;
  }

  // draw windows in the unmap list
  List<Qvwm>::Iterator j(&focusMgr.GetUnmapList());

  for (Qvwm* qvWm = j.GetHead(); qvWm; qvWm = j.GetNext()) {
    ASSERT(num < winNum);
    DrawItem( num, qvWm, (num == current) ? True : False ); 
    num++;
  }

  return;
}

/*
 * DrawTaskSwitcher --
 *   Draw task switcher.
 */
void TaskSwitcher::DrawTaskSwitcher()
{
  XPoint xp[3];
  
  xp[0].x = rc.width - 2;
  xp[0].y = 0;
  xp[1].x = 0;
  xp[1].y = 0;
  xp[2].x = 0;
  xp[2].y = rc.height - 2;
  
  XSetForeground(display, gc, gray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 1;
  xp[0].y = 0;
  xp[1].x = rc.width - 1;
  xp[1].y = rc.height - 1;
  xp[2].x = 0;
  xp[2].y = rc.height - 1;
  
  XSetForeground(display, gc, darkGrey.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 3;
  xp[0].y = 1;
  xp[1].x = 1;
  xp[1].y = 1;
  xp[2].x = 1;
  xp[2].y = rc.height - 3;
  
  XSetForeground(display, gc, white.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
  
  xp[0].x = rc.width - 2;
  xp[0].y = 1;
  xp[1].x = rc.width - 2;
  xp[1].y = rc.height - 2;
  xp[2].x = 1;
  xp[2].y = rc.height - 2;
  
  XSetForeground(display, gc, darkGray.pixel);
  XDrawLines(display, frame, gc, xp, 3, CoordModeOrigin);
  
}


/*
 * Draw Item --
 */
void TaskSwitcher::DrawItem( int num, Qvwm* qvWm, Bool chosen )
{

  ASSERT( qvWm );
  const char* name;
  Point pt;


  XRectangle ink, log;

  unsigned long background;
  unsigned long foreground;
  
  if ( chosen ) {
    background = MenuActiveColor.pixel;
    foreground = MenuStringActiveColor.pixel;
  } else {
    background = SwitcherColor.pixel;
    foreground = MenuStringColor.pixel;
  }

  XSetWindowBackground(display, itemwin[num], background);
  XClearWindow(display, itemwin[num]);

  name = qvWm -> name;
  XmbTextExtents(fsTitle, name, strlen(name), &ink, &log);
  pt.x = TS_ITEM_TEXT_LEFT_POSITION;
  pt.y = (TS_ITEM_HEIGHT - log.height) / 2 - log.y;

  XSetForeground(display, gc, foreground);
  XmbDrawString(display, itemwin[num], fsTitle, gc, pt.x, pt.y, name, strlen(name));


  // draw the icon
  QvImage* img = qvWm->imgLarge;  // use only here (not need Duplicate)

  Dim size = img-> GetSize();
  
  pt.x = ( TS_ITEM_TEXT_LEFT_POSITION - 2 - size.width ) / 2 + 1;
  pt.y = ( TS_ITEM_HEIGHT - size.height ) / 2;

  img->Display(itemwin[num], pt);

  if ( chosen && scroll ) {
    if ( rc.y != scroll_to[num] ) {
      rc.y = scroll_to[num];
      XMoveWindow(display, frame, rc.x, rc.y );
    }
  }

}




void TaskSwitcher::Initialize()
{
  swTimer = new Timer();
}
