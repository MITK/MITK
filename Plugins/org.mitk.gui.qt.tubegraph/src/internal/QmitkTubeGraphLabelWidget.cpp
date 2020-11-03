/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTubeGraphLabelWidget.h"

#include <QHBoxLayout>
#include <QColorDialog>

QmitkTubeGraphLabelWidget::QmitkTubeGraphLabelWidget(QWidget* parent)
:QWidget(parent)
{
  this->InitWidget();
}

QmitkTubeGraphLabelWidget::~QmitkTubeGraphLabelWidget()
{
  delete m_VisibilityCheckBox;
  delete m_LabelPushButton;
  delete m_ColoringPushButton;
}

void QmitkTubeGraphLabelWidget::InitWidget()
{

  m_VisibilityCheckBox = new QCheckBox("", this);
  m_VisibilityCheckBox->setChecked(true);
  m_VisibilityCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_LabelPushButton = new QPushButton("", this);
  //m_LabelPushButton->setCheckable(true);
  //m_LabelPushButton->setAutoExclusive(true);

  m_ColoringPushButton = new QPushButton(this);
  m_ColoringPushButton->setStyleSheet( QString("* { background-color: rgb(0,0,0)}"));
  m_ColoringPushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto   layout =  new QHBoxLayout;

  layout->addWidget(m_VisibilityCheckBox);
  layout->addWidget(m_LabelPushButton);
  layout->addWidget(m_ColoringPushButton);

  this->setLayout(layout);

  connect(m_VisibilityCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnVisibilityToggled(bool)));
  connect(m_LabelPushButton, SIGNAL(clicked()), this, SLOT(OnLabelButtonClicked()));

  connect(m_ColoringPushButton, SIGNAL(clicked()), this, SLOT(OnColoringButtonClicked()));
  //connect(m_ColoringPushButton, SIGNAL(clicked()), this, SIGNAL(SignalLabelColorChanged(this->GetLabelName())));
}

void QmitkTubeGraphLabelWidget::SetLabelName(QString name)
{
  m_LabelPushButton->setText(name);
}

QString QmitkTubeGraphLabelWidget::GetLabelName()
{
  return m_LabelPushButton->text();
}

void QmitkTubeGraphLabelWidget::SetLabelColor(mitk::Color* color)
{
  m_ColoringPushButton->setStyleSheet( QString("* { background-color: rgb(%1,%2,%3)}").arg(color->GetRed()).arg(color->GetGreen()).arg(color->GetBlue()) );
}

mitk::Color* QmitkTubeGraphLabelWidget::GetLabelColor()
{
  //return the background color of the button
  const QColor color = m_ColoringPushButton->palette().color(QPalette::Button);
  auto   rgb = new mitk::Color();
  rgb[0] = color.red(); rgb[1] = color.green(); rgb[2] = color.blue();
  return rgb;
}

void QmitkTubeGraphLabelWidget::OnVisibilityToggled(bool isVisible)
{
  emit SignalLabelVisibilityToggled(isVisible, this->GetLabelName());
}

void QmitkTubeGraphLabelWidget::OnLabelButtonClicked()
{
  emit SignalLabelButtonClicked(this->GetLabelName());
}

void QmitkTubeGraphLabelWidget::OnColoringButtonClicked()
{
  QColor usersChoice = QColorDialog::getColor();
  mitk::Color color;

  if (usersChoice.spec() != 0)
  {
    color[0] = usersChoice.red();
    color[1] = usersChoice.green();
    color[2] = usersChoice.blue();

    this->SetLabelColor(&color);
    emit SignalLabelColorChanged(color, this->GetLabelName());
  }
}
