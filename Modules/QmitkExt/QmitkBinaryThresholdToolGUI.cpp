/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkBinaryThresholdToolGUI.h"
#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkBinaryThresholdToolGUI, "")

QmitkBinaryThresholdToolGUI::QmitkBinaryThresholdToolGUI()
:QmitkToolGUI(),
 m_Slider(NULL),
 m_Spinner(NULL)
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* label = new QLabel( "Threshold :", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  mainLayout->addWidget(label);


  QBoxLayout* layout = new QHBoxLayout();

  m_Spinner = new QSpinBox();
  m_Spinner->setMaximum(20);
  m_Spinner->setMinimum(5);
  m_Spinner->setValue(1);

  connect(m_Spinner, SIGNAL(editingFinished()), this, SLOT(OnSpinnerValueChanged()) );
  layout->addWidget(m_Spinner);

  //m_Slider = new QSlider( 5, 20, 1, 1, Qt::Horizontal, this );
  m_Slider = new QSlider( Qt::Horizontal, this );
  m_Slider->setMinimum(5);
  m_Slider->setMaximum(20);
  m_Slider->setPageStep(1);
  m_Slider->setValue(1);
  connect( m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
  layout->addWidget( m_Slider );

  QPushButton* okButton = new QPushButton("Ok", this);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnAcceptThresholdPreview()));
  okButton->setFont( f );
  layout->addWidget( okButton );

  mainLayout->addLayout(layout);

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkBinaryThresholdToolGUI::~QmitkBinaryThresholdToolGUI()
{
  // !!!
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, int, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
  }

}

void QmitkBinaryThresholdToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, int, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
  }

  m_BinaryThresholdTool = dynamic_cast<mitk::BinaryThresholdTool*>( tool );

  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged += mitk::MessageDelegate2<QmitkBinaryThresholdToolGUI, int, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged += mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, int>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
  }
}

void QmitkBinaryThresholdToolGUI::OnSliderValueChanged(int value)
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->SetThresholdValue( value );
  }
  m_Spinner->setValue(value);
}
void QmitkBinaryThresholdToolGUI::OnAcceptThresholdPreview()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this ); // needs a QWidget as parent, "this" is not QWidget
    dialog->setPrompt("What did you just segment?");
    int dialogReturnValue = dialog->exec();

    std::string organName = dialog->GetSegmentationName().toStdString();
    mitk::Color color     = dialog->GetColor();

    delete dialog;

    if ( dialogReturnValue != QDialog::Rejected ) // user clicked cancel or pressed Esc or something similar
    {
      m_BinaryThresholdTool->AcceptCurrentThresholdValue( organName, color );
    }
    else
    {
      m_BinaryThresholdTool->CancelThresholding();
    }
  }
}

void QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged(int lower, int upper)
{
  m_Slider->setRange(lower, upper);
  m_Spinner->setRange(lower, upper);
}

void QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged(int current)
{
  m_Slider->setValue(current);
  m_Spinner->setValue(current);
}

void QmitkBinaryThresholdToolGUI::OnSpinnerValueChanged()
{
  m_Slider->setValue(m_Spinner->value());
}
