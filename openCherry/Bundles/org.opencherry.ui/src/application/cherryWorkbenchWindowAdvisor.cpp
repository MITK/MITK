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

#include <cherryWorkbenchPreferenceConstants.h>
#include <cherryObjects.h>
#include <cherryIPreferences.h>

#include "../internal/cherryWorkbenchWindowConfigurer.h"
#include "../internal/cherryWorkbenchPlugin.h"

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
  IWorkbenchConfigurer::Pointer wbConfig = GetWindowConfigurer()->GetWorkbenchConfigurer();
  std::string key = "introOpened"; //$NON-NLS-1$
  ObjectBool::Pointer introOpened = wbConfig->GetData(key).Cast<ObjectBool>();
  if (introOpened && introOpened->GetValue())
  {
    return;
  }

  wbConfig->SetData(key, ObjectBool::Pointer(new ObjectBool(true)));

  IPreferences::Pointer workbenchPrefs = WorkbenchPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();

  bool showIntro = workbenchPrefs->GetBool(WorkbenchPreferenceConstants::SHOW_INTRO, true);

  IIntroManager* introManager = wbConfig->GetWorkbench()->GetIntroManager();

  bool hasIntro = introManager->HasIntro();
  bool isNewIntroContentAvailable = introManager->IsNewContentAvailable();

  if (hasIntro && (showIntro || isNewIntroContentAvailable))
  {
    introManager
    ->ShowIntro(GetWindowConfigurer()->GetWindow(), false);

    workbenchPrefs->PutBool(WorkbenchPreferenceConstants::SHOW_INTRO, false);
    workbenchPrefs->Flush();
  }
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
