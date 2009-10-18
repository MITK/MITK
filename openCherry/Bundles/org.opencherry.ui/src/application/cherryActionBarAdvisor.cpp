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

#include "cherryActionBarAdvisor.h"

namespace cherry
{

const int ActionBarAdvisor::FILL_PROXY = 0x01;

const int ActionBarAdvisor::FILL_MENU_BAR = 0x02;

const int ActionBarAdvisor::FILL_COOL_BAR = 0x04;

const int ActionBarAdvisor::FILL_STATUS_LINE = 0x08;

ActionBarAdvisor::ActionBarAdvisor(IActionBarConfigurer::Pointer configurer)
{
  poco_assert(configurer.IsNotNull());
  actionBarConfigurer = configurer;
}

IActionBarConfigurer::Pointer ActionBarAdvisor::GetActionBarConfigurer()
{
  return actionBarConfigurer;
}

void ActionBarAdvisor::FillActionBars(int flags)
{
  if ((flags & FILL_PROXY) == 0)
  {
    this->MakeActions(actionBarConfigurer->GetWindowConfigurer()->GetWindow());
  }
  if ((flags & FILL_MENU_BAR) != 0)
  {
    this->FillMenuBar(actionBarConfigurer->GetMenuManager());
  }
  //        if ((flags & FILL_COOL_BAR) != 0) {
  //            fillCoolBar(actionBarConfigurer.getCoolBarManager());
  //        }
  //        if ((flags & FILL_STATUS_LINE) != 0) {
  //            fillStatusLine(actionBarConfigurer.getStatusLineManager());
  //        }
}

void ActionBarAdvisor::MakeActions(IWorkbenchWindow::Pointer  /*window*/)
{
  // do nothing
}

void ActionBarAdvisor::FillMenuBar(void*  /*menuBar*/)
{
  // do nothing
}

bool ActionBarAdvisor::SaveState(IMemento::Pointer  /*memento*/)
{
  return true;
}

bool ActionBarAdvisor::RestoreState(IMemento::Pointer  /*memento*/)
{
  return true;
}

}
