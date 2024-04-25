/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStartupPreferencePage.h"
#include <ui_QmitkStartupPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.startupdialog");
  }
}

QmitkStartupPreferencePage::QmitkStartupPreferencePage()
  : m_Ui(new Ui::QmitkStartupPreferencePage),
    m_Control(nullptr)
{
}

QmitkStartupPreferencePage::~QmitkStartupPreferencePage()
{
  delete m_Ui;
}

void QmitkStartupPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  this->Update();
}

QWidget* QmitkStartupPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkStartupPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkStartupPreferencePage::PerformCancel()
{
}

bool QmitkStartupPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();
  prefs->PutBool("skip", !m_Ui->showDialogCheckBox->isChecked());

  return true;
}

void QmitkStartupPreferencePage::Update()
{
  auto* prefs = GetPreferences();
  m_Ui->showDialogCheckBox->setChecked(!prefs->GetBool("skip", false));
}
