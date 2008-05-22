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

#include "cherryIWorkbenchPage.h"

namespace cherry {

const std::string IWorkbenchPage::EDITOR_ID_ATTR = "org.opencherry.ui.editorID"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_RESET = "reset"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_RESET_COMPLETE = "resetComplete"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_VIEW_SHOW = "viewShow"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_VIEW_HIDE = "viewHide"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_EDITOR_OPEN = "editorOpen"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_EDITOR_CLOSE = "editorClose"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_EDITOR_AREA_SHOW = "editorAreaShow"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_EDITOR_AREA_HIDE = "editorAreaHide"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_ACTION_SET_SHOW = "actionSetShow"; //$NON-NLS-1$

  const std::string IWorkbenchPage::CHANGE_ACTION_SET_HIDE = "actionSetHide"; //$NON-NLS-1$

  const int IWorkbenchPage::VIEW_ACTIVATE = 1;

  const int IWorkbenchPage::VIEW_VISIBLE = 2;

  const int IWorkbenchPage::VIEW_CREATE = 3;

  const int IWorkbenchPage::MATCH_NONE = 0;

  const int IWorkbenchPage::MATCH_INPUT = 1;

  const int IWorkbenchPage::MATCH_ID = 2;

}
