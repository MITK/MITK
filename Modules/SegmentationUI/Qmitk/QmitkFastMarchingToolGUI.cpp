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

#include "QmitkFastMarchingToolGUI.h"

#include "mitkBaseRenderer.h"
#include "mitkFastMarchingTool.h"

#include <QmitkNewSegmentationDialog.h>
#include <QApplication.h>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkFastMarchingToolGUI, "")

QmitkFastMarchingToolGUI::QmitkFastMarchingToolGUI() : QmitkToolGUI(), m_SelfCall(false), m_FastMarchingTool(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  m_Controls.m_slSigma->setDecimals(2);
  m_Controls.m_slSigma->setPageStep(0.1);
  m_Controls.m_slSigma->setSingleStep(0.01);
  m_Controls.m_slSigma->setMinimum(0.1);
  m_Controls.m_slSigma->setMaximum(5.0);
  m_Controls.m_slSigma->setTracking(false);
  m_Controls.m_slSigma->setToolTip("The \"Sigma\" parameter in the Gradient Magnitude filter.");
  connect( m_Controls.m_slSigma, SIGNAL(valueChanged(double)), this, SLOT(OnSigmaChanged(double)));

  m_Controls.m_slAlpha->setDecimals(2);
  m_Controls.m_slAlpha->setMinimum(-10);
  m_Controls.m_slAlpha->setMaximum(0);
  m_Controls.m_slAlpha->setPageStep(0.1);
  m_Controls.m_slAlpha->setSingleStep(0.01);
  m_Controls.m_slAlpha->setTracking(false);
  m_Controls.m_slAlpha->setToolTip("The \"Alpha\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));

  m_Controls.m_slBeta->setDecimals(2);
  m_Controls.m_slBeta->setTracking(false);
  m_Controls.m_slBeta->setMinimum(0);
  m_Controls.m_slBeta->setMaximum(30);
  m_Controls.m_slBeta->setPageStep(0.1);
  m_Controls.m_slBeta->setSingleStep(0.01);
  m_Controls.m_slBeta->setTracking(false);
  m_Controls.m_slBeta->setToolTip("The \"Beta\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));

  m_Controls.m_slStopValue->setTickInterval(1);
  m_Controls.m_slStopValue->setMinimum(-100);
  m_Controls.m_slStopValue->setMaximum(3000);
  m_Controls.m_slStopValue->setDecimals(0);
  m_Controls.m_slStopValue->setTracking(false);
  m_Controls.m_slStopValue->setToolTip("The \"Stop Value\" parameter in the FastMarching filter.");
  connect( m_Controls.m_slStopValue, SIGNAL(valueChanged(double)), this, SLOT(OnStopValueChanged(double)));

  m_Controls.m_pbClearSeeds->setToolTip("Clear current preview and start over again");
  connect( m_Controls.m_pbClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_pbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_pbShowAdvancedTools, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFastMarchingToolGUI::~QmitkFastMarchingToolGUI()
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
    m_FastMarchingTool->SigmaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnSigmaValueSet );
    m_FastMarchingTool->AlphaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnAlphaValueSet );
    m_FastMarchingTool->BetaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnBetaValueSet );
    m_FastMarchingTool->StopValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnStopValueSet );
  }
}

void QmitkFastMarchingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
    m_FastMarchingTool->SigmaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnSigmaValueSet );
    m_FastMarchingTool->AlphaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnAlphaValueSet );
    m_FastMarchingTool->BetaValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnBetaValueSet );
    m_FastMarchingTool->StopValueChanged -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnStopValueSet );
  }

  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool*>( tool );

  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
    m_FastMarchingTool->SigmaValueChanged += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnSigmaValueSet );
    m_FastMarchingTool->AlphaValueChanged += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnAlphaValueSet );
    m_FastMarchingTool->BetaValueChanged += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnBetaValueSet );
    m_FastMarchingTool->StopValueChanged += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, mitk::ScalarType>( this, &QmitkFastMarchingToolGUI::OnStopValueSet );
  }
}

void QmitkFastMarchingToolGUI::OnSigmaValueSet(mitk::ScalarType value)
{
    m_SelfCall = true;
    m_Controls.m_slSigma->setDecimals(2);
    m_Controls.m_slSigma->setValue( value );
    m_SelfCall = false;
}

void QmitkFastMarchingToolGUI::OnAlphaValueSet(mitk::ScalarType value)
{
    m_SelfCall = true;
    m_Controls.m_slAlpha->setValue( value );
    m_Controls.m_slAlpha->setDecimals(2);
    m_SelfCall = false;
}

void QmitkFastMarchingToolGUI::OnBetaValueSet(mitk::ScalarType value)
{
    m_SelfCall = true;
    m_Controls.m_slBeta->setValue( value );
    m_Controls.m_slBeta->setDecimals(2);
    m_SelfCall = false;
}

void QmitkFastMarchingToolGUI::OnStopValueSet(mitk::ScalarType value)
{
    m_SelfCall = true;
    m_Controls.m_slStopValue->setValue( value );
    m_Controls.m_slStopValue->setDecimals(0);
    m_SelfCall = false;
}

void QmitkFastMarchingToolGUI::OnStopValueChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetStoppingValue( value );
    m_FastMarchingTool->Run();
  }
}

void QmitkFastMarchingToolGUI::OnBetaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetBeta( value );
    m_FastMarchingTool->Run();
  }
}

void QmitkFastMarchingToolGUI::OnSigmaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetSigma( value );
    m_FastMarchingTool->Run();
  }
}

void QmitkFastMarchingToolGUI::OnAlphaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetAlpha( value );
    m_FastMarchingTool->Run();
  }
}

void QmitkFastMarchingToolGUI::OnAcceptPreview()
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->AcceptPreview();
  }
}

void QmitkFastMarchingToolGUI::OnCancel()
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->Cancel();
  }
}

void QmitkFastMarchingToolGUI::OnClearSeeds()
{
  m_FastMarchingTool->ClearSeeds();
}

void QmitkFastMarchingToolGUI::BusyStateChanged(bool value)
{
  if (value)
    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
    QApplication::restoreOverrideCursor();
}

void QmitkFastMarchingToolGUI::OnShowInformation( bool value )
{
  if (value)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkFastMarchingToolGUI::OnShowAdvancedControls( bool value )
{
  if (value)
    m_Controls.m_AdvancedControlsWidget->show();
  else
    m_Controls.m_AdvancedControlsWidget->hide();
}

void QmitkFastMarchingToolGUI::OnNewLabel()
{
  if (m_FastMarchingTool)
  {
    QmitkNewSegmentationDialog dialog(this);
//    dialog->SetSuggestionList( m_OrganColors );
    dialog.setWindowTitle("New Label");
    int dialogReturnValue = dialog.exec();
    if ( dialogReturnValue == QDialog::Rejected ) return;
    mitk::Color color = dialog.GetColor();
    std::string name = dialog.GetSegmentationName().toStdString();
    m_FastMarchingTool->CreateNewLabel(name, color);
  }
}
