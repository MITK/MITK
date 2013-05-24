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
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include "mitkStepper.h"
#include "mitkBaseRenderer.h"


MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkFastMarchingTool3DGUI, "")

QmitkFastMarchingTool3DGUI::QmitkFastMarchingTool3DGUI()
:QmitkToolGUI(),
m_TimeIsConnected(false),
m_UpperThresholdSlider(NULL)
{
  this->setContentsMargins( 0, 0, 0, 0 );

  // create the visible widgets
  QGridLayout *layout = new QGridLayout(this);


  /*StandardDeviation*/
  QLabel* labelStandardDeviation = new QLabel("Alpha: ");
  layout->addWidget(labelStandardDeviation,0,0);

  m_StandardDeviationLabel = new QLabel("-1.0");
  layout->addWidget(m_StandardDeviationLabel,0,2);

  m_StandardDeviationSlider = new QSlider( Qt::Horizontal);
  m_StandardDeviationSlider->setMinimum(-100);
  m_StandardDeviationSlider->setMaximum(0);
  m_StandardDeviationSlider->setPageStep(1);
  m_StandardDeviationSlider->setValue(-10);
  m_StandardDeviationSlider->setMaximumWidth(200);
  connect( m_StandardDeviationSlider, SIGNAL(valueChanged(int)), this, SLOT(OnStandardDeviationChanged(int)));
  layout->addWidget( m_StandardDeviationSlider,0,1);
  /*END StandardDeviation*/


  /*Mu*/
  QLabel* labelMu = new QLabel("Beta: ");
  layout->addWidget(labelMu,1,0);

  m_MuLabel = new QLabel("5.0");
  layout->addWidget(m_MuLabel,1,2);

  m_MuSlider = new QSlider( Qt::Horizontal);
  m_MuSlider->setMinimum(0);
  m_MuSlider->setMaximum(1000);
  m_MuSlider->setPageStep(1);
  m_MuSlider->setValue(50);
  m_MuSlider->setMaximumWidth(200);
  connect( m_MuSlider, SIGNAL(valueChanged(int)), this, SLOT(OnMuChanged(int)));
  layout->addWidget( m_MuSlider,1,1);
  /*END Mu*/


  /*StoppingValue*/
  QLabel* labelStoppingValue = new QLabel("Stopping value: ");
  layout->addWidget(labelStoppingValue,2,0);

  m_StoppingValueLabel = new QLabel("50.0");
  layout->addWidget(m_StoppingValueLabel,2,2);

  m_StoppingValueSlider = new QSlider( Qt::Horizontal);
  m_StoppingValueSlider->setMinimum(0);
  m_StoppingValueSlider->setMaximum(10000);
  m_StoppingValueSlider->setPageStep(1);
  m_StoppingValueSlider->setValue(500);
  m_StoppingValueSlider->setMaximumWidth(200);
  connect( m_StoppingValueSlider, SIGNAL(valueChanged(int)), this, SLOT(OnStoppingValueChanged(int)));
  layout->addWidget( m_StoppingValueSlider,2,1);
  /*END StoppingValue*/


  /*Upperthreshold*/
  QLabel* labelUpper = new QLabel("Upper Threshold: ");
  layout->addWidget(labelUpper,3,0);

  m_UpperThresholdLabel = new QLabel("100.0");
  layout->addWidget(m_UpperThresholdLabel,3,2);

  m_UpperThresholdSlider = new QSlider( Qt::Horizontal);
  m_UpperThresholdSlider->setMinimum(0);
  m_UpperThresholdSlider->setMaximum(10000);
  m_UpperThresholdSlider->setPageStep(1);
  m_UpperThresholdSlider->setValue(1000);
  m_UpperThresholdSlider->setMaximumWidth(200);
  connect( m_UpperThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(OnUpperThresholdChanged(int)));
  layout->addWidget( m_UpperThresholdSlider,3,1);
  /*END Upperthreshold*/


  /*LowerThreshold*/
  QLabel* labelLower = new QLabel("Lower Threshold: ");
  layout->addWidget(labelLower,4,0);

  m_LowerThresholdLabel = new QLabel("0.0");
  layout->addWidget(m_LowerThresholdLabel,4,2);

  m_LowerThresholdSlider = new QSlider( Qt::Horizontal);
  m_LowerThresholdSlider->setMinimum(-100);
  m_LowerThresholdSlider->setMaximum(100);
  m_LowerThresholdSlider->setPageStep(1);
  m_LowerThresholdSlider->setValue(0);
  m_LowerThresholdSlider->setMaximumWidth(200);
  connect( m_LowerThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(OnLowerThresholdChanged(int)));
  layout->addWidget( m_LowerThresholdSlider,4,1);
  /*END LowerThreshold*/

  m_ConfirmButton = new QPushButton("Confirm Segmentation");
  layout->addWidget(m_ConfirmButton, 6,0);
  connect( m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()) );

  m_LivePreviewCheckBox = new QCheckBox("Live preview");
  m_LivePreviewCheckBox->setChecked(false);
  m_LivePreviewCheckBox->setTristate(false);
  layout->addWidget(m_LivePreviewCheckBox, 5,0);
  connect( m_LivePreviewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnLivePreviewCheckBoxChanged(int)) );

  m_ClearSeedsButton = new QPushButton("Clear seeds");
  layout->addWidget(m_ClearSeedsButton, 5,1);
  connect( m_ClearSeedsButton, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

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

void QmitkFastMarchingTool3DGUI::OnUpperThresholdChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetUpperThreshold( value );

    //visualize slider value
    m_UpperThresholdLabel->setText(QString("%1 ").arg(float(value)/10.0));
  }
}

void QmitkFastMarchingTool3DGUI::OnLowerThresholdChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLowerThreshold( value );

    //visualize slider value
    m_LowerThresholdLabel->setText(QString("%1 ").arg(float(value)/10.0));
  }
}

void QmitkFastMarchingTool3DGUI::OnMuChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetMu( value );

    //visualize slider value
    m_MuLabel->setText(QString("%1 ").arg(float(value)/10.0));
  }
}

void QmitkFastMarchingTool3DGUI::OnStandardDeviationChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetStandardDeviation( value );

    //visualize slider value
    m_StandardDeviationLabel->setText(QString("%1 ").arg(float(value)/10.0));
  }
}

void QmitkFastMarchingTool3DGUI::OnStoppingValueChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetStoppingValue( value );

    //visualize slider value
    m_StoppingValueLabel->setText(QString("%1 ").arg(float(value)/10.0));
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
