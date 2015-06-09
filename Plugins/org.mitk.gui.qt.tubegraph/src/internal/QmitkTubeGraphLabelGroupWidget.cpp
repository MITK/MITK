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

#include "QmitkTubeGraphLabelGroupWidget.h"

#include "QmitkTubeGraphLabelWidget.h"

QmitkTubeGraphLabelGroupWidget::QmitkTubeGraphLabelGroupWidget(QWidget* parent, const char* name)
:QWidget(parent)
{
  m_GroupLayout = new QVBoxLayout();
  this->SetGroupName(QString(name));
  this->setLayout(m_GroupLayout);
}

QmitkTubeGraphLabelGroupWidget::~QmitkTubeGraphLabelGroupWidget()
{
  //m_Labels.clear();
  delete m_GroupLayout;
}

void QmitkTubeGraphLabelGroupWidget::AddLabel(QString name, mitk::Color color)
{
  auto   label = new QmitkTubeGraphLabelWidget(this);
  label->SetLabelName(name);
  label->SetLabelColor(&color);

  m_GroupLayout->addWidget(label);

  connect(label, SIGNAL(SignalLabelVisibilityToggled(bool, QString)), this, SLOT(OnVisibilityToggled(bool, QString)));
  connect(label, SIGNAL(SignalLabelButtonClicked(QString)), this, SLOT(OnLabelButtonClicked(QString)));
  connect(label, SIGNAL(SignalLabelColorChanged(mitk::Color, QString)), this, SLOT(OnColoringButtonClicked(mitk::Color, QString)));
}

void QmitkTubeGraphLabelGroupWidget::SetGroupName(QString name)
{
  m_GroupName = name;
}

QString QmitkTubeGraphLabelGroupWidget::GetGroupName()
{
  return m_GroupName;
}

void QmitkTubeGraphLabelGroupWidget::OnVisibilityToggled(bool isVisible, QString labelName)
{
  emit SignalLabelVisibilityInGroupToggled(isVisible, labelName, this->GetGroupName());
}

void QmitkTubeGraphLabelGroupWidget::OnLabelButtonClicked(QString labelName)
{
  emit SignalLabelButtonInGroupClicked(labelName, this->GetGroupName());
}

void QmitkTubeGraphLabelGroupWidget::OnColoringButtonClicked(mitk::Color color, QString labelName)
{
  emit SignalLabelColorInGroupChanged(color, labelName, this->GetGroupName());
}
