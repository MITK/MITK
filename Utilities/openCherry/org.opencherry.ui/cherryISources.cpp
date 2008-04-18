/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryISources.h"

namespace cherry {

const int ISources::ISources::WORKBENCH = 0;

 const int ISources::LEGACY_LEGACY = 1;

 const int ISources::LEGACY_LOW = 1 << 1;

 const int ISources::LEGACY_MEDIUM = 1 << 2;

 const int ISources::ACTIVE_CONTEXT = 1 << 6;

 const std::string ISources::ACTIVE_CONTEXT_NAME = "activeContexts"; //$NON-NLS-1$

 const int ISources::ACTIVE_ACTION_SETS = 1 << 8;

 const std::string ISources::ACTIVE_ACTION_SETS_NAME = "activeActionSets"; //$NON-NLS-1$

 const int ISources::ACTIVE_SHELL = 1 << 10;

 const std::string ISources::ACTIVE_SHELL_NAME = "activeShell"; //$NON-NLS-1$

 const int ISources::ACTIVE_WORKBENCH_WINDOW_SHELL = 1 << 12;

 const std::string ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME = "activeWorkbenchWindowShell"; //$NON-NLS-1$

 const int ISources::ACTIVE_WORKBENCH_WINDOW = 1 << 14;

 const std::string ISources::ACTIVE_WORKBENCH_WINDOW_NAME = "activeWorkbenchWindow"; //$NON-NLS-1$
 
 const int ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE = 1 << 15;
 
 const std::string ISources::ACTIVE_WORKBENCH_WINDOW_IS_COOLBAR_VISIBLE_NAME = ISources::ACTIVE_WORKBENCH_WINDOW_NAME
     + ".isCoolbarVisible"; //$NON-NLS-1$
 
 const std::string ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME = ISources::ACTIVE_WORKBENCH_WINDOW_NAME
     + ".isPerspectiveBarVisible"; //$NON-NLS-1$
 
 const std::string ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME = ISources::ACTIVE_WORKBENCH_WINDOW_NAME
 + ".activePerspective"; //$NON-NLS-1$

 const int ISources::ACTIVE_EDITOR = 1 << 16;

 const std::string ISources::ACTIVE_EDITOR_NAME = "activeEditor"; //$NON-NLS-1$

 const int ISources::ACTIVE_EDITOR_ID = 1 << 18;

 const std::string ISources::ACTIVE_EDITOR_ID_NAME = "activeEditorId"; //$NON-NLS-1$

 const int ISources::ACTIVE_PART = 1 << 20;

 const std::string ISources::ACTIVE_PART_NAME = "activePart"; //$NON-NLS-1$

 const int ISources::ACTIVE_PART_ID = 1 << 22;

 const std::string ISources::ACTIVE_PART_ID_NAME = "activePartId"; //$NON-NLS-1$

 const int ISources::ACTIVE_SITE = 1 << 26;

 const std::string ISources::ACTIVE_SITE_NAME = "activeSite"; //$NON-NLS-1$
 
 const std::string ISources::SHOW_IN_SELECTION = "showInSelection"; //$NON-NLS-1$
 
 const std::string ISources::SHOW_IN_INPUT = "showInInput"; //$NON-NLS-1$

 const int ISources::ACTIVE_CURRENT_SELECTION = 1 << 30;

 const std::string ISources::ACTIVE_CURRENT_SELECTION_NAME = "selection"; //$NON-NLS-1$

 const int ISources::ACTIVE_MENU = 1 << 31;

 const std::string ISources::ACTIVE_MENU_NAME = "activeMenu"; //$NON-NLS-1$
 
 const std::string ISources::ACTIVE_MENU_SELECTION_NAME = "activeMenuSelection";  //$NON-NLS-1$
 
 const std::string ISources::ACTIVE_MENU_EDITOR_INPUT_NAME = "activeMenuEditorInput";  //$NON-NLS-1$

 const std::string ISources::ACTIVE_FOCUS_CONTROL_NAME = "activeFocusControl"; //$NON-NLS-1$

 const std::string ISources::ACTIVE_FOCUS_CONTROL_ID_NAME = "activeFocusControlId"; //$NON-NLS-1$

}
