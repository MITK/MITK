/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <QmitkLevelWindowWidget.h>
#include <QmitkSliderLevelWindowWidget.h>

#include <ui_QmitkLevelWindowWidget.h>

QmitkLevelWindowWidget::QmitkLevelWindowWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f), ui(std::make_unique<Ui::QmitkLevelWindow>())
{
  ui->setupUi(this);

  m_Manager = mitk::LevelWindowManager::New();

  ui->SliderLevelWindowWidget->SetLevelWindowManager(m_Manager.GetPointer());
  ui->LineEditLevelWindowWidget->SetLevelWindowManager(m_Manager.GetPointer());
}

QmitkLevelWindowWidget::~QmitkLevelWindowWidget()
{
}

void QmitkLevelWindowWidget::SetDataStorage(mitk::DataStorage *ds)
{
  m_Manager->SetDataStorage(ds);
}

void QmitkLevelWindowWidget::SetImageNameVisible(bool visible)
{
  ui->SliderLevelWindowWidget->SetImageNameVisible(visible);
}

mitk::LevelWindowManager *QmitkLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
