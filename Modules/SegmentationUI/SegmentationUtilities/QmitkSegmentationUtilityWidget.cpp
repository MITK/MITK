/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationUtilityWidget.h"

QmitkSegmentationUtilityWidget::QmitkSegmentationUtilityWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QWidget(parent)
{
  this->SetTimeNavigationController(timeNavigationController);
}

QmitkSegmentationUtilityWidget::~QmitkSegmentationUtilityWidget()
{
}

mitk::SliceNavigationController* QmitkSegmentationUtilityWidget::GetTimeNavigationController() const
{
  return m_TimeNavigationController;
}

void QmitkSegmentationUtilityWidget::SetTimeNavigationController(mitk::SliceNavigationController* timeNavigationController)
{
  m_TimeNavigationController = timeNavigationController;
  this->setEnabled(timeNavigationController != nullptr);
}
