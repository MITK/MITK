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

#include "QmitkBinaryThresholdToolGUI.h"
#include "QmitkNewSegmentationDialog.h"

#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkBinaryThresholdToolGUI, "")

QmitkBinaryThresholdToolGUI::QmitkBinaryThresholdToolGUI()
:QmitkToolGUI(),
 m_isFloat(false),
 m_RangeMin(0),
 m_RangeMax(0),
 m_ChangingSlider(false),
 m_ChangingSpinner(false)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  connect( m_Controls.m_Spinner, SIGNAL(valueChanged(double)), this, SLOT(OnSpinnerValueChanged()) );
  connect( m_Controls.m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));

//  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkBinaryThresholdToolGUI::~QmitkBinaryThresholdToolGUI()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, double>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }
}

void QmitkBinaryThresholdToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, double>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }

  m_BinaryThresholdTool = dynamic_cast<mitk::BinaryThresholdTool*>( tool );

  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->IntervalBordersChanged += mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, double, double, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged += mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, double>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }
}

void QmitkBinaryThresholdToolGUI::OnSpinnerValueChanged()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_ChangingSpinner = true;
    double doubleVal = m_Controls.m_Spinner->value();
    int intVal = this->DoubleToSliderInt(doubleVal);
    m_BinaryThresholdTool->SetThresholdValue( doubleVal );
    if (m_ChangingSlider == false)
       m_Controls.m_Slider->setValue( intVal );
    m_ChangingSpinner = false;
  }
}

void QmitkBinaryThresholdToolGUI::OnSliderValueChanged(int value)
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
     m_ChangingSlider = true;
    double doubleVal = SliderIntToDouble(value);
    if (m_ChangingSpinner == false)
      m_Controls.m_Spinner->setValue(doubleVal);
    m_ChangingSlider = false;
  }
}

void QmitkBinaryThresholdToolGUI::OnCancel()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->Cancel();
  }
}

void QmitkBinaryThresholdToolGUI::OnAcceptPreview()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->AcceptPreview();
  }
}

void QmitkBinaryThresholdToolGUI::OnInvertPreview()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    m_BinaryThresholdTool->InvertPreview();
  }
}

void QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat)
{
  m_isFloat = isFloat;
  m_RangeMin = lower;
  m_RangeMax = upper;

  m_Controls.m_Spinner->setRange(lower, upper);
  if (!m_isFloat)
  {
    m_Controls.m_Slider->setRange(int(lower), int(upper));
    m_Controls.m_Spinner->setDecimals(0);
    m_Controls.m_Spinner->setSingleStep(1);
  }
  else
  {
    m_Controls.m_Slider->setRange(0, 99);
    m_Controls.m_Spinner->setDecimals(2);
    m_Range = upper-lower;
    m_Controls.m_Spinner->setSingleStep(m_Range/100);
  }

}

void QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged(double current)
{
  m_Controls.m_Slider->setValue(DoubleToSliderInt(current));
  m_Controls.m_Spinner->setValue(current);
}

double QmitkBinaryThresholdToolGUI::SliderIntToDouble(int val)
{
   if (!m_isFloat)
   {
      return double(val);
   }
   else
   {
      return double(val*(m_Range)/100 + m_RangeMin);
   }
}

int QmitkBinaryThresholdToolGUI::DoubleToSliderInt(double val)
{
   if (!m_isFloat)
   {
      return int(val);
   }
   else
   {
      int intVal = int( ((val-m_RangeMin) / m_Range)*100);
      return intVal;
   }
}

void QmitkBinaryThresholdToolGUI::BusyStateChanged(bool value)
{
  if (value)
      QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
      QApplication::restoreOverrideCursor();
}

void QmitkBinaryThresholdToolGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkBinaryThresholdToolGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkBinaryThresholdToolGUI::OnNewLabel()
{
  if (m_BinaryThresholdTool.IsNotNull())
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_BinaryThresholdTool->CreateNewLabel(name, color);
  }
}
