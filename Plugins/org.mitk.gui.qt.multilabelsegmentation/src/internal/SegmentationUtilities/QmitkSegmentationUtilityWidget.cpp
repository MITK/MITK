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
