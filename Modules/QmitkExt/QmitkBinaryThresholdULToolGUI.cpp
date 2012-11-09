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

#include "QmitkBinaryThresholdULToolGUI.h"
#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkBinaryThresholdULToolGUI, "")

QmitkBinaryThresholdULToolGUI::QmitkBinaryThresholdULToolGUI()
:QmitkToolGUI(),
m_RangeSlider(NULL)
{
  // create the visible widgets
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QLabel* label = new QLabel( "Threshold :", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  mainLayout->addWidget(label);

  QBoxLayout* layout = new QHBoxLayout();

  m_LowerSpinner = new QSpinBox();
  m_LowerSpinner->setMinimum(-2048);
  m_LowerSpinner->setMaximum(0);
  m_LowerSpinner->setValue(-2048);
  connect(m_LowerSpinner, SIGNAL(editingFinished()), this, SLOT(OnOneSpinnerChanged()) );

  m_RangeSlider = new QxtSpanSlider(Qt::Horizontal, this );
  m_RangeSlider->setMaximum(2048);
  m_RangeSlider->setMinimum(-2048);
  m_RangeSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);

  m_UpperSpinner = new QSpinBox();
  m_UpperSpinner->setMinimum(0);
  m_UpperSpinner->setMaximum(2048);
  m_UpperSpinner->setValue(2048);

  connect(m_UpperSpinner, SIGNAL(editingFinished()), this, SLOT(OnOneSpinnerChanged()) );
  connect(m_RangeSlider, SIGNAL(spanChanged(int, int)  ),this, SLOT( OnSpanChanged(int , int ) ));

  layout->addWidget(m_LowerSpinner);
  layout->addWidget(m_RangeSlider);
  layout->addWidget(m_UpperSpinner);

  mainLayout->addLayout(layout);

  QPushButton* okButton = new QPushButton("Ok", this);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnAcceptThresholdPreview()));
  okButton->setFont( f );
  mainLayout->addWidget( okButton );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkBinaryThresholdULToolGUI::~QmitkBinaryThresholdULToolGUI()
{
  // !!!
  if (m_BinaryThresholdULTool.IsNotNull())
  {
    m_BinaryThresholdULTool->IntervalBordersChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdULTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesChanged );
  }

}

void QmitkBinaryThresholdULToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_BinaryThresholdULTool.IsNotNull())
  {
    m_BinaryThresholdULTool->IntervalBordersChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdULTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesChanged );
  }

  m_BinaryThresholdULTool = dynamic_cast<mitk::BinaryThresholdULTool*>( tool );

  if (m_BinaryThresholdULTool.IsNotNull())
  {
    m_BinaryThresholdULTool->IntervalBordersChanged += mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdULTool->ThresholdingValuesChanged += mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, int, int>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesChanged );
  }
}

void QmitkBinaryThresholdULToolGUI::OnAcceptThresholdPreview()
{
  if (m_BinaryThresholdULTool.IsNotNull())
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this ); // needs a QWidget as parent, "this" is not QWidget
    dialog->setPrompt("What did you just segment?");
    int dialogReturnValue = dialog->exec();

    std::string organName = dialog->GetSegmentationName().toLocal8Bit().data();
    mitk::Color color     = dialog->GetColor();

    delete dialog;

    if ( dialogReturnValue != QDialog::Rejected ) // user clicked cancel or pressed Esc or something similar
    {
      m_BinaryThresholdULTool->AcceptCurrentThresholdValue( organName, color );
    }
    else
    {
      m_BinaryThresholdULTool->CancelThresholding();
    }
  }
}

void QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersChanged(int lower, int upper)
{
  m_RangeSlider->setMaximum(upper);
  m_RangeSlider->setMinimum(lower);
  m_LowerSpinner->setMaximum(upper);
  m_LowerSpinner->setMinimum(lower);
  m_UpperSpinner->setMaximum(upper);
  m_UpperSpinner->setMinimum(lower);
}

void QmitkBinaryThresholdULToolGUI::OnThresholdingValuesChanged(int lower, int upper)
{
  m_RangeSlider->setSpan(lower, upper);
  m_LowerSpinner->setValue(lower);
  m_UpperSpinner->setValue(upper);
}

void QmitkBinaryThresholdULToolGUI::OnSpanChanged(int lower, int upper)
{
  if (upper < lower)
  {
    int tmp = upper;
    upper = lower;
    lower = tmp;
  }

  if (m_BinaryThresholdULTool.IsNotNull())
  {
    m_BinaryThresholdULTool->SetThresholdValues(lower, upper);
  }
  if (m_LowerSpinner->value() != lower)
    m_LowerSpinner->setValue(lower);
  if (m_UpperSpinner->value() != upper)
    m_UpperSpinner->setValue(upper);
}

void QmitkBinaryThresholdULToolGUI::OnOneSpinnerChanged()
{
  m_RangeSlider->setLowerValue(m_LowerSpinner->value());
  m_RangeSlider->setUpperValue(m_UpperSpinner->value());
}
