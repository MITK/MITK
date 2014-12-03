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

#include "QmitkTimeSliceAnimationItem.h"
#include "QmitkTimeSliceAnimationWidget.h"
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStepper.h>
#include <ui_QmitkTimeSliceAnimationWidget.h>

static int GetNumberOfSlices()
{
  mitk::Stepper* stepper = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime();

  if (stepper != NULL)
    return std::max(1, static_cast<int>(stepper->GetSteps()));

  return 1;
}

QmitkTimeSliceAnimationWidget::QmitkTimeSliceAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkTimeSliceAnimationWidget)
{
  m_Ui->setupUi(this);

  this->connect(m_Ui->sliceRangeWidget, SIGNAL(minimumValueChanged(double)),
    this, SLOT(OnFromChanged(double)));

  this->connect(m_Ui->sliceRangeWidget, SIGNAL(maximumValueChanged(double)),
    this, SLOT(OnToChanged(double)));

  this->connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)),
    this, SLOT(OnReverseChanged(bool)));
}

QmitkTimeSliceAnimationWidget::~QmitkTimeSliceAnimationWidget()
{
}

void QmitkTimeSliceAnimationWidget::SetAnimationItem(QmitkAnimationItem* sliceAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkTimeSliceAnimationItem*>(sliceAnimationItem);

  if (m_AnimationItem == NULL)
    return;

  const int maximum = GetNumberOfSlices() - 1;
  const int from = std::min(m_AnimationItem->GetFrom(), maximum);
  const int to = std::min(m_AnimationItem->GetTo(), maximum);

  m_AnimationItem->SetFrom(from);
  m_AnimationItem->SetTo(to);

  m_Ui->sliceRangeWidget->setMaximum(maximum);
  m_Ui->sliceRangeWidget->setValues(from, to);
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}

void QmitkTimeSliceAnimationWidget::OnFromChanged(double from)
{
  if (m_AnimationItem == NULL)
    return;

  int intFrom = static_cast<int>(from);

  if (m_AnimationItem->GetFrom() != intFrom)
    m_AnimationItem->SetFrom(intFrom);
}

void QmitkTimeSliceAnimationWidget::OnToChanged(double to)
{
  if (m_AnimationItem == NULL)
    return;

  int intTo = static_cast<int>(to);

  if (m_AnimationItem->GetTo() != intTo)
    m_AnimationItem->SetTo(intTo);
}

void QmitkTimeSliceAnimationWidget::OnReverseChanged(bool reverse)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
