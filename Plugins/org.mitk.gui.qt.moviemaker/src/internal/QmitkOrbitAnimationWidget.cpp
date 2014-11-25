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
#include <limits>
#include <ui_QmitkOrbitAnimationWidget.h>

QmitkOrbitAnimationWidget::QmitkOrbitAnimationWidget(QWidget* parent)
  : QmitkAnimationWidget(parent),
    m_Ui(new Ui::QmitkOrbitAnimationWidget)
{
  m_Ui->setupUi(this);

  m_Ui->orbitLineEdit->setValidator(new QIntValidator(0, std::numeric_limits<int>::max(), this));

  this->connect(m_Ui->startAngleSlider, SIGNAL(valueChanged(int)),
    m_Ui->startAngleSpinBox, SLOT(setValue(int)));

  this->connect(m_Ui->startAngleSpinBox, SIGNAL(valueChanged(int)),
    m_Ui->startAngleSlider, SLOT(setValue(int)));

  this->connect(m_Ui->startAngleSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(OnStartAngleChanged(int)));

  this->connect(m_Ui->orbitLineEdit, SIGNAL(editingFinished()),
    this, SLOT(OnOrbitEditingFinished()));

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

  m_Ui->startAngleSlider->setValue(m_AnimationItem->GetStartAngle());
  m_Ui->orbitLineEdit->setText(QString("%1").arg(m_AnimationItem->GetOrbit()));
  m_Ui->reverseCheckBox->setChecked(m_AnimationItem->GetReverse());
}

void QmitkOrbitAnimationWidget::OnStartAngleChanged(int angle)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetStartAngle() != angle)
    m_AnimationItem->SetStartAngle(angle);
}

void QmitkOrbitAnimationWidget::OnOrbitEditingFinished()
{
  if (m_AnimationItem == NULL)
    return;

  int angle = m_Ui->orbitLineEdit->text().toInt();

  if (m_AnimationItem->GetOrbit() != angle)
    m_AnimationItem->SetOrbit(angle);
}

void QmitkOrbitAnimationWidget::OnReverseChanged(bool reverse)
{
  if (m_AnimationItem == NULL)
    return;

  if (m_AnimationItem->GetReverse() != reverse)
    m_AnimationItem->SetReverse(reverse);
}
