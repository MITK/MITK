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

#include "cherryIPageLayout.h"

namespace cherry {

const std::string IPageLayout::ID_EDITOR_AREA = "org.opencherry.ui.editors"; //$NON-NLS-1$

const std::string IPageLayout::ID_RES_NAV = "org.opencherry.ui.views.ResourceNavigator"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROP_SHEET = "org.opencherry.ui.views.PropertySheet"; //$NON-NLS-1$

const std::string IPageLayout::ID_OUTLINE = "org.opencherry.ui.views.ContentOutline"; //$NON-NLS-1$

const std::string IPageLayout::ID_BOOKMARKS = "org.opencherry.ui.views.BookmarkView"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROBLEM_VIEW = "org.opencherry.ui.views.ProblemView"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROGRESS_VIEW = "org.opencherry.ui.views.ProgressView"; //$NON-NLS-1$

const std::string IPageLayout::ID_TASK_LIST = "org.opencherry.ui.views.TaskList"; //$NON-NLS-1$

const std::string IPageLayout::ID_NAVIGATE_ACTION_SET = "org.opencherry.ui.NavigateActionSet"; //$NON-NLS-1$

const int IPageLayout::LEFT = 1;

const int IPageLayout::RIGHT = 2;

const int IPageLayout::TOP = 3;

const int IPageLayout::BOTTOM = 4;

const float IPageLayout::RATIO_MIN = 0.05f;

const float IPageLayout::RATIO_MAX = 0.95f;

const float IPageLayout::DEFAULT_VIEW_RATIO = 0.5f;

const float IPageLayout::INVALID_RATIO = -1.0;

const float IPageLayout::NULL_RATIO = -2.0;

}
