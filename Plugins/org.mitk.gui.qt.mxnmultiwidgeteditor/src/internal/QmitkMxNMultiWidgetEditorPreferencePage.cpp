/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidgetEditorPreferencePage.h"
#include <ui_QmitkMxNMultiWidgetEditorPreferencePage.h>
#include "QmitkMxNMultiWidgetEditor.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkMxNMultiWidgetEditor::EDITOR_ID.toStdString());
  }
}

QmitkMxNMultiWidgetEditorPreferencePage::QmitkMxNMultiWidgetEditorPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkMxNMultiWidgetEditorPreferencePage>())
{
  // nothing here
}

QmitkMxNMultiWidgetEditorPreferencePage::~QmitkMxNMultiWidgetEditorPreferencePage()
{
  //nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_Ui->setupUi(m_MainControl);

  connect(m_Ui->m_ResetButton, SIGNAL(clicked()), SLOT(ResetPreferencesAndGUI()));

  Update();
}

QWidget* QmitkMxNMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkMxNMultiWidgetEditorPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool("Show level/window readout", m_Ui->m_ShowLevelWindowReadout->isChecked());
  prefs->PutBool("Expanded navigator", m_Ui->m_ExpandedNavigator->isChecked());
  prefs->PutBool("PACS like mouse interaction", m_Ui->m_PACSLikeMouseMode->isChecked());

  prefs->PutInt("crosshair gap size", m_Ui->m_CrosshairGapSize->value());

  return true;
}

void QmitkMxNMultiWidgetEditorPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_Ui->m_ShowLevelWindowReadout->setChecked(prefs->GetBool("Show level/window readout", true));
  m_Ui->m_ExpandedNavigator->setChecked(prefs->GetBool("Expanded navigator", false));
  m_Ui->m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));

  m_Ui->m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
}

void QmitkMxNMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  auto prefs = GetPreferences();
  prefs->Clear();

  this->Update();
  this->PerformOk();
}
