/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryIWorkbenchPage.h"

namespace berry {

const QString IWorkbenchPage::EDITOR_ID_ATTR = "org.blueberry.ui.editorID";

const QString IWorkbenchPage::CHANGE_RESET = "reset";

const QString IWorkbenchPage::CHANGE_RESET_COMPLETE = "resetComplete";

const QString IWorkbenchPage::CHANGE_VIEW_SHOW = "viewShow";

const QString IWorkbenchPage::CHANGE_VIEW_HIDE = "viewHide";

const QString IWorkbenchPage::CHANGE_EDITOR_OPEN = "editorOpen";

const QString IWorkbenchPage::CHANGE_EDITOR_CLOSE = "editorClose";

const QString IWorkbenchPage::CHANGE_EDITOR_AREA_SHOW = "editorAreaShow";

const QString IWorkbenchPage::CHANGE_EDITOR_AREA_HIDE = "editorAreaHide";

const QString IWorkbenchPage::CHANGE_ACTION_SET_SHOW = "actionSetShow";

const QString IWorkbenchPage::CHANGE_ACTION_SET_HIDE = "actionSetHide";

const int IWorkbenchPage::VIEW_ACTIVATE = 1;

const int IWorkbenchPage::VIEW_VISIBLE = 2;

const int IWorkbenchPage::VIEW_CREATE = 3;

const int IWorkbenchPage::MATCH_NONE = 0;

const int IWorkbenchPage::MATCH_INPUT = 1;

const int IWorkbenchPage::MATCH_ID = 2;

IWorkbenchPage::~IWorkbenchPage()
{

}

}
