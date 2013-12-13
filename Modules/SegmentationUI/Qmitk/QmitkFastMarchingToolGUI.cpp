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
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"

#include <QmitkNewSegmentationDialog.h>
#include <QmitkStepperAdapter.h>
#include <QApplication.h>
#include "mitkFastMarchingTool.h"

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
  m_Controls.m_slSigma->setValue(1.0);
  m_Controls.m_slSigma->setTracking(false);
  m_Controls.m_slSigma->setToolTip("The \"sigma\" parameter in the Gradient Magnitude filter.");
  connect( m_Controls.m_slSigma, SIGNAL(valueChanged(double)), this, SLOT(OnSigmaChanged(double)));

  m_Controls.m_slAlpha->setMinimum(-10);
  m_Controls.m_slAlpha->setMaximum(0);
  m_Controls.m_slAlpha->setPageStep(0.1);
  m_Controls.m_slAlpha->setSingleStep(0.01);
  m_Controls.m_slAlpha->setValue(-2.5);
  m_Controls.m_slAlpha->setTracking(false);
  m_Controls.m_slAlpha->setToolTip("The \"alpha\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));

  m_Controls.m_slBeta->setTracking(false);
//  m_slBeta->setOrientation(Qt::Horizontal);
  m_Controls.m_slBeta->setMinimum(0);
  m_Controls.m_slBeta->setMaximum(100);
  m_Controls.m_slBeta->setPageStep(0.1);
  m_Controls.m_slBeta->setSingleStep(0.01);
  m_Controls.m_slBeta->setValue(3.5);
  m_Controls.m_slBeta->setTracking(false);
  m_Controls.m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping filter.");
  connect( m_Controls.m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));

  m_Controls.m_slwThreshold->setMinimum(-100);
  m_Controls.m_slwThreshold->setMaximum(5000);
  m_Controls.m_slwThreshold->setTickInterval(1);
  m_Controls.m_slwThreshold->setMinimumValue(-100);
  m_Controls.m_slwThreshold->setMaximumValue(2000);
  m_Controls.m_slwThreshold->setDecimals(0);
  m_Controls.m_slwThreshold->setTracking(false);
  m_Controls.m_slwThreshold->setToolTip("The lower and upper thresholds for the final thresholding");
  connect( m_Controls.m_slwThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(OnThresholdChanged(double, double)));

  m_Controls.m_pbClearSeeds->setToolTip("Clear current preview and start over again");
  connect( m_Controls.m_pbClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  connect( m_Controls.m_pbAcceptPreview, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()) );
  connect( m_Controls.m_pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()) );

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

void QmitkFastMarchingToolGUI::Update()
{
  m_FastMarchingTool->SetLowerThreshold( m_Controls.m_slwThreshold->minimumValue());
  m_FastMarchingTool->SetUpperThreshold( m_Controls.m_slwThreshold->maximumValue());
//  m_FastMarchingTool->SetStoppingValue( this->m_slStoppingValue->value());
  m_FastMarchingTool->SetSigma( m_Controls.m_slSigma->value());
  m_FastMarchingTool->SetAlpha( m_Controls.m_slAlpha->value());
  m_FastMarchingTool->SetBeta( m_Controls.m_slBeta->value());
  m_FastMarchingTool->Run();
}

void QmitkFastMarchingToolGUI::OnThresholdChanged(double lower, double upper)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetLowerThreshold( lower );
    m_FastMarchingTool->SetUpperThreshold( upper );
    this->Update();
  }
}

void QmitkFastMarchingToolGUI::OnBetaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetBeta( value );
    this->Update();
  }
}

void QmitkFastMarchingToolGUI::OnSigmaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetSigma( value );
    this->Update();
  }
}

void QmitkFastMarchingToolGUI::OnAlphaChanged(double value)
{
  if (m_FastMarchingTool && (!m_SelfCall))
  {
    m_FastMarchingTool->SetAlpha( value );
    this->Update();
  }
}

void QmitkFastMarchingToolGUI::OnStoppingValueChanged(double value)
{
  /*
  if (m_FastMarchingTool.IsNotNull() && (!m_SelfCall))
  {
    m_FastMarchingTool->SetStoppingValue( value );
    this->Update();
  }
  */
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
  this->Update();
}

void QmitkFastMarchingToolGUI::BusyStateChanged(bool value)
{
  if (value)
      QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
      QApplication::restoreOverrideCursor();
}
