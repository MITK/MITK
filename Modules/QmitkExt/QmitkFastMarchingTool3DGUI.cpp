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
#include <qpushbutton.h>
#include <qlayout.h>
#include <QGroupBox>
//#include <qpainter.h>
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
  m_slAlpha->setPageStep(1);
  m_slAlpha->setValue(-10);
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
  m_slBeta->setPageStep(1);
  m_slBeta->setValue(50);
  m_slBeta->setTracking(false);
  m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping algorithm.");
  connect( m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));
  vlayout->addWidget( m_slBeta );

  // stopping value controls
  QLabel* labelStoppingValue = new QLabel(gbControls);
  labelStoppingValue->setText("Stopping value: ");
  vlayout->addWidget( labelStoppingValue );

  m_slStoppingValue = new ctkSliderWidget(gbControls);
  m_slStoppingValue->setMinimum(0);
  m_slStoppingValue->setMaximum(10000);
  m_slStoppingValue->setPageStep(1);
  m_slStoppingValue->setValue(500);
  m_slStoppingValue->setTracking(false);
  m_slStoppingValue->setToolTip("The \"stopping value\" parameter in the fast marching 3D algorithm");
  connect( m_slStoppingValue, SIGNAL(valueChanged(double)), this, SLOT(OnStoppingValueChanged(double)));
  vlayout->addWidget( m_slStoppingValue );

  // upper threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Upper Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
   }

  m_slUpperThreshold = new ctkSliderWidget(gbControls);
  m_slUpperThreshold->setMinimum(0);
  m_slUpperThreshold->setMaximum(10000);
  m_slUpperThreshold->setPageStep(1);
  m_slUpperThreshold->setValue(1000);
  m_slUpperThreshold->setTracking(false);
  m_slUpperThreshold->setToolTip("The \"upper threshold\" parameter in the final thresholding");
  connect( m_slUpperThreshold, SIGNAL(valueChanged(double)), this, SLOT(OnUpperThresholdChanged(double)));
  vlayout->addWidget( m_slUpperThreshold );

  // lower threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(gbControls);
   lbl->setText("Lower Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
   }

  m_slLowerThreshold = new ctkSliderWidget(gbControls);
  m_slLowerThreshold->setMinimum(-100);
  m_slLowerThreshold->setMaximum(100);
  m_slLowerThreshold->setPageStep(1);
  m_slLowerThreshold->setValue(0);
  m_slLowerThreshold->setTracking(false);
  m_slLowerThreshold->setToolTip("The \"lower threshold\" parameter in the final thresholding");
  connect( m_slLowerThreshold, SIGNAL(valueChanged(double)), this, SLOT(OnLowerThresholdChanged(double)));
  vlayout->addWidget( m_slLowerThreshold );

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
}

void QmitkFastMarchingTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool3D*>( tool );

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

void QmitkFastMarchingTool3DGUI::OnUpperThresholdChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetUpperThreshold( value );
  }
}

void QmitkFastMarchingTool3DGUI::OnLowerThresholdChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLowerThreshold( value );
  }
}

void QmitkFastMarchingTool3DGUI::OnBetaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetBeta( value );
  }
}

void QmitkFastMarchingTool3DGUI::OnAlphaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetAlpha( value );
  }
}

void QmitkFastMarchingTool3DGUI::OnStoppingValueChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetStoppingValue( value );
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
}
