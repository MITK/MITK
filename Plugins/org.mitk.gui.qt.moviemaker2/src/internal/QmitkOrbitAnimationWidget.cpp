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

#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include <ui_QmitkOrbitAnimationWidget.h>

QmitkOrbitAnimationWidget::QmitkOrbitAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkOrbitAnimationWidget)
{
  m_Ui->setupUi(this);

  this->connect(m_Ui->angleSlider, SIGNAL(valueChanged(int)),
    m_Ui->angleSpinBox, SLOT(setValue(int)));

  this->connect(m_Ui->angleSpinBox, SIGNAL(valueChanged(int)),
    m_Ui->angleSlider, SLOT(setValue(int)));

  this->connect(m_Ui->angleSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(OnAngleChanged(int)));

  this->connect(m_Ui->reverseCheckBox, SIGNAL(clicked(bool)),
    this, SLOT(OnReverseChanged(bool)));
}

QmitkOrbitAnimationWidget::~QmitkOrbitAnimationWidget()
{
}

void QmitkOrbitAnimationWidget::SetAnimationItem(QmitkAnimationItem* orbitAnimationItem)
{
  m_AnimationItem = dynamic_cast<QmitkOrbitAnimationItem*>(orbitAnimationItem);

  if (m_AnimationItem == NULL)
    return;

  m_Ui->angleSlider->setValue(m_AnimationItem->GetAngle());
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}

void QmitkOrbitAnimationWidget::OnAngleChanged(int angle)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetAngle() != angle)
    m_AnimationItem->SetAngle(angle);
}

void QmitkOrbitAnimationWidget::OnReverseChanged(bool reverse)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
