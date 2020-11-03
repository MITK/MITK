/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIPageLayout.h"

namespace berry {

const QString IPageLayout::ID_EDITOR_AREA = "org.blueberry.ui.editors";

const QString IPageLayout::ID_RES_NAV = "org.blueberry.ui.views.ResourceNavigator";

const QString IPageLayout::ID_PROP_SHEET = "org.blueberry.ui.views.PropertySheet";

const QString IPageLayout::ID_OUTLINE = "org.blueberry.ui.views.ContentOutline";

const QString IPageLayout::ID_BOOKMARKS = "org.blueberry.ui.views.BookmarkView";

const QString IPageLayout::ID_PROBLEM_VIEW = "org.blueberry.ui.views.ProblemView";

const QString IPageLayout::ID_PROGRESS_VIEW = "org.blueberry.ui.views.ProgressView";

const QString IPageLayout::ID_TASK_LIST = "org.blueberry.ui.views.TaskList";

const QString IPageLayout::ID_NAVIGATE_ACTION_SET = "org.blueberry.ui.NavigateActionSet";

const int IPageLayout::LEFT = 1;

const int IPageLayout::RIGHT = 2;

const int IPageLayout::TOP = 3;

const int IPageLayout::BOTTOM = 4;

const float IPageLayout::RATIO_MIN = 0.05f;

const float IPageLayout::RATIO_MAX = 0.95f;

const float IPageLayout::DEFAULT_VIEW_RATIO = 0.5f;

const float IPageLayout::INVALID_RATIO = -1.0;

const float IPageLayout::nullptr_RATIO = -2.0;

}
