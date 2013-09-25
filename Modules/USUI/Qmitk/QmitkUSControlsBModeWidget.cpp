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

#include "QmitkUSControlsBModeWidget.h"
#include "ui_QmitkUSControlsBModeWidget.h"

QmitkUSControlsBModeWidget::QmitkUSControlsBModeWidget(mitk::USControlInterfaceBMode::Pointer controlInterface, QWidget *parent)
  : QWidget(parent), ui(new Ui::QmitkUSControlsBModeWidget),
    m_ControlInterface(controlInterface)
{
  ui->setupUi(this);

  if ( ! m_ControlInterface )
  {
    ui->scanningDepthComboBox->setEnabled(false);
    ui->scanningGainSlider->setEnabled(false);
    ui->scanningRejectionSlider->setEnabled(false);
    return;
  }

  if ( ! m_ControlInterface->GetIsActive() ) { m_ControlInterface->SetIsActive(true); }

    // get possible scanning depth values and set combo box values according to them
  std::vector<double> scanningDepths = m_ControlInterface->GetScanningDepthValues();
  double curDepthValue = m_ControlInterface->GetScanningDepth();
  for (std::vector<double>::iterator it = scanningDepths.begin(); it != scanningDepths.end(); it++)
  {
    ui->scanningDepthComboBox->addItem(QString::number(*it, 'f', 2));

    // set current index to last inserted element if this element is equal
    // to the current depth value got from the interface
    if (curDepthValue == *it) ui->scanningDepthComboBox->setCurrentIndex(ui->scanningDepthComboBox->count()-1);
  }


  ui->scanningGainSlider->setMinimum(m_ControlInterface->GetScanningGainMin());
  ui->scanningGainSlider->setMaximum(m_ControlInterface->GetScanningGainMax());
  ui->scanningGainSlider->setTickInterval(m_ControlInterface->GetScanningGainTick());
  ui->scanningGainSlider->setValue(m_ControlInterface->GetScanningGain());

  ui->scanningRejectionSlider->setMinimum(m_ControlInterface->GetScanningRejectionMin());
  ui->scanningRejectionSlider->setMaximum(m_ControlInterface->GetScanningRejectionMax());
  ui->scanningRejectionSlider->setTickInterval(m_ControlInterface->GetScanningGainTick());
  ui->scanningRejectionSlider->setValue(m_ControlInterface->GetScanningGain());

  connect( ui->scanningDepthComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDepthControlActivated(int)) );
  connect( ui->scanningGainSlider, SIGNAL(valueChanged(int)), this, SLOT(OnGainControlValueChanged(int)) );
  connect( ui->scanningRejectionSlider, SIGNAL(valueChanged(int)), this, SLOT(OnRejectionControlValueChanged(int)) );
}

QmitkUSControlsBModeWidget::~QmitkUSControlsBModeWidget()
{
  delete ui;
}

// slots
void QmitkUSControlsBModeWidget::OnDepthControlActivated(int)
{
  m_ControlInterface->SetScanningDepth((ui->scanningDepthComboBox->currentText()).toDouble());
}

void QmitkUSControlsBModeWidget::OnGainControlValueChanged(int value)
{
  m_ControlInterface->SetScanningGain(static_cast<double>(value));
}

void QmitkUSControlsBModeWidget::OnRejectionControlValueChanged(int value)
{
  m_ControlInterface->SetScanningRejection(static_cast<double>(value));
}