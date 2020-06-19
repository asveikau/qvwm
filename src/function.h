#ifndef FUNCTION_H_
#define FUNCTION_H_

enum FuncNumber {
  Q_NONE,
  // qvwm
  Q_EXIT,
  Q_RESTART,
  // window
  Q_MINIMIZE,
  Q_MAXIMIZE,
  Q_MAXIMIZE_ONEWAY,
  Q_RESTORE,
  Q_EXPAND,
  Q_EXPAND_LEFT,
  Q_EXPAND_RIGHT,
  Q_EXPAND_UP,
  Q_EXPAND_DOWN,
  Q_MOVE,
  Q_RESIZE,
  Q_RAISE,
  Q_LOWER,
  Q_CLOSE,
  Q_KILL,
  Q_TOGGLE_ONTOP,
  Q_ENABLE_ONTOP,
  Q_DISABLE_ONTOP,
  Q_TOGGLE_STICKY,
  Q_ENABLE_STICKY,
  Q_DISABLE_STICKY,
  Q_TOGGLE_FOCUS,
  Q_TOGGLE_BORDER,
  Q_TOGGLE_BORDER_EDGE,
  Q_TOGGLE_BUTTON1,
  Q_TOGGLE_BUTTON2,
  Q_TOGGLE_BUTTON3,
  Q_TOGGLE_CTRLBTN,
  Q_TOGGLE_TBUTTON,
  Q_TOGGLE_TITLE,
  // window focus
  Q_SWITCH_TASK,
  Q_SWITCH_TASK_BACK,
  Q_CHANGE_WINDOW,
  Q_CHANGE_WINDOW_BACK,
  Q_CHANGE_WINDOW_INSCR,
  Q_CHANGE_WINDOW_BACK_INSCR,
  Q_DESKTOP_FOCUS,
  Q_FOCUS,
  Q_RAISE_FOCUS,
  // window rearrangement
  Q_OVERLAP,
  Q_OVERLAP_INSCR,
  Q_TILE_HORZ,
  Q_TILE_HORZ_INSCR,
  Q_TILE_VERT,
  Q_TILE_VERT_INSCR,
  Q_MINIMIZE_ALL,
  Q_MINIMIZE_ALL_INSCR,
  Q_RESTORE_ALL,
  Q_RESTORE_ALL_INSCR,
  Q_CLOSE_ALL,
  Q_CLOSE_ALL_INSCR,
  // menu
  Q_POPUP_START_MENU,
  Q_POPUP_DESKTOP_MENU,
  Q_POPUP_MENU,
  Q_POPDOWN_MENU,
  Q_POPDOWN_ALL_MENU,
  Q_UP_FOCUS,
  Q_DOWN_FOCUS,
  Q_RIGHT_FOCUS,
  Q_LEFT_FOCUS,
  // menu item
  Q_SEPARATOR,
  Q_DIR,
  Q_EXEC,
  Q_ACTION,
  // paging
  Q_LEFT_PAGING,
  Q_RIGHT_PAGING,
  Q_UP_PAGING,
  Q_DOWN_PAGING,
  // taskbar
  Q_BOTTOM,
  Q_TOP,
  Q_LEFT,
  Q_RIGHT,
  Q_TOGGLE_TASKBAR,
  Q_ENABLE_TASKBAR,
  Q_DISABLE_TASKBAR,
  Q_SHOW_TASKBAR,
  Q_HIDE_TASKBAR,
  Q_TOGGLE_AUTOHIDE,
  Q_ENABLE_AUTOHIDE,
  Q_DISABLE_AUTOHIDE,
  // pager
  Q_TOGGLE_PAGER,
  Q_ENABLE_PAGER,
  Q_DISABLE_PAGER,
  // icon
  Q_LINEUP_ICON,
  Q_EXEC_ICON,
  Q_DELETE_ICON
};

#include "hash.h"

class Menu;
class Qvwm;

/*
 * Qvwm internal function.
 */
class QvFunction {
public:
  static Hash<FuncNumber>* funcHashTable;

public:
  static Bool execFunction(FuncNumber fn, Menu* menu, int index);
  static Bool execFunction(FuncNumber fn, Menu* menu);
  static Bool execFunction(FuncNumber fn, Qvwm* qvWm);
  static Bool execFunction(FuncNumber fn);

  static void initialize();
};

struct FuncTable {
  const char* name;
  FuncNumber num;
};

#endif // FUNCTION_H_
