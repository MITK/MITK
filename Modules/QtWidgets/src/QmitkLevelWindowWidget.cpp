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
#include "QmitkLevelWindowWidget.h"
#include "QmitkSliderLevelWindowWidget.h"


QmitkLevelWindowWidget::QmitkLevelWindowWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f)
{
  this->setupUi(this);

  m_Manager = mitk::LevelWindowManager::New();

  SliderLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
  connect(SliderLevelWindowWidget, &QmitkSliderLevelWindowWidget::RequestUpdate, this, &QmitkLevelWindowWidget::RequestUpdate);
  LineEditLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
  connect(LineEditLevelWindowWidget, &QmitkLineEditLevelWindowWidget::RequestUpdate, this, &QmitkLevelWindowWidget::RequestUpdate);
}


void QmitkLevelWindowWidget::SetDataStorage( mitk::DataStorage* ds )
{
  m_Manager->SetDataStorage(ds);
}


mitk::LevelWindowManager* QmitkLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
