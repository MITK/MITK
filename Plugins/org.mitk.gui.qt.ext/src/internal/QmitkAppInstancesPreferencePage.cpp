/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAppInstancesPreferencePage.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("General");
  }
}

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

  Update();
}

QWidget* QmitkAppInstancesPreferencePage::GetQtControl() const
{
  return mainWidget;
}

bool QmitkAppInstancesPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool("newInstance.always", controls.newInstanceAlways->isChecked());
  prefs->PutBool("newInstance.scene", controls.newInstanceScene->isChecked());

  return true;
}

void QmitkAppInstancesPreferencePage::PerformCancel()
{

}

void QmitkAppInstancesPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  bool always = prefs->GetBool("newInstance.always", false);
  bool scene = prefs->GetBool("newInstance.scene", true);

  controls.newInstanceAlways->setChecked(always);
  controls.newInstanceScene->setChecked(scene);
}
