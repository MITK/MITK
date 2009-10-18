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

#include "cherryWorkbenchWindowAdvisor.h"

#include <Poco/Bugcheck.h>

#include "../internal/cherryWorkbenchWindowConfigurer.h"

namespace cherry
{

IWorkbenchWindowConfigurer::Pointer WorkbenchWindowAdvisor::GetWindowConfigurer()
{
  return windowConfigurer;
}

WorkbenchWindowAdvisor::WorkbenchWindowAdvisor(
    IWorkbenchWindowConfigurer::Pointer configurer)
{
  poco_assert(configurer.IsNotNull());
  this->windowConfigurer = configurer;
}

WorkbenchWindowAdvisor::~WorkbenchWindowAdvisor()
{
}

void WorkbenchWindowAdvisor::PreWindowOpen()
{
  // do nothing
}

ActionBarAdvisor::Pointer WorkbenchWindowAdvisor::CreateActionBarAdvisor(
    IActionBarConfigurer::Pointer configurer)
{
  ActionBarAdvisor::Pointer actionBarAdvisor(new ActionBarAdvisor(configurer));
  return actionBarAdvisor;
}

void WorkbenchWindowAdvisor::PostWindowRestore()
{
  // do nothing
}

void WorkbenchWindowAdvisor::OpenIntro()
{
  // TODO: Refactor this into an IIntroManager.openIntro(IWorkbenchWindow) call

  // introOpened flag needs to be global
//  IWorkbenchConfigurer wbConfig = getWindowConfigurer().getWorkbenchConfigurer();
//  final String key = "introOpened"; //$NON-NLS-1$
//  Boolean introOpened = (Boolean) wbConfig.getData(key);
//  if (introOpened != null && introOpened.booleanValue())
//  {
//    return;
//  }
//
//  wbConfig.setData(key, Boolean.TRUE);
//
//  boolean showIntro = PrefUtil.getAPIPreferenceStore().getBoolean(IWorkbenchPreferenceConstants.SHOW_INTRO);
//
//  IIntroManager introManager = wbConfig.getWorkbench().getIntroManager();
//
//  boolean hasIntro = introManager.hasIntro();
//  boolean isNewIntroContentAvailable = introManager.isNewContentAvailable();
//
//  if (hasIntro && (showIntro || isNewIntroContentAvailable))
//  {
//    introManager
//    .showIntro(getWindowConfigurer().getWindow(), false);
//
//    PrefUtil.getAPIPreferenceStore().setValue(IWorkbenchPreferenceConstants.SHOW_INTRO, false);
//    PrefUtil.saveAPIPrefs();
//  }
}

void WorkbenchWindowAdvisor::PostWindowCreate()
{
  // do nothing
}

void WorkbenchWindowAdvisor::PostWindowOpen()
{
  // do nothing
}

bool WorkbenchWindowAdvisor::PreWindowShellClose()
{
  // do nothing, but allow the close() to proceed
  return true;
}

void WorkbenchWindowAdvisor::PostWindowClose()
{
  // do nothing
}

void WorkbenchWindowAdvisor::CreateWindowContents(Shell::Pointer shell)
{
  this->GetWindowConfigurer().Cast<WorkbenchWindowConfigurer>()->CreateDefaultContents(shell);
}

void* WorkbenchWindowAdvisor::CreateEmptyWindowContents(void*  /*parent*/)
{
  return 0;
}

bool WorkbenchWindowAdvisor::SaveState(IMemento::Pointer  /*memento*/)
{
  // do nothing
  return true;
}

bool WorkbenchWindowAdvisor::RestoreState(IMemento::Pointer  /*memento*/)
{
  // do nothing
  return true;
}

}
