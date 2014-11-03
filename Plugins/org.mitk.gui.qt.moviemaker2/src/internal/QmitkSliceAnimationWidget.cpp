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

#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include <ui_QmitkSliceAnimationWidget.h>

QmitkSliceAnimationWidget::QmitkSliceAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkSliceAnimationWidget)
{
  m_Ui->setupUi(this);

  this->connect(m_Ui->windowComboBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(OnRenderWindowChanged(int)));

  this->connect(m_Ui->sliceRangeWidget, SIGNAL(minimumValueChanged(double)),
    this, SLOT(OnFromChanged(double)));

  this->connect(m_Ui->sliceRangeWidget, SIGNAL(maximumValueChanged(double)),
    this, SLOT(OnToChanged(double)));

  this->connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)),
    this, SLOT(OnReverseChanged(bool)));
}

QmitkSliceAnimationWidget::~QmitkSliceAnimationWidget()
{
}

void QmitkSliceAnimationWidget::SetAnimationItem(QmitkAnimationItem* sliceAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkSliceAnimationItem*>(sliceAnimationItem);

  if (m_AnimationItem == NULL)
    return;

  m_Ui->windowComboBox->setCurrentIndex(m_AnimationItem->GetRenderWindow());
  m_Ui->sliceRangeWidget->setValues(m_AnimationItem->GetFrom(), m_AnimationItem->GetTo());
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}

void QmitkSliceAnimationWidget::OnRenderWindowChanged(int renderWindow)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetRenderWindow() != renderWindow)
    m_AnimationItem->SetRenderWindow(renderWindow);
}

void QmitkSliceAnimationWidget::OnFromChanged(double from)
{
  if (m_AnimationItem == NULL)
    return;

  int intFrom = static_cast<int>(from);

  if (m_AnimationItem->GetFrom() != intFrom)
    m_AnimationItem->SetFrom(intFrom);
}

void QmitkSliceAnimationWidget::OnToChanged(double to)
{
  if (m_AnimationItem == NULL)
    return;

  int intTo = static_cast<int>(to);

  if (m_AnimationItem->GetTo() != intTo)
    m_AnimationItem->SetTo(intTo);
}

void QmitkSliceAnimationWidget::OnReverseChanged(bool reverse)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
