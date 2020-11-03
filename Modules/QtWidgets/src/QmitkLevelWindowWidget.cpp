/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkLevelWindowWidget.h"
#include "QmitkSliderLevelWindowWidget.h"

QmitkLevelWindowWidget::QmitkLevelWindowWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  this->setupUi(this);

  m_Manager = mitk::LevelWindowManager::New();

  SliderLevelWindowWidget->SetLevelWindowManager(m_Manager.GetPointer());
  LineEditLevelWindowWidget->SetLevelWindowManager(m_Manager.GetPointer());
}

void QmitkLevelWindowWidget::SetDataStorage(mitk::DataStorage *ds)
{
  m_Manager->SetDataStorage(ds);
}

mitk::LevelWindowManager *QmitkLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
