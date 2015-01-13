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

#include "QmitkConfirmSegmentationDialog.h"

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


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkFastMarchingTool3DGUI, "")

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
  lblHelp->setText("Press shift-click to add seeds repeatedly.");
  lblHelp->setFont(fntHelp);

  widgetLayout->addWidget(lblHelp);

  // Sigma controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Sigma: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   widgetLayout->addItem(hlayout);
  }

  m_slSigma = new ctkSliderWidget(this);
  m_slSigma->setMinimum(0.1);
  m_slSigma->setMaximum(5.0);
  m_slSigma->setPageStep(0.1);
  m_slSigma->setSingleStep(0.01);
  m_slSigma->setValue(1.0);
  m_slSigma->setTracking(false);
  m_slSigma->setToolTip("The \"sigma\" parameter in the Gradient Magnitude filter.");
  connect( m_slSigma, SIGNAL(valueChanged(double)), this, SLOT(OnSigmaChanged(double)));
  widgetLayout->addWidget( m_slSigma );

  // Alpha controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Alpha: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   widgetLayout->addItem(hlayout);
  }

  m_slAlpha = new ctkSliderWidget(this);
  m_slAlpha->setMinimum(-10);
  m_slAlpha->setMaximum(0);
  m_slAlpha->setPageStep(0.1);
  m_slAlpha->setSingleStep(0.01);
  m_slAlpha->setValue(-2.5);
  m_slAlpha->setTracking(false);
  m_slAlpha->setToolTip("The \"alpha\" parameter in the Sigmoid mapping filter.");
  connect( m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));
  widgetLayout->addWidget( m_slAlpha );

  // Beta controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Beta: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   widgetLayout->addLayout(hlayout);
  }

  m_slBeta = new ctkSliderWidget(this);
  m_slBeta->setMinimum(0);
  m_slBeta->setMaximum(100);
  m_slBeta->setPageStep(0.1);
  m_slBeta->setSingleStep(0.01);
  m_slBeta->setValue(3.5);
  m_slBeta->setTracking(false);
  m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping filter.");
  connect( m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));
  widgetLayout->addWidget( m_slBeta );

  // stopping value controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Stopping value: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   widgetLayout->addLayout(hlayout);
  }

  m_slStoppingValue = new ctkSliderWidget(this);
  m_slStoppingValue->setMinimum(0);
  m_slStoppingValue->setMaximum(10000);
  m_slStoppingValue->setPageStep(10);
  m_slStoppingValue->setSingleStep(1);
  m_slStoppingValue->setValue(2000);
  m_slStoppingValue->setDecimals(0);
  m_slStoppingValue->setTracking(false);
  m_slStoppingValue->setToolTip("The \"stopping value\" parameter in the fast marching 3D algorithm");
  connect( m_slStoppingValue, SIGNAL(valueChanged(double)), this, SLOT(OnStoppingValueChanged(double)));
  widgetLayout->addWidget( m_slStoppingValue );

  // threshold controls
  {
   QHBoxLayout *hlayout = new QHBoxLayout();
   hlayout->setSpacing(2);

   QLabel *lbl = new QLabel(this);
   lbl->setText("Threshold: ");
   hlayout->addWidget(lbl);

   QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   hlayout->addItem(sp2);

   widgetLayout->addLayout(hlayout);
  }

  m_slwThreshold = new ctkRangeWidget(this);
  m_slwThreshold->setMinimum(-100);
  m_slwThreshold->setMaximum(5000);
  m_slwThreshold->setMinimumValue(-100);
  m_slwThreshold->setMaximumValue(2000);
  m_slwThreshold->setDecimals(0);
  m_slwThreshold->setTracking(false);
  m_slwThreshold->setToolTip("The lower and upper thresholds for the final thresholding");
  connect( m_slwThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(OnThresholdChanged(double, double)));
  widgetLayout->addWidget( m_slwThreshold );

  m_btClearSeeds = new QPushButton("Clear");
  m_btClearSeeds->setToolTip("Clear current result and start over again");
  widgetLayout->addWidget(m_btClearSeeds);
  connect( m_btClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()) );

  m_btConfirm = new QPushButton("Confirm Segmentation");
  m_btConfirm->setToolTip("Incorporate current result in your working session.");
  m_btConfirm->setEnabled(false);
  widgetLayout->addWidget(m_btConfirm);
  connect( m_btConfirm, SIGNAL(clicked()), this, SLOT(OnConfirmSegmentation()) );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );

  this->setEnabled(false);

  m_slSigma->setDecimals(2);
  m_slBeta->setDecimals(2);
  m_slAlpha->setDecimals(2);
}

