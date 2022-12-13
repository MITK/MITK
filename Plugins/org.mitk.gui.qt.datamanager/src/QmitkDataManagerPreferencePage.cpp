/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataManagerPreferencePage.h"
#include "QmitkDataManagerView.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkDataManagerView::VIEW_ID.toStdString());
  }
}

QmitkDataManagerPreferencePage::QmitkDataManagerPreferencePage()
  : m_MainControl(nullptr)
{
  // nothing here
}

void QmitkDataManagerPreferencePage::Init(berry::IWorkbench::Pointer )
{
  // nothing here
}

void QmitkDataManagerPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);
  m_EnableSingleEditing = new QCheckBox;
  m_PlaceNewNodesOnTop = new QCheckBox;
  m_ShowHelperObjects = new QCheckBox;
  m_ShowNodesContainingNoData = new QCheckBox;

  m_AllowParentChange = new QCheckBox;

  auto formLayout = new QFormLayout;
  formLayout->addRow("&Single click property editing:", m_EnableSingleEditing);
  formLayout->addRow("&Place new nodes on top:", m_PlaceNewNodesOnTop);
  formLayout->addRow("&Show helper objects:", m_ShowHelperObjects);
  formLayout->addRow("&Show nodes containing no data", m_ShowNodesContainingNoData);
  formLayout->addRow("&Allow changing of parent node:", m_AllowParentChange);

  m_MainControl->setLayout(formLayout);
  Update();
}

QWidget* QmitkDataManagerPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkDataManagerPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool("Single click property editing", m_EnableSingleEditing->isChecked());
  prefs->PutBool("Place new nodes on top", m_PlaceNewNodesOnTop->isChecked());
  prefs->PutBool("Show helper objects", m_ShowHelperObjects->isChecked());
  prefs->PutBool("Show nodes containing no data", m_ShowNodesContainingNoData->isChecked());
  prefs->PutBool("Allow changing of parent node", m_AllowParentChange->isChecked());

  return true;
}

void QmitkDataManagerPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkDataManagerPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_EnableSingleEditing->setChecked(prefs->GetBool("Single click property editing", true));
  m_PlaceNewNodesOnTop->setChecked(prefs->GetBool("Place new nodes on top", true));
  m_ShowHelperObjects->setChecked(prefs->GetBool("Show helper objects", false));
  m_ShowNodesContainingNoData->setChecked(prefs->GetBool("Show nodes containing no data", false));
  m_AllowParentChange->setChecked(prefs->GetBool("Allow changing of parent node", false));
}
