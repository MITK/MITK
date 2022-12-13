/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMitkWorkbenchIntroPreferencePage.h"
#include "QmitkMitkWorkbenchIntroPart.h"

#include <berryPlatformUI.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <ui_QmitkMitkWorkbenchIntroPreferencePage.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.qt.extapplicationintro");
  }
}

QmitkMitkWorkbenchIntroPreferencePage::QmitkMitkWorkbenchIntroPreferencePage()
  : m_Ui(new Ui::QmitkMitkWorkbenchIntroPreferencePage),
    m_Control(nullptr)
{
}

QmitkMitkWorkbenchIntroPreferencePage::~QmitkMitkWorkbenchIntroPreferencePage()
{
}

void QmitkMitkWorkbenchIntroPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkMitkWorkbenchIntroPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  this->Update();
}

QWidget* QmitkMitkWorkbenchIntroPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkMitkWorkbenchIntroPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();
  prefs->PutBool("show tips", m_Ui->showTipsCheckBox->isChecked());

  auto intro = berry::PlatformUI::GetWorkbench()->GetIntroManager()->GetIntro();

  if (intro.IsNotNull())
  {
    auto* workbenchIntro = dynamic_cast<QmitkMitkWorkbenchIntroPart*>(intro.GetPointer());

    if (workbenchIntro != nullptr)
      workbenchIntro->ReloadPage();
  }

  return true;
}

void QmitkMitkWorkbenchIntroPreferencePage::PerformCancel()
{
}

void QmitkMitkWorkbenchIntroPreferencePage::Update()
{
  auto* prefs = GetPreferences();
  m_Ui->showTipsCheckBox->setChecked(prefs->GetBool("show tips", true));
}
