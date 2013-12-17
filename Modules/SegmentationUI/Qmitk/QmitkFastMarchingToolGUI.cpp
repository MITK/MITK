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

#include <QMessageBox>
#include "mitkBaseRenderer.h"
#include "mitkFastMarchingTool.h"

#include <QmitkNewSegmentationDialog.h>
#include <QmitkStepperAdapter.h>
#include <QApplication.h>

MITK_TOOL_GUI_MACRO(SegmentationUI_EXPORT, QmitkFastMarchingToolGUI, "")

QmitkFastMarchingToolGUI::QmitkFastMarchingToolGUI() : QmitkToolGUI(), m_TimeIsConnected(false), m_SelfCall(false), m_FastMarchingTool(NULL)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();
  m_Controls.m_AdvancedControlsWidget->hide();

  m_Controls.m_slSigma->setPageStep(0.1);
//  m_slSigma->setTickInterval(0.01);
  m_Controls.m_slSigma->setSingleStep(0.01);
  m_Controls.m_slSigma->setMinimum(0.1);
  m_Controls.m_slSigma->setMaximum(5.0);
  m_Controls.m_slSigma->setTracking(false);
  m_Controls.m_slSigma->setToolTip("The \"sigma\" parameter in the Gradient Magnitude filter.");
  connect( m_Controls.m_slSigma, SIGNAL(valueChanged(double)), this, SLOT(OnSigmaChanged(double)));

  m_Controls.m_slAlpha->setMinimum(-10);
  m_Controls.m_slAlpha->setMaximum(0);
  m_Controls.m_slAlpha->setPageStep(0.1);
  m_Controls.m_slAlpha->setSingleStep(0.01);
  m_Controls.m_slAlpha->setTracking(false);
  m_Controls.m_slAlpha->setToolTip("The \"alpha\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));

  m_Controls.m_slBeta->setTracking(false);
//  m_slBeta->setOrientation(Qt::Horizontal);
  m_Controls.m_slBeta->setMinimum(0);
  m_Controls.m_slBeta->setMaximum(30);
  m_Controls.m_slBeta->setPageStep(0.1);
  m_Controls.m_slBeta->setSingleStep(0.01);
  m_Controls.m_slBeta->setTracking(false);
  m_Controls.m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));

  m_Controls.m_slStopValue->setTickInterval(1);
  m_Controls.m_slStopValue->setMinimum(-100);
  m_Controls.m_slStopValue->setMaximum(3000);
  m_Controls.m_slStopValue->setDecimals(0);
  m_Controls.m_slStopValue->setTracking(false);
  m_Controls.m_slStopValue->setToolTip("The \"stop value\" parameter in the FastMarching filter.");
  connect( m_Controls.m_slStopValue, SIGNAL(valueChanged(double)), this, SLOT(OnStopValueChanged(double)));

  m_Controls.m_pbClearSeeds->setToolTip("Clear current preview and start over again");
  connect( m_Controls.m_pbClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );
  connect( m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)) );
  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_cbShowAdvancedControls, SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedControls(bool)) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );

  m_Controls.m_slAlpha->setDecimals(2);
  m_Controls.m_slSigma->setDecimals(2);
  m_Controls.m_slBeta->setDecimals(2);
}

QmitkFastMarchingToolGUI::~QmitkFastMarchingToolGUI()
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
  }
}

void QmitkFastMarchingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
  }

  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool*>( tool );

  if (m_FastMarchingTool)
  {
    m_FastMarchingTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkFastMarchingToolGUI, bool>( this, &QmitkFastMarchingToolGUI::BusyStateChanged );
    m_SelfCall = true;
    m_Controls.m_slSigma->setValue( m_FastMarchingTool->GetSigma() );
    m_Controls.m_slAlpha->setValue( m_FastMarchingTool->GetAlpha() );
    m_Controls.m_slBeta->setValue( m_FastMarchingTool->GetBeta() );
    m_Controls.m_slStopValue->setValue( m_FastMarchingTool->GetStoppingValue() );
    m_SelfCall = false;
    //listen to timestep change events
    mitk::BaseRenderer::Pointer renderer;
    renderer = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") );
    if (renderer.IsNotNull() && !m_TimeIsConnected)
    {
        new QmitkStepperAdapter(this, renderer->GetSliceNavigationController()->GetTime(), "stepper");
      //  connect(m_TimeStepper, SIGNAL(Refetch()), this, SLOT(Refetch()));
        m_TimeIsConnected = true;
    }
  }
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
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->AcceptPreview();
  }
}

void QmitkFastMarchingToolGUI::OnCancel()
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->Cancel();
  }
}

void QmitkFastMarchingToolGUI::SetStepper(mitk::Stepper *stepper)
{
  this->m_TimeStepper = stepper;
}

void QmitkFastMarchingToolGUI::Refetch()
{
  //event from image navigator recieved - timestep has changed
//    m_FastMarchingTool->SetCurrentTimeStep(m_TimeStepper->GetPos());
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

void QmitkFastMarchingToolGUI::OnShowInformation( bool on )
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}

void QmitkFastMarchingToolGUI::OnShowAdvancedControls( bool on )
{
  if (on)
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
//    m_FastMarchingTool->CreateNewLabel(name, color);
  }
}
