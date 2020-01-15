/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISources.h"

#include <QString>

namespace berry
{

int ISources::WORKBENCH()
{
  static int val = 0;
  return val;
}

int ISources::ACTIVE_CONTEXT()
{
  static int val = 1 << 3;
  return val;
}

const QString ISources::ACTIVE_CONTEXT_NAME()
{
  static const QString val = "activeContexts";
  return val;
}

int ISources::ACTIVE_ACTION_SETS()
{
  static int val = 1 << 5;
  return val;
}

const QString ISources::ACTIVE_ACTION_SETS_NAME()
{
  static const QString val = "activeActionSets";
  return val;
}

int ISources::ACTIVE_SHELL()
{
  static int val = 1 << 7;
  return val;
}

const QString ISources::ACTIVE_SHELL_NAME()
{
  static const QString val = "activeShell";
  return val;
}

int ISources::ACTIVE_WORKBENCH_WINDOW_SHELL()
{
  static int val = 1 << 9;
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()
{
  static const QString val = "activeWorkbenchWindowShell";
  return val;
}

int ISources::ACTIVE_WORKBENCH_WINDOW()
{
  static int val = 1 << 11;
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
{
  static const QString val = "activeWorkbenchWindow";
  return val;
}

int ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE()
{
  static int val = 1 << 12;
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME()
{
  static const QString val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".isToolbarVisible";
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME()
{
  static const QString val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".isPerspectiveBarVisible";
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME()
{
  static const QString val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".isStatusLineVisible";
  return val;
}

const QString ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME()
{
  static const QString val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".activePerspective";
  return val;
}

int ISources::ACTIVE_EDITOR()
{
  static int val = 1 << 13;
  return val;
}

const QString ISources::ACTIVE_EDITOR_NAME()
{
  static const QString val = "activeEditor";
  return val;
}

const QString ISources::ACTIVE_EDITOR_INPUT_NAME()
{
  static const QString val = "activeEditorInput";
  return val;
}

int ISources::ACTIVE_EDITOR_ID()
{
  static int val = 1 << 15;
  return val;
}

const QString ISources::ACTIVE_EDITOR_ID_NAME()
{
  static const QString val = "activeEditorId";
  return val;
}

int ISources::ACTIVE_PART()
{
  static int val = 1 << 17;
  return val;
}

const QString ISources::ACTIVE_PART_NAME()
{
  static const QString val = "activePart";
  return val;
}

int ISources::ACTIVE_PART_ID()
{
  static int val = 1 << 19;
  return val;
}

const QString ISources::ACTIVE_PART_ID_NAME()
{
  static const QString val = "activePartId";
  return val;
}

int ISources::ACTIVE_SITE()
{
  static int val = 1 << 23;
  return val;
}

const QString ISources::ACTIVE_SITE_NAME()
{
  static const QString val = "activeSite";
  return val;
}

const QString ISources::SHOW_IN_SELECTION()
{
  static const QString val = "showInSelection";
  return val;
}

const QString ISources::SHOW_IN_INPUT()
{
  static const QString val = "showInInput";
  return val;
}

int ISources::ACTIVE_CURRENT_SELECTION()
{
  static int val = 1 << 27;
  return val;
}

const QString ISources::ACTIVE_CURRENT_SELECTION_NAME()
{
  static const QString val = "selection";
  return val;
}

int ISources::ACTIVE_MENU()
{
  static int val = 1 << 30;
  return val;
}

const QString ISources::ACTIVE_MENU_NAME()
{
  static const QString val = "activeMenu";
  return val;
}

const QString ISources::ACTIVE_MENU_SELECTION_NAME()
{
  static const QString val = "activeMenuSelection";
  return val;
}

const QString ISources::ACTIVE_MENU_EDITOR_INPUT_NAME()
{
  static const QString val = "activeMenuEditorInput";
  return val;
}

const QString ISources::ACTIVE_FOCUS_CONTROL_NAME()
{
  static const QString val = "activeFocusControl";
  return val;
}

const QString ISources::ACTIVE_FOCUS_CONTROL_ID_NAME()
{
  static const QString val = "activeFocusControlId";
  return val;
}

}