QmitkFastMarchingTool3DGUI::~QmitkFastMarchingTool3DGUI()
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );
    m_FastMarchingTool->RemoveReadyListener(mitk::MessageDelegate<QmitkFastMarchingTool3DGUI>(this, &QmitkFastMarchingTool3DGUI::OnFastMarchingToolReady) );
  }
}

void QmitkFastMarchingTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );
    m_FastMarchingTool->RemoveReadyListener(mitk::MessageDelegate<QmitkFastMarchingTool3DGUI>(this, &QmitkFastMarchingTool3DGUI::OnFastMarchingToolReady) );
  }

  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool3D*>( tool );

  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->CurrentlyBusy += mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, bool>( this, &QmitkFastMarchingTool3DGUI::BusyStateChanged );
    m_FastMarchingTool->AddReadyListener(mitk::MessageDelegate<QmitkFastMarchingTool3DGUI>(this, &QmitkFastMarchingTool3DGUI::OnFastMarchingToolReady) );

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

void QmitkFastMarchingTool3DGUI::Update()
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLowerThreshold( this->m_slwThreshold->minimumValue());
    m_FastMarchingTool->SetUpperThreshold( this->m_slwThreshold->maximumValue());
    m_FastMarchingTool->SetStoppingValue( this->m_slStoppingValue->value());
    m_FastMarchingTool->SetSigma( this->m_slSigma->value());
    m_FastMarchingTool->SetAlpha( this->m_slAlpha->value());
    m_FastMarchingTool->SetBeta( this->m_slBeta->value());
    m_FastMarchingTool->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnThresholdChanged(double lower, double upper)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetLowerThreshold( lower );
    m_FastMarchingTool->SetUpperThreshold( upper );
    this->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnBetaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetBeta( value );
    this->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnSigmaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetSigma( value );
    this->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnAlphaChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetAlpha( value );
    this->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnStoppingValueChanged(double value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetStoppingValue( value );
    this->Update();
  }
}

void QmitkFastMarchingTool3DGUI::OnConfirmSegmentation()
{
  QmitkConfirmSegmentationDialog dialog;
  QString segName = QString::fromStdString(m_FastMarchingTool->GetCurrentSegmentationName());

  dialog.SetSegmentationName(segName);
  int result = dialog.exec();

  switch(result)
  {
  case QmitkConfirmSegmentationDialog::CREATE_NEW_SEGMENTATION:
    m_FastMarchingTool->SetOverwriteExistingSegmentation(false);
    break;
  case QmitkConfirmSegmentationDialog::OVERWRITE_SEGMENTATION:
    m_FastMarchingTool->SetOverwriteExistingSegmentation(true);
    break;
  case QmitkConfirmSegmentationDialog::CANCEL_SEGMENTATION:
    return;
  }
  if (m_FastMarchingTool.IsNotNull())
  {
    m_btConfirm->setEnabled(false);
    m_FastMarchingTool->ConfirmSegmentation();
  }
}

void QmitkFastMarchingTool3DGUI::SetStepper(mitk::Stepper *stepper)
{
    this->m_TimeStepper = stepper;
}

void QmitkFastMarchingTool3DGUI::Refetch()
{
  //event from image navigator recieved - timestep has changed
    m_FastMarchingTool->SetCurrentTimeStep(m_TimeStepper->GetPos());
}

void QmitkFastMarchingTool3DGUI::OnClearSeeds()
{
  //event from image navigator recieved - timestep has changed
   m_FastMarchingTool->ClearSeeds();
   m_btConfirm->setEnabled(false);
   this->Update();
}

void QmitkFastMarchingTool3DGUI::BusyStateChanged(bool value)
{
  if (value)
      QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
  else
      QApplication::restoreOverrideCursor();
}

void QmitkFastMarchingTool3DGUI::OnFastMarchingToolReady()
{
  this->setEnabled(true);
  this->m_btConfirm->setEnabled(true);
}
