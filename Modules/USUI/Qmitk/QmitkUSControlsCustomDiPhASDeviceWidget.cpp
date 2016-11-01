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

#include "QmitkUSControlsCustomDiPhASDeviceWidget.h"
#include "ui_QmitkUSControlsCustomDiPhASDeviceWidget.h"
#include <QMessageBox>


#include <mitkException.h>

QmitkUSControlsCustomDiPhASDeviceWidget::QmitkUSControlsCustomDiPhASDeviceWidget(QWidget *parent)
  : QmitkUSAbstractCustomWidget(parent), ui(new Ui::QmitkUSControlsCustomDiPhASDeviceWidget)
{
}

QmitkUSControlsCustomDiPhASDeviceWidget::~QmitkUSControlsCustomDiPhASDeviceWidget()
{
  delete ui;
}

std::string QmitkUSControlsCustomDiPhASDeviceWidget::GetDeviceClass() const
{
  return "org.mitk.modules.us.USDiPhASDevice";
}

QmitkUSAbstractCustomWidget* QmitkUSControlsCustomDiPhASDeviceWidget::Clone(QWidget* parent) const
{
  QmitkUSAbstractCustomWidget* clonedWidget = new QmitkUSControlsCustomDiPhASDeviceWidget(parent);
  clonedWidget->SetDevice(this->GetDevice());
  return clonedWidget;
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnDeviceSet()
{
  m_ControlInterface = dynamic_cast<mitk::USDiPhASDeviceCustomControls*>
    (this->GetDevice()->GetControlInterfaceCustom().GetPointer());

  if ( m_ControlInterface.IsNotNull() )
  {
    m_ControlInterface->passGUIOut([this](QString str)->void{
      if (this && this->ui) {
        this->ui->CurrentState->setText(str);
      } });
  }
  else
  {
    MITK_WARN("QmitkUSAbstractCustomWidget")("QmitkUSControlsCustomDiPhASDeviceWidget")
        << "Did not get a custom device control interface.";
  }

  //now pass the default values

  m_ControlInterface->SetSilentUpdate(true); // don't update the scanmode everytime

  OnTransmitPhaseLengthChanged();
  OnExcitationFrequencyChanged();
  OnTransmitEventsChanged();
  OnVoltageChanged();
  OnScanDepthChanged();
  OnAveragingCountChanged();
  OnTGCMinChanged();
  OnTGCMaxChanged();
  OnDataTypeChanged();
  OnPitchChanged();
  OnReconstructedSamplesChanged();
  OnReconstructedLinesChanged();
  OnSpeedOfSoundChanged();
  OnBandpassEnabledChanged();
  OnLowCutChanged();
  OnHighCutChanged();
  OnUseBModeFilterChanged();

  m_ControlInterface->SetSilentUpdate(false); // on the last update pass the scanmode and geometry!

  OnModeChanged();
}

void QmitkUSControlsCustomDiPhASDeviceWidget::Initialize()
{
  ui->setupUi(this);

  connect(ui->UseBModeFilter, SIGNAL(stateChanged(int)), this, SLOT(OnUseBModeFilterChanged()));
  connect(ui->StartStopRecord, SIGNAL(clicked()), this, SLOT(OnRecordChanged()));

  //transmit
  connect(ui->TransmitPhaseLength, SIGNAL(valueChanged(double)), this, SLOT(OnTransmitPhaseLengthChanged()));
  connect(ui->ExcitationFrequency, SIGNAL(valueChanged(double)), this, SLOT(OnExcitationFrequencyChanged()));
  connect(ui->TransmitEvents, SIGNAL(valueChanged(int)), this, SLOT(OnTransmitEventsChanged()));
  connect(ui->Voltage, SIGNAL(valueChanged(int)), this, SLOT(OnVoltageChanged()));
  connect(ui->Mode, SIGNAL(currentTextChanged(QString)), this, SLOT(OnModeChanged()));

  //Receive
  connect(ui->ScanDepth, SIGNAL(valueChanged(double)), this, SLOT(OnScanDepthChanged()));
  connect(ui->AveragingCount, SIGNAL(valueChanged(int)), this, SLOT(OnAveragingCountChanged()));
  connect(ui->TimeGainCompensationMin, SIGNAL(valueChanged(int)), this, SLOT(OnTGCMinChanged()));
  connect(ui->TimeGainCompensationMax, SIGNAL(valueChanged(int)), this, SLOT(OnTGCMaxChanged()));
  connect(ui->DataType, SIGNAL(currentTextChanged(QString)), this, SLOT(OnDataTypeChanged()));

  //Beamforming
  connect(ui->PitchOfTransducer, SIGNAL(valueChanged(double)), this, SLOT(OnPitchChanged()));
  connect(ui->ReconstructedSamplesPerLine, SIGNAL(valueChanged(int)), this, SLOT(OnReconstructedSamplesChanged()));
  connect(ui->ReconstructedLines, SIGNAL(valueChanged(int)), this, SLOT(OnReconstructedLinesChanged()));
  connect(ui->SpeedOfSound, SIGNAL(valueChanged(int)), this, SLOT(OnSpeedOfSoundChanged()));

  //Bandpass
  connect(ui->BandpassEnabled, SIGNAL(currentTextChanged(QString)), this, SLOT(OnBandpassEnabledChanged()));
  connect(ui->LowCut, SIGNAL(valueChanged(double)), this, SLOT(OnLowCutChanged()));
  connect(ui->HighCut, SIGNAL(valueChanged(double)), this, SLOT(OnHighCutChanged()));
}

//slots

void QmitkUSControlsCustomDiPhASDeviceWidget::OnUseBModeFilterChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  bool UseBModeFilter = ui->UseBModeFilter->isChecked();
  m_ControlInterface->SetUseBModeFilter(UseBModeFilter);
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnRecordChanged()
{
  if (ui->StartStopRecord->text() == "Start Recording")
  {
    ui->StartStopRecord->setText("Stop Recording");
    m_ControlInterface->SetRecord(true);
  }
  else
  {
    ui->StartStopRecord->setText("Start Recording");
    m_ControlInterface->SetRecord(false);
  }
}

//Transmit
void QmitkUSControlsCustomDiPhASDeviceWidget::OnTransmitPhaseLengthChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetTransmitPhaseLength(ui->TransmitPhaseLength->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnExcitationFrequencyChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetExcitationFrequency(ui->ExcitationFrequency->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnTransmitEventsChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  m_ControlInterface->SetTransmitEvents(ui->TransmitEvents->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnVoltageChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetVoltage(ui->Voltage->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnModeChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  QString Mode = ui->Mode->currentText();
  bool silent = m_ControlInterface->GetSilentUpdate();
  m_ControlInterface->SetSilentUpdate(true);

  if (Mode == "Ultrasound only") {
    m_ControlInterface->SetMode(false);
    ui->TransmitEvents->setValue(1);
  }
  else if (Mode == "Interleaved") {
    m_ControlInterface->SetMode(true);
    ui->TransmitEvents->setValue(1);
  }
  if (!silent) { m_ControlInterface->SetSilentUpdate(false); }
  OnTransmitEventsChanged();
}

//Receive
void QmitkUSControlsCustomDiPhASDeviceWidget::OnScanDepthChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetScanDepth(ui->ScanDepth->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnAveragingCountChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetAveragingCount(ui->AveragingCount->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnTGCMinChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  int tgcMin = ui->TimeGainCompensationMin->value();
  int tgcMax = ui->TimeGainCompensationMax->value();
  if (tgcMin > tgcMax) {
    ui->TimeGainCompensationMin->setValue(tgcMax);
    MITK_INFO << "User tried to set tgcMin>tgcMax.";
  }
  
  m_ControlInterface->SetTGCMin(ui->TimeGainCompensationMin->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnTGCMaxChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  int tgcMin = ui->TimeGainCompensationMin->value();
  int tgcMax = ui->TimeGainCompensationMax->value();
  if (tgcMin > tgcMax) {
    ui->TimeGainCompensationMax->setValue(tgcMin);
    MITK_INFO << "User tried to set tgcMin>tgcMax.";
  }
  
  m_ControlInterface->SetTGCMax(ui->TimeGainCompensationMax->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnDataTypeChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  QString DataType = ui->DataType->currentText();
  if (DataType == "Image Data") {
    m_ControlInterface->SetDataType(0);
  }
  else if (DataType == "Beamformed Data") {
    m_ControlInterface->SetDataType(1);
  }
} // 0= image; 1= beamformed;

//Beamforming
void QmitkUSControlsCustomDiPhASDeviceWidget::OnPitchChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetPitch(ui->PitchOfTransducer->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnReconstructedSamplesChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetReconstructedSamples(ui->ReconstructedSamplesPerLine->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnReconstructedLinesChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetReconstructedLines(ui->ReconstructedLines->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnSpeedOfSoundChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetSpeedOfSound(ui->SpeedOfSound->value());
}

//Bandpass
void QmitkUSControlsCustomDiPhASDeviceWidget::OnBandpassEnabledChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  if (ui->BandpassEnabled->currentText() == "On") {
    m_ControlInterface->SetBandpassEnabled(true);
  }
  else {
    m_ControlInterface->SetBandpassEnabled(false);
  }
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnLowCutChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  unsigned int Low = ui->LowCut->value();
  unsigned int High = ui->HighCut->value();
  if (Low > High) {
    ui->LowCut->setValue(High);
    MITK_INFO << "User tried to set LowCut>HighCut.";
  }

  m_ControlInterface->SetLowCut(ui->LowCut->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnHighCutChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  unsigned int Low = ui->LowCut->value();
  unsigned int High = ui->HighCut->value();
  if (Low > High) {
    ui->HighCut->setValue(Low);
    MITK_INFO << "User tried to set LowCut>HighCut.";
  }

  m_ControlInterface->SetHighCut(ui->HighCut->value());
}
