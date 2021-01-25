/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFastMarchingToolGUIBase.h"

#include "mitkBaseRenderer.h"
#include "mitkStepper.h"
#include <QApplication>
#include <QGroupBox>
#include <QMessageBox>
#include <ctkRangeWidget.h>
#include <ctkSliderWidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

QmitkFastMarchingToolGUIBase::QmitkFastMarchingToolGUIBase(bool mode2D) : QmitkAutoSegmentationToolGUIBase(mode2D)
{
}

QmitkFastMarchingToolGUIBase::~QmitkFastMarchingToolGUIBase()
{
}

void QmitkFastMarchingToolGUIBase::Update()
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetLowerThreshold(m_slwThreshold->minimumValue());
    tool->SetUpperThreshold(m_slwThreshold->maximumValue());
    tool->SetStoppingValue(m_slStoppingValue->value());
    tool->SetSigma(m_slSigma->value());
    tool->SetAlpha(m_slAlpha->value());
    tool->SetBeta(m_slBeta->value());
    tool->UpdatePreview();
  }
}

void QmitkFastMarchingToolGUIBase::OnThresholdChanged(double lower, double upper)
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetLowerThreshold(lower);
    tool->SetUpperThreshold(upper);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::OnBetaChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetBeta(value);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::OnSigmaChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetSigma(value);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::OnAlphaChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetAlpha(value);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::OnStoppingValueChanged(double value)
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->SetStoppingValue(value);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::OnClearSeeds()
{
  auto tool = this->GetConnectedToolAs<mitk::FastMarchingBaseTool>();
  if (nullptr != tool)
  {
    tool->ClearSeeds();
    this->EnableWidgets(false);
    this->Update();
  }
}

void QmitkFastMarchingToolGUIBase::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  auto tool = dynamic_cast<mitk::FastMarchingBaseTool*>(newTool);
  if (nullptr != tool)
  {
    tool->SetLowerThreshold(m_slwThreshold->minimumValue());
    tool->SetUpperThreshold(m_slwThreshold->maximumValue());
    tool->SetStoppingValue(m_slStoppingValue->value());
    tool->SetSigma(m_slSigma->value());
    tool->SetAlpha(m_slAlpha->value());
    tool->SetBeta(m_slBeta->value());
    tool->ClearSeeds();
  }
}

void QmitkFastMarchingToolGUIBase::InitializeUI(QBoxLayout* mainLayout)
{
  mainLayout->setContentsMargins(0, 0, 0, 0);

  QFont fntHelp;
  fntHelp.setBold(true);

  QLabel* lblHelp = new QLabel(this);
  lblHelp->setText("Press shift-click to add seeds repeatedly.");
  lblHelp->setFont(fntHelp);

  mainLayout->addWidget(lblHelp);

  // Sigma controls
  {
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(2);

    QLabel* lbl = new QLabel(this);
    lbl->setText("Sigma: ");
    hlayout->addWidget(lbl);

    QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(sp2);

    mainLayout->addItem(hlayout);
  }

  m_slSigma = new ctkSliderWidget(this);
  m_slSigma->setMinimum(0.1);
  m_slSigma->setMaximum(5.0);
  m_slSigma->setPageStep(0.1);
  m_slSigma->setSingleStep(0.01);
  m_slSigma->setValue(1.0);
  m_slSigma->setTracking(false);
  m_slSigma->setToolTip("The \"sigma\" parameter in the Gradient Magnitude filter.");
  connect(m_slSigma, SIGNAL(valueChanged(double)), this, SLOT(OnSigmaChanged(double)));
  mainLayout->addWidget(m_slSigma);

  // Alpha controls
  {
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(2);

    QLabel* lbl = new QLabel(this);
    lbl->setText("Alpha: ");
    hlayout->addWidget(lbl);

    QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(sp2);

    mainLayout->addItem(hlayout);
  }

  m_slAlpha = new ctkSliderWidget(this);
  m_slAlpha->setMinimum(-10);
  m_slAlpha->setMaximum(0);
  m_slAlpha->setPageStep(0.1);
  m_slAlpha->setSingleStep(0.01);
  m_slAlpha->setValue(-2.5);
  m_slAlpha->setTracking(false);
  m_slAlpha->setToolTip("The \"alpha\" parameter in the Sigmoid mapping filter.");
  connect(m_slAlpha, SIGNAL(valueChanged(double)), this, SLOT(OnAlphaChanged(double)));
  mainLayout->addWidget(m_slAlpha);

  // Beta controls
  {
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(2);

    QLabel* lbl = new QLabel(this);
    lbl->setText("Beta: ");
    hlayout->addWidget(lbl);

    QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(sp2);

    mainLayout->addLayout(hlayout);
  }

  m_slBeta = new ctkSliderWidget(this);
  m_slBeta->setMinimum(0);
  m_slBeta->setMaximum(100);
  m_slBeta->setPageStep(0.1);
  m_slBeta->setSingleStep(0.01);
  m_slBeta->setValue(3.5);
  m_slBeta->setTracking(false);
  m_slBeta->setToolTip("The \"beta\" parameter in the Sigmoid mapping filter.");
  connect(m_slBeta, SIGNAL(valueChanged(double)), this, SLOT(OnBetaChanged(double)));
  mainLayout->addWidget(m_slBeta);

  // stopping value controls
  {
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(2);

    QLabel* lbl = new QLabel(this);
    lbl->setText("Stopping value: ");
    hlayout->addWidget(lbl);

    QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(sp2);

    mainLayout->addLayout(hlayout);
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
  connect(m_slStoppingValue, SIGNAL(valueChanged(double)), this, SLOT(OnStoppingValueChanged(double)));
  mainLayout->addWidget(m_slStoppingValue);

  // threshold controls
  {
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(2);

    QLabel* lbl = new QLabel(this);
    lbl->setText("Threshold: ");
    hlayout->addWidget(lbl);

    QSpacerItem* sp2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(sp2);

    mainLayout->addLayout(hlayout);
  }

  m_slwThreshold = new ctkRangeWidget(this);
  m_slwThreshold->setMinimum(-100);
  m_slwThreshold->setMaximum(5000);
  m_slwThreshold->setMinimumValue(-100);
  m_slwThreshold->setMaximumValue(2000);
  m_slwThreshold->setDecimals(0);
  m_slwThreshold->setTracking(false);
  m_slwThreshold->setToolTip("The lower and upper thresholds for the final thresholding");
  connect(m_slwThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(OnThresholdChanged(double, double)));
  mainLayout->addWidget(m_slwThreshold);

  m_btClearSeeds = new QPushButton("Clear");
  m_btClearSeeds->setToolTip("Clear current result and start over again");
  m_btClearSeeds->setEnabled(false);
  mainLayout->addWidget(m_btClearSeeds);
  connect(m_btClearSeeds, SIGNAL(clicked()), this, SLOT(OnClearSeeds()));

  m_slSigma->setDecimals(2);
  m_slBeta->setDecimals(2);
  m_slAlpha->setDecimals(2);

  this->EnableWidgets(false);

  Superclass::InitializeUI(mainLayout);
}

void QmitkFastMarchingToolGUIBase::EnableWidgets(bool enable)
{
  Superclass::EnableWidgets(enable);
  m_slSigma->setEnabled(enable);
  m_slAlpha->setEnabled(enable);
  m_slBeta->setEnabled(enable);
  m_slStoppingValue->setEnabled(enable);
  m_slwThreshold->setEnabled(enable);
  m_btClearSeeds->setEnabled(enable);
}
