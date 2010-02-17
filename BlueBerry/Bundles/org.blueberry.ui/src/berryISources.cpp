/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryISources.h"

namespace berry
{

int ISources::WORKBENCH()
{
  static int val = 0;
  return val;
}

int ISources::LEGACY_LEGACY()
{
  static int val = 1;
  return val;
}

int ISources::LEGACY_LOW()
{
  static int val = 1 << 1;
  return val;
}

int ISources::LEGACY_MEDIUM()
{
  static int val = 1 << 2;
  return val;
}

int ISources::ACTIVE_CONTEXT()
{
  static int val = 1 << 6;
  return val;
}

const std::string ISources::ACTIVE_CONTEXT_NAME()
{
  static const std::string val = "activeContexts";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_ACTION_SETS()
{
  static int val = 1 << 8;
  return val;
}

const std::string ISources::ACTIVE_ACTION_SETS_NAME()
{
  static const std::string val = "activeActionSets";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_SHELL()
{
  static int val = 1 << 10;
  return val;
}

const std::string ISources::ACTIVE_SHELL_NAME()
{
  static const std::string val = "activeShell";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_WORKBENCH_WINDOW_SHELL()
{
  static int val = 1 << 12;
  return val;
}

const std::string ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()
{
  static const std::string val = "activeWorkbenchWindowShell";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_WORKBENCH_WINDOW()
{
  static int val = 1 << 14;
  return val;
}

const std::string ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
{
  static const std::string val = "activeWorkbenchWindow";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE()
{
  static int val = 1 << 15;
  return val;
}

const std::string ISources::ACTIVE_WORKBENCH_WINDOW_IS_COOLBAR_VISIBLE_NAME()
{
  static const std::string val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".isCoolbarVisible";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME()
{
  static const std::string val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".isPerspectiveBarVisible";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME()
{
  static const std::string val = ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
      + ".activePerspective";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_EDITOR()
{
  static int val = 1 << 16;
  return val;
}

const std::string ISources::ACTIVE_EDITOR_NAME()
{
  static const std::string val = "activeEditor";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_EDITOR_ID()
{
  static int val = 1 << 18;
  return val;
}

const std::string ISources::ACTIVE_EDITOR_ID_NAME()
{
  static const std::string val = "activeEditorId";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_PART()
{
  static int val = 1 << 20;
  return val;
}

const std::string ISources::ACTIVE_PART_NAME()
{
  static const std::string val = "activePart";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_PART_ID()
{
  static int val = 1 << 22;
  return val;
}

const std::string ISources::ACTIVE_PART_ID_NAME()
{
  static const std::string val = "activePartId";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_SITE()
{
  static int val = 1 << 26;
  return val;
}

const std::string ISources::ACTIVE_SITE_NAME()
{
  static const std::string val = "activeSite";
  return val;
} //$NON-NLS-1$

const std::string ISources::SHOW_IN_SELECTION()
{
  static const std::string val = "showInSelection";
  return val;
} //$NON-NLS-1$

const std::string ISources::SHOW_IN_INPUT()
{
  static const std::string val = "showInInput";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_CURRENT_SELECTION()
{
  static int val = 1 << 30;
  return val;
}

const std::string ISources::ACTIVE_CURRENT_SELECTION_NAME()
{
  static const std::string val = "selection";
  return val;
} //$NON-NLS-1$

int ISources::ACTIVE_MENU()
{
  static int val = 1 << 31;
  return val;
}

const std::string ISources::ACTIVE_MENU_NAME()
{
  static const std::string val = "activeMenu";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_MENU_SELECTION_NAME()
{
  static const std::string val = "activeMenuSelection";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_MENU_EDITOR_INPUT_NAME()
{
  static const std::string val = "activeMenuEditorInput";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_FOCUS_CONTROL_NAME()
{
  static const std::string val = "activeFocusControl";
  return val;
} //$NON-NLS-1$

const std::string ISources::ACTIVE_FOCUS_CONTROL_ID_NAME()
{
  static const std::string val = "activeFocusControlId";
  return val;
} //$NON-NLS-1$

}
