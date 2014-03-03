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
#include "mitkBinaryThresholdULTool.h"

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkBinaryThresholdULToolGUI, "")

QmitkBinaryThresholdULToolGUI::QmitkBinaryThresholdULToolGUI() : QmitkToolGUI(), m_BinaryThresholdULTool(NULL), m_SelfCall(false)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  m_Controls.m_DoubleThresholdSlider->setTracking(false);

  connect(m_Controls.m_DoubleThresholdSlider, SIGNAL(valuesChanged(double,double)), this, SLOT(OnThresholdsChanged(double, double)));
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkBinaryThresholdULToolGUI::~QmitkBinaryThresholdULToolGUI()
{
  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersSet );
    m_BinaryThresholdULTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesSet );
    m_BinaryThresholdULTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdULToolGUI, bool>( this, &QmitkBinaryThresholdULToolGUI::BusyStateChanged );
  }

}

void QmitkBinaryThresholdULToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersSet );
    m_BinaryThresholdULTool->ThresholdingValuesChanged -= mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesSet );
    m_BinaryThresholdULTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdULToolGUI, bool>( this, &QmitkBinaryThresholdULToolGUI::BusyStateChanged );
  }

  m_BinaryThresholdULTool = dynamic_cast<mitk::BinaryThresholdULTool*>( tool );

  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->IntervalBordersChanged += mitk::MessageDelegate3<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersSet );
    m_BinaryThresholdULTool->ThresholdingValuesChanged += mitk::MessageDelegate2<QmitkBinaryThresholdULToolGUI, mitk::ScalarType, mitk::ScalarType>( this, &QmitkBinaryThresholdULToolGUI::OnThresholdingValuesSet );
    m_BinaryThresholdULTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkBinaryThresholdULToolGUI, bool>( this, &QmitkBinaryThresholdULToolGUI::BusyStateChanged );
  }
}

void QmitkBinaryThresholdULToolGUI::OnCancel()
{
  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->Cancel();
  }
}

void QmitkBinaryThresholdULToolGUI::OnAcceptPreview()
{
  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->AcceptPreview();
  }
}

void QmitkBinaryThresholdULToolGUI::OnInvertPreview()
{
  if (m_BinaryThresholdULTool)
  {
    m_BinaryThresholdULTool->InvertPreview();
  }
}

void QmitkBinaryThresholdULToolGUI::OnThresholdingIntervalBordersSet(mitk::ScalarType lower, mitk::ScalarType upper, bool isFloat)
{
  m_SelfCall = true;
  m_Controls.m_DoubleThresholdSlider->setMinimum(lower);
  m_Controls.m_DoubleThresholdSlider->setMaximum(upper);
  if (isFloat)
    m_Controls.m_DoubleThresholdSlider->setDecimals(2);
  else
    m_Controls.m_DoubleThresholdSlider->setDecimals(0);
  m_SelfCall = false;
}

void QmitkBinaryThresholdULToolGUI::OnThresholdingValuesSet(mitk::ScalarType lower, mitk::ScalarType upper)
{
  m_SelfCall = true;
  m_Controls.m_DoubleThresholdSlider->setValues(lower, upper);
  m_SelfCall = false;
}

void QmitkBinaryThresholdULToolGUI::OnThresholdsChanged(double min, double max)
{
  if (m_BinaryThresholdULTool && !m_SelfCall)
  {
    m_BinaryThresholdULTool->SetThresholdValues(min, max);
    m_BinaryThresholdULTool->Run();
  }
}

void QmitkBinaryThresholdULToolGUI::OnShowAdvancedControls( bool on )
{
  if (on)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkBinaryThresholdULToolGUI::OnNewLabel()
{
  if (m_BinaryThresholdULTool)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_BinaryThresholdULTool->CreateNewLabel(name, color);
  }
}

void QmitkBinaryThresholdULToolGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkBinaryThresholdULToolGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}
