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
#include "QmitkPixelManipulationToolGUI.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QRadioButton>

MITK_TOOL_GUI_MACRO (, QmitkPixelManipulationToolGUI, "");


QmitkPixelManipulationToolGUI::QmitkPixelManipulationToolGUI() : QmitkToolGUI()
{
  QBoxLayout* mainLayout = new QVBoxLayout(this);
  QRadioButton* radio1 = new QRadioButton("change the masked pixels by value: ", this);
  QRadioButton* radio2 = new QRadioButton("set the masked pixels to value: ", this);

  radio1->setChecked(true);

  connect(radio1, SIGNAL(toggled(bool)), this, SLOT(SetFixedValueOff(bool)));
  connect(radio2, SIGNAL(toggled(bool)), this, SLOT(SetFixedValueOn(bool)));

  mainLayout->addWidget(radio1);
  mainLayout->addWidget(radio2);

  m_Slider = new QSlider(Qt::Horizontal, this);
  m_Slider->setRange(-5000, 5000);
  m_Slider->setValue(0);

  connect(m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)) );

  m_Spinner = new QSpinBox(this);
  m_Spinner->setRange(-5000, 5000);
  m_Spinner->setValue(0);

  connect(m_Spinner, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxChanged()) );

  QBoxLayout* layout1 = new QHBoxLayout();
  layout1->addWidget(m_Slider);
  layout1->addWidget(m_Spinner);

  QPushButton* okButton = new QPushButton("Ok");

  connect(okButton, SIGNAL(clicked()), this, SLOT(OnOkButtonClicked()) );

  mainLayout->addLayout(layout1);
  mainLayout->addWidget(okButton);

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkPixelManipulationToolGUI::~QmitkPixelManipulationToolGUI()
{
  if (m_PixelManipulationTool.IsNotNull())
  {

  }
}

void QmitkPixelManipulationToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_PixelManipulationTool.IsNotNull())
  {
  }
  m_PixelManipulationTool = dynamic_cast<mitk::PixelManipulationTool*> (tool);
  if (m_PixelManipulationTool.IsNotNull())
  {
  }
}

void QmitkPixelManipulationToolGUI::SetFixedValueOff(bool flag)
{
  if (flag)
  {
    if (m_PixelManipulationTool.IsNotNull())
      m_PixelManipulationTool->FixedValueOff();
  }
}

void QmitkPixelManipulationToolGUI::SetFixedValueOn(bool flag)
{
  if (flag)
  {
    if (m_PixelManipulationTool.IsNotNull())
      m_PixelManipulationTool->FixedValueOn();
  }
}

void QmitkPixelManipulationToolGUI::OnSpinBoxChanged()
{
  m_Slider->setValue(m_Spinner->value());
}

void QmitkPixelManipulationToolGUI::OnSliderValueChanged(int value)
{
  if (m_PixelManipulationTool.IsNotNull())
  {
    m_PixelManipulationTool->SetValue(value);
    m_Spinner->setValue(value);
  }
}

void QmitkPixelManipulationToolGUI::OnOkButtonClicked()
{
  if (m_PixelManipulationTool.IsNotNull())
  {
    m_PixelManipulationTool->CalculateImage();
  }
}
