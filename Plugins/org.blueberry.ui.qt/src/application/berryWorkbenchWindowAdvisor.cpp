/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchWindowAdvisor.h"

#include <Poco/Bugcheck.h>

#include <berryShell.h>
#include <berryActionBarAdvisor.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <berryObjects.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>

#include "internal/berryWorkbenchWindowConfigurer.h"
#include "berryWorkbenchPlugin.h"

namespace berry
{

IWorkbenchWindowConfigurer::Pointer WorkbenchWindowAdvisor::GetWindowConfigurer()
{
  return windowConfigurer;
}

WorkbenchWindowAdvisor::WorkbenchWindowAdvisor(
    const IWorkbenchWindowConfigurer::Pointer& configurer)
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
  QString key = "introOpened"; //$NON-NLS-1$
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

QWidget* WorkbenchWindowAdvisor::CreateEmptyWindowContents(QWidget*  /*parent*/)
{
  return nullptr;
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
