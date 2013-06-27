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

#include "QmitkFastMarchingTool3DGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <ctkSliderWidget.h>
#include <ctkRangeWidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QGroupBox>
#include <QApplication>
#include <QMessageBox>
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"


MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkFastMarchingTool3DGUI, "")

QmitkFastMarchingTool3DGUI::QmitkFastMarchingTool3DGUI()
:QmitkToolGUI(),
m_TimeIsConnected(false)
{
  this->setContentsMargins( 0, 0, 0, 0 );

  // create the visible widgets
  QVBoxLayout *widgetLayout = new QVBoxLayout(this);
  widgetLayout->setContentsMargins(0, 0, 0, 0);

  QFont fntHelp;
  fntHelp.setBold(true);

  QLabel *lblHelp = new QLabel(this);
  lblHelp->setText("Press shift-click to add landmarks");
  lblHelp->setFont(fntHelp);

  widgetLayout->addWidget(lblHelp);

  QGroupBox *gbControls = new QGroupBox(this);
  gbControls->setCheckable(false);
  gbControls->setTitle(" Tool Controls ");

  // create the visible widgets
  QVBoxLayout *vlayout = new QVBoxLayout(gbControls);
  vlayout->setContentsMargins(5, 5, 5, 5);

  // Alpha controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Sigmoid Alpha: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addItem(hlayout);
  }

  m_slAlpha = new ctkSliderWidget(gbControls);
  m_slAlpha->setMinimum(-100);
  m_slAlpha->setMaximum(0);
  m_slAlpha->setPageStep(0.1);
  m_slAlpha->setValue(-0.5);
  m_slAlpha->setDecimals(1);
  m_slAlpha->setTracking(false);
  m_slAlpha->setToolTip("The \"alpha\" parameter in the Sigmoid mapping algorithm.");
  connect( m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));
  vlayout->addWidget( m_slAlpha );

  // Beta controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Sigmoid Beta: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
  }

  m_slBeta = new ctkSliderWidget(gbControls);
  m_slBeta->setMinimum(0);
  m_slBeta->setMaximum(1000);
  m_slBeta->setPageStep(0.1);
  m_slBeta->setValue(3.0);
  m_slBeta->setDecimals(1);
  m_slBeta->setTracking(false);
  m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping algorithm.");
  connect( m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));
  vlayout->addWidget( m_slBeta );

  // stopping value controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Stopping value: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
  }

  m_slStoppingValue = new ctkSliderWidget(gbControls);
  m_slStoppingValue->setMinimum(0);
  m_slStoppingValue->setMaximum(10000);
  m_slStoppingValue->setPageStep(1);
  m_slStoppingValue->setValue(100);
  m_slStoppingValue->setDecimals(0);
  m_slStoppingValue->setTracking(false);
  m_slStoppingValue->setToolTip("The \"stopping value\" parameter in the fast marching 3D algorithm");
  connect( m_slStoppingValue, SIGNAL(valueChanged(double)), this, SLOT(OnStoppingValueChanged(double)));
  vlayout->addWidget( m_slStoppingValue );

  // threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
  }

  m_slwThreshold = new ctkRangeWidget(gbControls);
  m_slwThreshold->setMinimum(-100);
  m_slwThreshold->setMaximum(5000);
  m_slwThreshold->setMinimumValue(0);
  m_slwThreshold->setMaximumValue(200);
  m_slwThreshold->setDecimals(0);
  m_slwThreshold->setTracking(false);
  m_slwThreshold->setToolTip("The lower and upper thresholds for the final thresholding");
  connect( m_slwThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(OnThresholdChanged(double, double)));
  vlayout->addWidget( m_slwThreshold );

  widgetLayout->addWidget(gbControls);

  m_btClearSeeds = new QPushButton("Clear");
  m_btClearSeeds->setToolTip("Clear current result and start over again");
  widgetLayout->addWidget(m_btClearSeeds);
  connect( m_btClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  m_btConfirm = new QPushButton("Accept");
  m_btConfirm->setToolTip("Incorporate current result in your working session.");
  widgetLayout->addWidget(m_btConfirm);
  connect( m_btConfirm, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFastMarchingTool3DGUI::~QmitkFastMarchingTool3DGUI()
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );
  }
}

void QmitkFastMarchingTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );
  }

  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool3D*>( tool );

  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );

    // set tool´s default values
    m_FastMarchingTool->SetLowerThreshold( this->m_slwThreshold->minimumValue());
    m_FastMarchingTool->SetUpperThreshold( this->m_slwThreshold->maximumValue());
    m_FastMarchingTool->SetStoppingValue( this->m_slStoppingValue->value());
    m_FastMarchingTool->SetSigma( this->m_slSigma->value());
    m_FastMarchingTool->SetAlpha( this->m_slAlpha->value());
    m_FastMarchingTool->SetBeta( this->m_slBeta->value());
  }

  //listen to timestep change events
  mitk::BaseRenderer::Pointer renderer;
  renderer = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") );
  if (renderer.IsNotNull() && !m_TimeIsConnected)
  {
    mitk::Stepper * stepper = renderer->GetSliceNavigationController()->GetTime();
    m_TimeStepper = new QmitkStepperAdapter(this, stepper, "exampleStepper");

    connect(m_TimeStepper, SIGNAL(Refetch()), this, SLOT(OnStepperRefetch()));

    m_TimeIsConnected = true;
  }
}

void QmitkFastMarchingTool3DGUI::OnThresholdChanged(double lower, double upper)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLowerThreshold( lower );
    m_FastMarchingTool->SetUpperThreshold( upper );
    m_FastMarchingTool->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnBetaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetBeta( value );
    m_FastMarchingTool->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnAlphaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetAlpha( value );
    m_FastMarchingTool->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnStoppingValueChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetStoppingValue( value );
    m_FastMarchingTool->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnConfirmSegmentation()
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->ConfirmSegmentation();
  }
}

void QmitkFastMarchingTool3DGUI::OnStepperRefetch()
{
  //event from image navigator recieved - timestep has changed
   m_FastMarchingTool->SetCurrentTimeStep(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep());
}

void QmitkFastMarchingTool3DGUI::OnClearSeeds()
{
  //event from image navigator recieved - timestep has changed
   m_FastMarchingTool->ClearSeeds();
   m_FastMarchingTool->Update();
}

void QmitkFastMarchingTool3DGUI::OnToolErrorMessage(std::string s)
{
  QMessageBox::warning(NULL, "Fast Marching 3D Tool", QString( s.c_str() ), QMessageBox::Ok, QMessageBox::NoButton);
}


void QmitkFastMarchingTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
      QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
      QApplication::restoreOverrideCursor();
}
