/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUSControlsBModeWidget.h"
#include "ui_QmitkUSControlsBModeWidget.h"

QmitkUSControlsBModeWidget::QmitkUSControlsBModeWidget(mitk::USControlInterfaceBMode::Pointer controlInterface, QWidget *parent)
  : QWidget(parent), ui(new Ui::QmitkUSControlsBModeWidget),
    m_ControlInterface(controlInterface)
{
  ui->setupUi(this);

  if ( ! m_ControlInterface )
  {
    this->setDisabled(true);
    /*ui->scanningDepthComboBox->setEnabled(false);
    ui->scanningGainSlider->setEnabled(false);
    ui->scanningRejectionSlider->setEnabled(false);*/
    return;
  }

  if ( ! m_ControlInterface->GetIsActive() ) { m_ControlInterface->SetIsActive(true); }

  // get possible scanning depth values and set combo box values according to them
  std::vector<double> scanningDepths = m_ControlInterface->GetScanningDepthValues();
  double curDepthValue = m_ControlInterface->GetScanningDepth();
  for (auto it = scanningDepths.begin(); it != scanningDepths.end(); it++)
  {
    ui->scanningDepthComboBox->addItem(QString::number(*it, 'f', 2));

    // set current index to last inserted element if this element is equal
    // to the current depth value got from the interface
    if (curDepthValue == *it) ui->scanningDepthComboBox->setCurrentIndex(ui->scanningDepthComboBox->count()-1);
  }

  // get possible scanning frequency values and set combo box values according to them
  std::vector<double> scanningFrequencies = m_ControlInterface->GetScanningFrequencyValues();
  double curFrequencyValue = m_ControlInterface->GetScanningFrequency();
  for (auto it = scanningFrequencies.begin(); it != scanningFrequencies.end(); it++)
  {
    ui->scanningFrequencyComboBox->addItem(QString::number(*it, 'f', 2) + QString(" MHz"));

    // set current index to last inserted element if this element is equal
    // to the current depth value got from the interface
    if (curFrequencyValue == *it) ui->scanningFrequencyComboBox->setCurrentIndex(ui->scanningFrequencyComboBox->count()-1);
  }

  ui->scanningPowerSlider->setMinimum(m_ControlInterface->GetScanningPowerMin());
  ui->scanningPowerSlider->setMaximum(m_ControlInterface->GetScanningPowerMax());
  ui->scanningPowerSlider->setTickInterval(m_ControlInterface->GetScanningPowerTick());
  ui->scanningPowerSlider->setValue(m_ControlInterface->GetScanningPower());

  ui->scanningGainSlider->setMinimum(m_ControlInterface->GetScanningGainMin());
  ui->scanningGainSlider->setMaximum(m_ControlInterface->GetScanningGainMax());
  ui->scanningGainSlider->setTickInterval(m_ControlInterface->GetScanningGainTick());
  ui->scanningGainSlider->setValue(m_ControlInterface->GetScanningGain());

  ui->scanningRejectionSlider->setMinimum(m_ControlInterface->GetScanningRejectionMin());
  ui->scanningRejectionSlider->setMaximum(m_ControlInterface->GetScanningRejectionMax());
  ui->scanningRejectionSlider->setTickInterval(m_ControlInterface->GetScanningRejectionTick());
  ui->scanningRejectionSlider->setValue(m_ControlInterface->GetScanningRejection());

  ui->scanningDynamicRangeSlider->setMinimum(m_ControlInterface->GetScanningDynamicRangeMin());
  ui->scanningDynamicRangeSlider->setMaximum(m_ControlInterface->GetScanningDynamicRangeMax());
  ui->scanningDynamicRangeSlider->setTickInterval(m_ControlInterface->GetScanningDynamicRangeTick());
  ui->scanningDynamicRangeSlider->setValue(m_ControlInterface->GetScanningDynamicRange());

  connect( ui->scanningFrequencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrequencyControlIndexChanged(int)) );
  connect( ui->scanningDepthComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDepthControlActivated(int)) );
  connect( ui->scanningPowerSlider, SIGNAL(valueChanged(int)), this, SLOT(OnPowerControlValueChanged(int)) );
  connect( ui->scanningGainSlider, SIGNAL(valueChanged(int)), this, SLOT(OnGainControlValueChanged(int)) );
  connect( ui->scanningRejectionSlider, SIGNAL(valueChanged(int)), this, SLOT(OnRejectionControlValueChanged(int)) );
  connect( ui->scanningDynamicRangeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnDynamicRangeControlValueChanged(int)) );
}

QmitkUSControlsBModeWidget::~QmitkUSControlsBModeWidget()
{
  delete ui;
}

// slots
void QmitkUSControlsBModeWidget::OnFrequencyControlIndexChanged(int)
{
  QString currentText = ui->scanningFrequencyComboBox->currentText();
  m_ControlInterface->SetScanningFrequency((currentText.left(currentText.size()-5)).toDouble());
}

void QmitkUSControlsBModeWidget::OnDepthControlActivated(int)
{
  m_ControlInterface->SetScanningDepth(ui->scanningDepthComboBox->currentText().toDouble());
}

void QmitkUSControlsBModeWidget::OnPowerControlValueChanged(int value)
{
  m_ControlInterface->SetScanningPower(static_cast<double>(value));
  ui->scanningPowerLabel_value->setText(QString::number(value) +"%");

}

void QmitkUSControlsBModeWidget::OnGainControlValueChanged(int value)
{
  m_ControlInterface->SetScanningGain(static_cast<double>(value));
  ui->scanningGainLabel_value->setText(QString::number(value) + "%");
}

void QmitkUSControlsBModeWidget::OnRejectionControlValueChanged(int value)
{
  m_ControlInterface->SetScanningRejection(static_cast<double>(value));
  ui->scanningRejectionLabel_value->setText(QString::number(value));
}

void QmitkUSControlsBModeWidget::OnDynamicRangeControlValueChanged(int value)
{
  m_ControlInterface->SetScanningDynamicRange(static_cast<double>(value));
  ui->scanningDynamicRangeLabel_value->setText(QString::number(value) + "dB");
}
