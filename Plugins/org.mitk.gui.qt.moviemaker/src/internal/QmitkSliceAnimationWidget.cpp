/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include <mitkBaseRenderer.h>
#include <ui_QmitkSliceAnimationWidget.h>

namespace
{
  int GetNumberOfSlices(int renderWindow)
  {
    const QString renderWindowName = QString("stdmulti.widget%1").arg(renderWindow);
    vtkRenderWindow* theRenderWindow = mitk::BaseRenderer::GetRenderWindowByName(renderWindowName.toStdString());

    if (theRenderWindow != nullptr)
    {
      mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(theRenderWindow)->GetSliceNavigationController()->GetSlice();

      if (stepper != nullptr)
        return std::max(1, static_cast<int>(stepper->GetSteps()));
    }

    return 1;
  }
}

QmitkSliceAnimationWidget::QmitkSliceAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkSliceAnimationWidget),
    m_AnimationItem(nullptr)
{
  m_Ui->setupUi(this);

  connect(m_Ui->windowComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnRenderWindowChanged(int)));
  connect(m_Ui->sliceRangeWidget, SIGNAL(minimumValueChanged(double)), this, SLOT(OnFromChanged(double)));
  connect(m_Ui->sliceRangeWidget, SIGNAL(maximumValueChanged(double)), this, SLOT(OnToChanged(double)));
  connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnReverseChanged(bool)));
}

QmitkSliceAnimationWidget::~QmitkSliceAnimationWidget()
{
}

void QmitkSliceAnimationWidget::SetAnimationItem(QmitkAnimationItem* sliceAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkSliceAnimationItem*>(sliceAnimationItem);

  if (nullptr == m_AnimationItem)
    return;

  m_Ui->windowComboBox->setCurrentIndex(m_AnimationItem->GetRenderWindow());

  const int maximum = GetNumberOfSlices(m_AnimationItem->GetRenderWindow()) - 1;
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

void QmitkSliceAnimationWidget::OnRenderWindowChanged(int renderWindow)
{
  if (nullptr == m_AnimationItem)
    return;

  const int lastSlice = static_cast<int>(GetNumberOfSlices(renderWindow) - 1);

  if (lastSlice < m_AnimationItem->GetFrom())
    m_AnimationItem->SetFrom(lastSlice);

  if (lastSlice < m_AnimationItem->GetTo())
    m_AnimationItem->SetTo(lastSlice);

  m_Ui->sliceRangeWidget->setMaximum(lastSlice);
  m_Ui->sliceRangeWidget->setValues(m_AnimationItem->GetFrom(), m_AnimationItem->GetTo());

  if (m_AnimationItem->GetRenderWindow() != renderWindow)
    m_AnimationItem->SetRenderWindow(renderWindow);
}

void QmitkSliceAnimationWidget::OnFromChanged(double from)
{
  if (nullptr == m_AnimationItem)
    return;

  int intFrom = static_cast<int>(from);

  if (m_AnimationItem->GetFrom() != intFrom)
    m_AnimationItem->SetFrom(intFrom);
}

void QmitkSliceAnimationWidget::OnToChanged(double to)
{
  if (nullptr == m_AnimationItem)
    return;

  int intTo = static_cast<int>(to);

  if (m_AnimationItem->GetTo() != intTo)
    m_AnimationItem->SetTo(intTo);
}

void QmitkSliceAnimationWidget::OnReverseChanged(bool reverse)
{
  if (nullptr == m_AnimationItem)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
