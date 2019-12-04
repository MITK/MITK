/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include <ui_QmitkOrbitAnimationWidget.h>

QmitkOrbitAnimationWidget::QmitkOrbitAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkOrbitAnimationWidget)
{
  m_Ui->setupUi(this);

  this->connect(m_Ui->orbitSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(OnOrbitChanged(int)));

  this->connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)),
    this, SLOT(OnReverseChanged(bool)));
}

QmitkOrbitAnimationWidget::~QmitkOrbitAnimationWidget()
{
}

void QmitkOrbitAnimationWidget::SetAnimationItem(QmitkAnimationItem* orbitAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkOrbitAnimationItem*>(orbitAnimationItem);

  if (m_AnimationItem == nullptr)
    return;

  m_Ui->orbitSpinBox->setValue(m_AnimationItem->GetOrbit());
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}


void QmitkOrbitAnimationWidget::OnOrbitChanged(int )
{
  if (m_AnimationItem == nullptr)
    return;

  if (m_AnimationItem->GetOrbit() != m_Ui->orbitSpinBox->value())
    m_AnimationItem->SetOrbit(m_Ui->orbitSpinBox->value());
}

void QmitkOrbitAnimationWidget::OnReverseChanged(bool reverse)
{
  if (m_AnimationItem == nullptr)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
