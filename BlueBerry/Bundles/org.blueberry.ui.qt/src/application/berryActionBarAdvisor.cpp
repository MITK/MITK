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

#include "berryActionBarAdvisor.h"

#include <berryIActionBarConfigurer.h>
#include <berryIWorkbenchWindowConfigurer.h>
#include <berryIWorkbenchWindow.h>
#include <berryIMemento.h>

namespace berry
{

ActionBarAdvisor::ActionBarAdvisor(const SmartPointer<IActionBarConfigurer>& configurer)
  : actionBarConfigurer(configurer)
{
  Q_ASSERT(configurer.IsNotNull());
}

IActionBarConfigurer::Pointer ActionBarAdvisor::GetActionBarConfigurer() const
{
  return actionBarConfigurer;
}

void ActionBarAdvisor::FillActionBars(FillFlags flags)
{
  if (flags.testFlag(FILL_PROXY))
  {
    this->MakeActions(actionBarConfigurer->GetWindowConfigurer()->GetWindow().GetPointer());
  }
  if (flags.testFlag(FILL_MENU_BAR))
  {
    this->FillMenuBar(actionBarConfigurer->GetMenuManager());
  }
  if (flags.testFlag(FILL_TOOL_BAR))
  {
    this->FillToolBar(actionBarConfigurer->GetToolBarManager());
  }
  if (flags.testFlag(FILL_STATUS_LINE))
  {
    //this->FillStatusLine(actionBarConfigurer->GetStatusLineManager());
  }
}

void ActionBarAdvisor::MakeActions(IWorkbenchWindow*  /*window*/)
{
  // do nothing
}

void ActionBarAdvisor::FillMenuBar(IMenuManager*  /*menuBar*/)
{
  // do nothing
}

void ActionBarAdvisor::FillToolBar(IToolBarManager* /*toolBar*/)
{
  // do nothing
}

void ActionBarAdvisor::FillStatusLine(IStatusLineManager* /*statusLine*/)
{
  // do nothing
}

bool ActionBarAdvisor::SaveState(SmartPointer<IMemento>  /*memento*/)
{
  return true;
}

bool ActionBarAdvisor::RestoreState(SmartPointer<IMemento>  /*memento*/)
{
  return true;
}

}
