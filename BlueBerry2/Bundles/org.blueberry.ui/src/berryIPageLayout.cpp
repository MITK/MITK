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

#include "berryIPageLayout.h"

namespace berry {

const std::string IPageLayout::ID_EDITOR_AREA = "org.blueberry.ui.editors"; //$NON-NLS-1$

const std::string IPageLayout::ID_RES_NAV = "org.blueberry.ui.views.ResourceNavigator"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROP_SHEET = "org.blueberry.ui.views.PropertySheet"; //$NON-NLS-1$

const std::string IPageLayout::ID_OUTLINE = "org.blueberry.ui.views.ContentOutline"; //$NON-NLS-1$

const std::string IPageLayout::ID_BOOKMARKS = "org.blueberry.ui.views.BookmarkView"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROBLEM_VIEW = "org.blueberry.ui.views.ProblemView"; //$NON-NLS-1$

const std::string IPageLayout::ID_PROGRESS_VIEW = "org.blueberry.ui.views.ProgressView"; //$NON-NLS-1$

const std::string IPageLayout::ID_TASK_LIST = "org.blueberry.ui.views.TaskList"; //$NON-NLS-1$

const std::string IPageLayout::ID_NAVIGATE_ACTION_SET = "org.blueberry.ui.NavigateActionSet"; //$NON-NLS-1$

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
