/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTimeSliceAnimationItem.h"
#include "QmitkTimeSliceAnimationWidget.h"
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkStepper.h>
#include <ui_QmitkTimeSliceAnimationWidget.h>

namespace
{
  int GetNumberOfSlices()
  {
    mitk::Stepper* stepper = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime();

    if (stepper != nullptr)
      return std::max(1, static_cast<int>(stepper->GetSteps()));

    return 1;
  }
}

QmitkTimeSliceAnimationWidget::QmitkTimeSliceAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkTimeSliceAnimationWidget)
{
  m_Ui->setupUi(this);

  connect(m_Ui->sliceRangeWidget, SIGNAL(minimumValueChanged(double)), this, SLOT(OnFromChanged(double)));
  connect(m_Ui->sliceRangeWidget, SIGNAL(maximumValueChanged(double)), this, SLOT(OnToChanged(double)));
  connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnReverseChanged(bool)));
}

QmitkTimeSliceAnimationWidget::~QmitkTimeSliceAnimationWidget()
{
}

void QmitkTimeSliceAnimationWidget::SetAnimationItem(QmitkAnimationItem* sliceAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkTimeSliceAnimationItem*>(sliceAnimationItem);

  if (nullptr == m_AnimationItem)
    return;

  const int maximum = GetNumberOfSlices() - 1;
  const int from = std::min(m_AnimationItem->GetFrom(), maximum);
  int to = std::max(from, std::min(m_AnimationItem->GetTo(), maximum));

  if (0 == to)
    to = maximum;

  m_AnimationItem->SetFrom(from);
  m_AnimationItem->SetTo(to);

  m_Ui->sliceRangeWidget->setMaximum(maximum);
  m_Ui->sliceRangeWidget->setValues(from, to);
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}

void QmitkTimeSliceAnimationWidget::OnFromChanged(double from)
{
  if (nullptr == m_AnimationItem)
    return;

  int intFrom = static_cast<int>(from);

  if (m_AnimationItem->GetFrom() != intFrom)
    m_AnimationItem->SetFrom(intFrom);
}

void QmitkTimeSliceAnimationWidget::OnToChanged(double to)
{
  if (nullptr == m_AnimationItem)
    return;

  int intTo = static_cast<int>(to);

  if (m_AnimationItem->GetTo() != intTo)
    m_AnimationItem->SetTo(intTo);
}

void QmitkTimeSliceAnimationWidget::OnReverseChanged(bool reverse)
{
  if (nullptr == m_AnimationItem)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
