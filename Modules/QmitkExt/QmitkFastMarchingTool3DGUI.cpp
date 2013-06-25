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
#include <QCheckBox>
#include <qpainter.h>
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"


MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkFastMarchingTool3DGUI, "")

QmitkFastMarchingTool3DGUI::QmitkFastMarchingTool3DGUI()
:QmitkToolGUI(),
m_TimeIsConnected(false)
{
  this->setContentsMargins( 0, 0, 0, 0 );

  // create the visible widgets
  QVBoxLayout *vlayout = new QVBoxLayout(this);

  // Alpha controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Sigmoid Alpha: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
  }

  m_AlphaSlider = new ctkSliderWidget(this);
  m_AlphaSlider->setMinimum(-100);
  m_AlphaSlider->setMaximum(0);
  m_AlphaSlider->setPageStep(1);
  m_AlphaSlider->setValue(-10);
  m_AlphaSlider->setTracking(false);
//  m_AlphaSlider->setMaximumWidth(200);
  m_AlphaSlider->setToolTip("The \"alpha\" parameter in the fast marching 3D algorithm");
  connect( m_AlphaSlider, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));
  vlayout->addWidget( m_AlphaSlider );

  // Beta controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Sigmoid Beta: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
  }

  m_BetaSlider = new ctkSliderWidget(this);
  m_BetaSlider->setMinimum(0);
  m_BetaSlider->setMaximum(1000);
  m_BetaSlider->setPageStep(1);
  m_BetaSlider->setValue(50);
  m_BetaSlider->setTracking(false);
//  m_BetaSlider->setMaximumWidth(200);
  m_BetaSlider->setToolTip("The \"beta\" parameter in the fast marching 3D algorithm");
  connect( m_BetaSlider, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));
  vlayout->addWidget( m_BetaSlider );

  // stopping value controls
  QLabel* labelStoppingValue = new QLabel("Stopping value: ");
  vlayout->addWidget( labelStoppingValue );

  m_StoppingValueSlider = new ctkSliderWidget(this);
  m_StoppingValueSlider->setMinimum(0);
  m_StoppingValueSlider->setMaximum(10000);
  m_StoppingValueSlider->setPageStep(1);
  m_StoppingValueSlider->setValue(500);
  m_StoppingValueSlider->setTracking(false);
//  m_StoppingValueSlider->setMaximumWidth(200);
  m_StoppingValueSlider->setToolTip("The \"stopping value\" parameter in the fast marching 3D algorithm");
  connect( m_StoppingValueSlider, SIGNAL(valueChanged(double)), this, SLOT(OnStoppingValueChanged(double)));
  vlayout->addWidget( m_StoppingValueSlider );

  // upper threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Upper Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
   }

  m_UpperThresholdSlider = new ctkSliderWidget(this);
  m_UpperThresholdSlider->setMinimum(0);
  m_UpperThresholdSlider->setMaximum(10000);
  m_UpperThresholdSlider->setPageStep(1);
  m_UpperThresholdSlider->setValue(1000);
  m_UpperThresholdSlider->setTracking(false);
//  m_UpperThresholdSlider->setMaximumWidth(200);
  m_UpperThresholdSlider->setToolTip("The \"upper threshold\" parameter in the fast marching 3D algorithm");
  connect( m_UpperThresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnUpperThresholdChanged(double)));
  vlayout->addWidget( m_UpperThresholdSlider );

  // lower threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Lower Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   vlayout->addLayout(hlayout);
   }

  m_LowerThresholdSlider = new ctkSliderWidget(this);
  m_LowerThresholdSlider->setMinimum(-100);
  m_LowerThresholdSlider->setMaximum(100);
  m_LowerThresholdSlider->setPageStep(1);
  m_LowerThresholdSlider->setValue(0);
  m_LowerThresholdSlider->setTracking(false);
//  m_LowerThresholdSlider->setMaximumWidth(200);
  m_LowerThresholdSlider->setToolTip("The \"lower threshold\" parameter in the fast marching 3D algorithm");
  connect( m_LowerThresholdSlider, SIGNAL(valueChanged(double)), this, SLOT(OnLowerThresholdChanged(double)));
  vlayout->addWidget( m_LowerThresholdSlider );

/*
  m_cbxLivePreview = new QCheckBox("Live preview");
  m_cbxLivePreview->setChecked(true);
  m_cbxLivePreview->setTristate(false);
  layout->addWidget(m_cbxLivePreview, 5,0);
  connect( m_cbxLivePreview, SIGNAL(stateChanged(int)), this, SLOT(OnLivePreviewCheckBoxChanged(int)) );
*/
  m_btClearSeeds = new QPushButton("Clear");
  m_btClearSeeds->setToolTip("Clear current result and start over again");
  //layout->addWidget(m_btClearSeeds, 5,1);
  vlayout->addWidget(m_btClearSeeds);
  connect( m_btClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  m_btConfirm = new QPushButton("Accept");
  m_btConfirm->setToolTip("Incorporate current result in your working session.");
  vlayout->addWidget(m_btConfirm);
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
    m_TimeStepper = new QmitkStepperAdapter(this,
      stepper,
      "exampleStepper");

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

void QmitkFastMarchingTool3DGUI::OnLivePreviewCheckBoxChanged(int value)
{
  bool b(value);
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLivePreviewEnabled(b);
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
