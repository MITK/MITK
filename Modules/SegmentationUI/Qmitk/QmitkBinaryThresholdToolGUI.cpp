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
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkBinaryThresholdToolGUI, "")

QmitkBinaryThresholdToolGUI::QmitkBinaryThresholdToolGUI() : QmitkToolGUI(), m_BinaryThresholdTool(NULL), m_SelfCall(false)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  m_Controls.m_swThreshold->setTracking(true);

  connect( m_Controls.m_swThreshold, SIGNAL(valueChanged(double)), this, SLOT(OnThresholdSliderValueChanged(double)));
//  connect( m_Controls.m_pbRun, SIGNAL(clicked()), this, SLOT(OnRun()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
//  connect( m_Controls.m_pbDifference, SIGNAL(clicked()), this, SLOT(OnInvertPreview()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkBinaryThresholdToolGUI::~QmitkBinaryThresholdToolGUI()
{
  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, mitk::ScalarType>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }
}

void QmitkBinaryThresholdToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->IntervalBordersChanged -= mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, mitk::ScalarType>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }

  m_BinaryThresholdTool = dynamic_cast<mitk::BinaryThresholdTool*>( tool );

  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->IntervalBordersChanged += mitk::MessageDelegate3<QmitkBinaryThresholdToolGUI, mitk::ScalarType, mitk::ScalarType, bool>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged );
    m_BinaryThresholdTool->ThresholdingValueChanged += mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, mitk::ScalarType>( this, &QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged );
    m_BinaryThresholdTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkBinaryThresholdToolGUI, bool>( this, &QmitkBinaryThresholdToolGUI::BusyStateChanged );
  }
}

void QmitkBinaryThresholdToolGUI::OnThresholdSliderValueChanged(double value)
{
  if (m_BinaryThresholdTool && !m_SelfCall)
  {
    m_BinaryThresholdTool->SetThresholdValue(value);
  }
}

void QmitkBinaryThresholdToolGUI::OnCancel()
{
  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->Cancel();
  }
}

void QmitkBinaryThresholdToolGUI::OnAcceptPreview()
{
  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->AcceptPreview();
//    m_BinaryThresholdTool->Cancel();
  }
}

void QmitkBinaryThresholdToolGUI::OnInvertPreview()
{
  if (m_BinaryThresholdTool)
  {
    m_BinaryThresholdTool->InvertPreview();
  }
}

void QmitkBinaryThresholdToolGUI::OnThresholdingIntervalBordersChanged(mitk::ScalarType lower, mitk::ScalarType upper, bool isFloat)
{
  m_SelfCall = true;
  m_Controls.m_swThreshold->setRange(lower, upper);
  if (isFloat)
    m_Controls.m_swThreshold->setDecimals(2);
  else
    m_Controls.m_swThreshold->setDecimals(0);
  m_SelfCall = false;
}

void QmitkBinaryThresholdToolGUI::OnThresholdingValueChanged(mitk::ScalarType value)
{
  m_SelfCall = true;
  m_Controls.m_swThreshold->setValue(value);
  m_SelfCall = false;
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
  if (m_BinaryThresholdTool)
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
