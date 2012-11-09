/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkAppInstancesPreferencePage.h"

#include <berryIPreferencesService.h>
#include <berryQtPreferences.h>

QmitkAppInstancesPreferencePage::QmitkAppInstancesPreferencePage()
{

}

void QmitkAppInstancesPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkAppInstancesPreferencePage::CreateQtControl(QWidget* parent)
{
  mainWidget = new QWidget(parent);
  controls.setupUi(mainWidget);

  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  prefs = prefService->GetSystemPreferences()->Node("/General");

  Update();
}

QWidget* QmitkAppInstancesPreferencePage::GetQtControl() const
{
  return mainWidget;
}

bool QmitkAppInstancesPreferencePage::PerformOk()
{
  prefs->PutBool("newInstance.always", controls.newInstanceAlways->isChecked());
  prefs->PutBool("newInstance.scene", controls.newInstanceScene->isChecked());
  return true;
}

void QmitkAppInstancesPreferencePage::PerformCancel()
{

}

void QmitkAppInstancesPreferencePage::Update()
{
  bool always = prefs->GetBool("newInstance.always", false);
  bool scene = prefs->GetBool("newInstance.scene", true);

  controls.newInstanceAlways->setChecked(always);
  controls.newInstanceScene->setChecked(scene);
}
