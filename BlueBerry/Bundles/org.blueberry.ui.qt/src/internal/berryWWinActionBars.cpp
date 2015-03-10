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
  return 0;
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
  return 0;
}

IToolBarManager* WWinActionBars::GetToolBarManager()
{
  //return window->GetToolBarManager();
  //TODO ToolBarManager
  return 0;
}

void WWinActionBars::SetGlobalActionHandler(const QString& /*actionID*/, Action* /*handler*/)
{
}

void WWinActionBars::UpdateActionBars()
{
  window->UpdateActionBars();
}

}
