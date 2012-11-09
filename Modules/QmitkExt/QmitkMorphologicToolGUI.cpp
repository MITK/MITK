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
#include "QmitkMorphologicToolGUI.h"
#include "QmitkNewSegmentationDialog.h"
#include <QBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>

MITK_TOOL_GUI_MACRO( , QmitkMorphologicToolGUI, "")

QmitkMorphologicToolGUI::QmitkMorphologicToolGUI(): QmitkToolGUI(),
m_Slider(NULL)
{
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QBoxLayout* layout1 = new QHBoxLayout();
  QBoxLayout* layout2 = new QHBoxLayout();
  QBoxLayout* layout3 = new QHBoxLayout();

  QLabel* label = new QLabel( "radius:", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  layout1->addWidget(label);

  m_Slider = new QSlider(Qt::Horizontal, this );
  m_Slider->setMaximum(15);
  m_Slider->setMinimum(0);
  m_Slider->setPageStep(1);
  connect( m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
  m_Slider->setValue( 0 );
  layout1->addWidget( m_Slider );

  m_SpinBox = new QSpinBox(this);
  m_SpinBox->setSuffix(tr(" pixel"));
  m_SpinBox->setMaximum(15);
  m_SpinBox->setMinimum(0);
  m_SpinBox->setSingleStep(1);
  connect(m_SpinBox, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxValueChanged()) );
  m_SpinBox->setValue(0);
  layout1->addWidget(m_SpinBox);

  QLabel* label1 = new QLabel("structuring element:", this);
  QButtonGroup* group = new QButtonGroup(this);
  QRadioButton* crossRadioButton = new QRadioButton("cross");
  QRadioButton* ballRadioButton = new QRadioButton("ball");
  ballRadioButton->setChecked(true);

  group->addButton(ballRadioButton, 1);
  group->addButton(crossRadioButton, 2);

  layout3->addWidget(label1);
  layout3->addWidget(ballRadioButton);
  layout3->addWidget(crossRadioButton);
  connect(group, SIGNAL(buttonClicked(int)), this, SLOT(OnStructElementChanged(int)) );

  m_CheckBox = new QCheckBox("Preview" ,this);
  m_CheckBox->setCheckState(Qt::Checked);
  connect(m_CheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckStateChanged(int)) );

  layout2->addWidget(m_CheckBox);

  QPushButton* okButton = new QPushButton("OK", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  layout2->addWidget(okButton);

  mainLayout->addLayout(layout3);
  mainLayout->addLayout(layout1);
  mainLayout->addLayout(layout2);

   connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkMorphologicToolGUI::~QmitkMorphologicToolGUI()
{

}

void QmitkMorphologicToolGUI::OnSliderValueChanged(int value)
{
  m_SpinBox->setValue(value);
  if (m_MorphologicTool.IsNotNull())
    m_MorphologicTool->SetRadius(value);
}

void QmitkMorphologicToolGUI::OnSpinBoxValueChanged()
{
  m_Slider->setValue(m_SpinBox->value());
}

void QmitkMorphologicToolGUI::OnCheckStateChanged(int state)
{
  if (m_MorphologicTool.IsNotNull())
    m_MorphologicTool->SetPreview(state);
}

void QmitkMorphologicToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_MorphologicTool = dynamic_cast<mitk::MorphologicTool*>( tool );
}

void QmitkMorphologicToolGUI::OnAcceptPreview()
{
  if (m_MorphologicTool.IsNotNull())
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );
    dialog->setPrompt("What did you just segment?");
    int dialogReturnValue = dialog->exec();

    std::string name = dialog->GetSegmentationName().toLocal8Bit().data();
    mitk::Color color     = dialog->GetColor();

    delete dialog;

    if ( dialogReturnValue != QDialog::Rejected )
    {
      m_MorphologicTool->AcceptPreview( name, color );
    }
    else
    {
      m_MorphologicTool->CancelPreviewing();
    }
  }
}

void QmitkMorphologicToolGUI::OnStructElementChanged(int id)
{
  if (m_MorphologicTool.IsNotNull())
    m_MorphologicTool->SetStructuringElementType(id);

}
