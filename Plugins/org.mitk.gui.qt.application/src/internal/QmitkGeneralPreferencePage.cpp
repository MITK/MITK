/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkGeneralPreferencePage.h"

#include "QmitkDataNodeGlobalReinitAction.h"

#include <QCheckBox>
#include <QFormLayout>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkDataNodeGlobalReinitAction::ACTION_ID.toStdString());
  }
}

QmitkGeneralPreferencePage::QmitkGeneralPreferencePage()
  : m_MainControl(nullptr)
{
  // nothing here
}

void QmitkGeneralPreferencePage::Init(berry::IWorkbench::Pointer)
{
  // nothing here
}

void QmitkGeneralPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_GlobalReinitOnNodeDelete = new QCheckBox;
  m_GlobalReinitOnNodeVisibilityChanged = new QCheckBox;

  auto formLayout = new QFormLayout;
  formLayout->addRow("&Call global reinit if node is deleted", m_GlobalReinitOnNodeDelete);
  formLayout->addRow("&Call global reinit if node visibility is changed", m_GlobalReinitOnNodeVisibilityChanged);

  m_MainControl->setLayout(formLayout);
  Update();
}

QWidget* QmitkGeneralPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkGeneralPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool("Call global reinit if node is deleted", m_GlobalReinitOnNodeDelete->isChecked());
  prefs->PutBool("Call global reinit if node visibility is changed", m_GlobalReinitOnNodeVisibilityChanged->isChecked());

  return true;
}

void QmitkGeneralPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkGeneralPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_GlobalReinitOnNodeDelete->setChecked(prefs->GetBool("Call global reinit if node is deleted", true));
  m_GlobalReinitOnNodeVisibilityChanged->setChecked(prefs->GetBool("Call global reinit if node visibility is changed", false));
}
