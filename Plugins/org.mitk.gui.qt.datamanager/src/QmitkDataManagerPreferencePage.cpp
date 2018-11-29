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

#include "QmitkDataManagerPreferencePage.h"
#include "QmitkDataManagerView.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_DataManagerPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkDataManagerView::VIEW_ID);

  m_MainControl = new QWidget(parent);
  m_EnableSingleEditing = new QCheckBox;
  m_PlaceNewNodesOnTop = new QCheckBox;
  m_ShowHelperObjects = new QCheckBox;
  m_ShowNodesContainingNoData = new QCheckBox;

  m_UseSurfaceDecimation = new QCheckBox;
  m_AllowParentChange = new QCheckBox;

  auto formLayout = new QFormLayout;
  formLayout->addRow("&Single click property editing:", m_EnableSingleEditing);
  formLayout->addRow("&Place new nodes on top:", m_PlaceNewNodesOnTop);
  formLayout->addRow("&Show helper objects:", m_ShowHelperObjects);
  formLayout->addRow("&Show nodes containing no data", m_ShowNodesContainingNoData);
  formLayout->addRow("&Use surface decimation:", m_UseSurfaceDecimation);
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
  m_DataManagerPreferencesNode->PutBool("Single click property editing", m_EnableSingleEditing->isChecked());
  m_DataManagerPreferencesNode->PutBool("Place new nodes on top", m_PlaceNewNodesOnTop->isChecked());
  m_DataManagerPreferencesNode->PutBool("Show helper objects", m_ShowHelperObjects->isChecked());
  m_DataManagerPreferencesNode->PutBool("Show nodes containing no data", m_ShowNodesContainingNoData->isChecked());
  m_DataManagerPreferencesNode->PutBool("Use surface decimation", m_UseSurfaceDecimation->isChecked());
  m_DataManagerPreferencesNode->PutBool("Allow changing of parent node", m_AllowParentChange->isChecked());

  return true;
}

void QmitkDataManagerPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkDataManagerPreferencePage::Update()
{
  m_EnableSingleEditing->setChecked(m_DataManagerPreferencesNode->GetBool("Single click property editing", true));
  m_PlaceNewNodesOnTop->setChecked(m_DataManagerPreferencesNode->GetBool("Place new nodes on top", true));
  m_ShowHelperObjects->setChecked(m_DataManagerPreferencesNode->GetBool("Show helper objects", false));
  m_ShowNodesContainingNoData->setChecked(m_DataManagerPreferencesNode->GetBool("Show nodes containing no data", false));
  m_UseSurfaceDecimation->setChecked(m_DataManagerPreferencesNode->GetBool("Use surface decimation", true));
  m_AllowParentChange->setChecked(m_DataManagerPreferencesNode->GetBool("Allow changing of parent node", false));
}
