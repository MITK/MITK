/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryWWinActionBars.h"

#include "berryWorkbenchWindow.h"
#include "berryMenuManager.h"

namespace berry {

WWinActionBars::WWinActionBars(WorkbenchWindow* window)
  : window(window)
{
}

void WWinActionBars::ClearGlobalActionHandlers()
{
}

Action* WWinActionBars::GetGlobalActionHandler(const QString& /*actionID*/) const
{
  return nullptr;
}

IMenuManager* WWinActionBars::GetMenuManager()
{
  return window->GetMenuManager();
}

IServiceLocator* WWinActionBars::GetServiceLocator()
{
  return window;
}

IStatusLineManager* WWinActionBars::GetStatusLineManager()
{
  //TODO StatusLineManager
  //return window->GetStatusLineManager();
  return nullptr;
}

IToolBarManager* WWinActionBars::GetToolBarManager()
{
  //return window->GetToolBarManager();
  //TODO ToolBarManager
  return nullptr;
}

void WWinActionBars::SetGlobalActionHandler(const QString& /*actionID*/, Action* /*handler*/)
{
}

void WWinActionBars::UpdateActionBars()
{
  window->UpdateActionBars();
}

}
