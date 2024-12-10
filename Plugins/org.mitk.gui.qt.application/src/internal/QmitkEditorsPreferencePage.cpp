/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEditorsPreferencePage.h"
#include <ui_QmitkEditorsPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.editors");
  }
}

QmitkEditorsPreferencePage::QmitkEditorsPreferencePage()
  : m_Control(nullptr),
    m_Ui(new Ui::QmitkEditorsPreferencePage)
{
}

QmitkEditorsPreferencePage::~QmitkEditorsPreferencePage() = default;

void QmitkEditorsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkEditorsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  this->Update();
}

QWidget* QmitkEditorsPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkEditorsPreferencePage::PerformOk()
{
  auto prefs = GetPreferences();

  const bool constrainZoomingAndPanning = m_Ui->m_ConstrainZoomingAndPanningCheckBox->isChecked();
  prefs->PutBool("Use constrained zooming and panning", constrainZoomingAndPanning);

  prefs->PutInt("max TS", m_Ui->m_MaxTSSpinBox->value());
  return true;
}

void QmitkEditorsPreferencePage::PerformCancel()
{
}

void QmitkEditorsPreferencePage::Update()
{
  auto prefs = GetPreferences();

  const bool constrainZoomingAndPanning = prefs->GetBool("Use constrained zooming and panning", true);
  m_Ui->m_ConstrainZoomingAndPanningCheckBox->setChecked(constrainZoomingAndPanning);

  const auto maxTS = prefs->GetInt("max TS", 50);
  m_Ui->m_MaxTSSpinBox->setValue(maxTS);
}
