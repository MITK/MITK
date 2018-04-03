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

QmitkUSControlsCustomDiPhASDeviceWidget::QmitkUSControlsCustomDiPhASDeviceWidget()
  : ui(new Ui::QmitkUSControlsCustomDiPhASDeviceWidget)
{
}

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
      if (this->ui) {
        this->ui->CurrentState->setText(str);
      } });
  }
  else
  {
    MITK_WARN("QmitkUSAbstractCustomWidget")("QmitkUSControlsCustomDiPhASDeviceWidget")
        << "Did not get a custom device control interface.";
  }

  //now pass the default values

  m_OldReconstructionLines = 0;

  m_ControlInterface->SetSilentUpdate(true); // don't update the scanmode everytime

  OnTransmitPhaseLengthChanged();
  OnExcitationFrequencyChanged();
  OnTransmitEventsChanged();
  OnVoltageChanged();
  OnScanDepthChanged(); // HERE
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
  OnUseBModeFilterChanged(); // HERE
  OnVerticalSpacingChanged();
  OnScatteringCoefficientChanged();
  OnCompensateScatteringChanged();
  OnChangedSavingSettings();
  OnCompensateEnergyChanged();

  m_ControlInterface->SetSilentUpdate(false); // on the last update pass the scanmode and geometry!

  OnModeChanged(); // HERE
}

void QmitkUSControlsCustomDiPhASDeviceWidget::Initialize()
{
  ui->setupUi(this);

  connect(ui->CompensateEnergy, SIGNAL(stateChanged(int)), this, SLOT(OnCompensateEnergyChanged()));
  connect(ui->UseBModeFilter, SIGNAL(stateChanged(int)), this, SLOT(OnUseBModeFilterChanged()));
  connect(ui->StartStopRecord, SIGNAL(clicked()), this, SLOT(OnRecordChanged()));
  connect(ui->ScatteringCoefficient, SIGNAL(valueChanged(int)), this, SLOT(OnScatteringCoefficientChanged()));
  connect(ui->CompensateScattering, SIGNAL(stateChanged(int)), this, SLOT(OnCompensateScatteringChanged()));
  connect(ui->VerticalSpacing, SIGNAL(valueChanged(double)), this, SLOT(OnVerticalSpacingChanged()));
  connect(ui->SaveBeamformed, SIGNAL(stateChanged(int)), this, SLOT(OnChangedSavingSettings()));
  connect(ui->SaveRaw, SIGNAL(stateChanged(int)), this, SLOT(OnChangedSavingSettings()));
  //transmit
  connect(ui->TransmitPhaseLength, SIGNAL(valueChanged(double)), this, SLOT(OnTransmitPhaseLengthChanged()));
  connect(ui->ExcitationFrequency, SIGNAL(valueChanged(double)), this, SLOT(OnExcitationFrequencyChanged()));
  connect(ui->TransmitEvents, SIGNAL(valueChanged(int)), this, SLOT(OnTransmitEventsChanged()));
  connect(ui->Voltage, SIGNAL(valueChanged(int)), this, SLOT(OnVoltageChanged()));
  connect(ui->Mode, SIGNAL(currentTextChanged(QString)), this, SLOT(OnModeChanged()));

  //Receive
  connect(ui->ScanDepth, SIGNAL(valueChanged(double)), this, SLOT(OnScanDepthChanged()));
  connect(ui->AveragingCount, SIGNAL(valueChanged(int)), this, SLOT(OnAveragingCountChanged()));
  connect(ui->TimeGainCompensationMinSlider, SIGNAL(valueChanged(int)), this, SLOT(OnTGCMinChanged()));
  connect(ui->TimeGainCompensationMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(OnTGCMaxChanged()));
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

void QmitkUSControlsCustomDiPhASDeviceWidget::OnCompensateEnergyChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  bool CompensateEnergy = ui->CompensateEnergy->isChecked();
  m_ControlInterface->SetCompensateEnergy(CompensateEnergy);
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnUseBModeFilterChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  bool UseBModeFilter = ui->UseBModeFilter->isChecked();
  m_ControlInterface->SetUseBModeFilter(UseBModeFilter);
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnRecordChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  if (ui->StartStopRecord->text() == "Start Recording")
  {
    ui->StartStopRecord->setText("Stop Recording");

    ui->UseBModeFilter->setEnabled(false);
    ui->ScatteringCoefficient->setEnabled(false);
    ui->CompensateScattering->setEnabled(false);
    ui->VerticalSpacing->setEnabled(false);
    ui->SaveBeamformed->setEnabled(false);
    ui->SaveRaw->setEnabled(false);
    ui->TransmitPhaseLength->setEnabled(false);
    ui->ExcitationFrequency->setEnabled(false);
    ui->TransmitEvents->setEnabled(false);
    ui->Voltage->setEnabled(false);
    ui->Mode->setEnabled(false);
    ui->ScanDepth->setEnabled(false);
    ui->AveragingCount->setEnabled(false);
    ui->TimeGainCompensationMinSlider->setEnabled(false);
    ui->TimeGainCompensationMaxSlider->setEnabled(false);
    ui->DataType->setEnabled(false);
    ui->PitchOfTransducer->setEnabled(false);
    ui->ReconstructedSamplesPerLine->setEnabled(false);
    ui->ReconstructedLines->setEnabled(false);
    ui->SpeedOfSound->setEnabled(false);
    ui->BandpassEnabled->setEnabled(false);
    ui->LowCut->setEnabled(false);
    ui->HighCut->setEnabled(false);
    ui->CompensateEnergy->setEnabled(false);

    m_ControlInterface->SetRecord(true);
  }
  else
  {
    ui->StartStopRecord->setText("Start Recording");

    ui->UseBModeFilter->setEnabled(true);
    ui->CompensateScattering->setEnabled(true);
    if(ui->CompensateScattering->isChecked())
      ui->ScatteringCoefficient->setEnabled(true);
    ui->VerticalSpacing->setEnabled(true);
    ui->SaveBeamformed->setEnabled(true);
    ui->SaveRaw->setEnabled(true);
    ui->TransmitPhaseLength->setEnabled(true);
    ui->ExcitationFrequency->setEnabled(true);
    ui->TransmitEvents->setEnabled(true);
    ui->Voltage->setEnabled(true);
    ui->Mode->setEnabled(true);
    ui->ScanDepth->setEnabled(true);
    ui->AveragingCount->setEnabled(true);
    ui->TimeGainCompensationMinSlider->setEnabled(true);
    ui->TimeGainCompensationMaxSlider->setEnabled(true);
    ui->DataType->setEnabled(true);
    ui->PitchOfTransducer->setEnabled(true);
    ui->ReconstructedSamplesPerLine->setEnabled(true);
    ui->ReconstructedLines->setEnabled(true);
    ui->SpeedOfSound->setEnabled(true);
    ui->BandpassEnabled->setEnabled(true);
    ui->LowCut->setEnabled(true);
    ui->HighCut->setEnabled(true);
    ui->CompensateEnergy->setEnabled(true);

    m_ControlInterface->SetRecord(false);
  }
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnVerticalSpacingChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetVerticalSpacing(ui->VerticalSpacing->value());
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnScatteringCoefficientChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetScatteringCoefficient(ui->ScatteringCoefficient->value());
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnCompensateScatteringChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  if (ui->CompensateScattering->isChecked())
    ui->ScatteringCoefficient->setEnabled(true);
  else
    ui->ScatteringCoefficient->setEnabled(false);

  m_ControlInterface->SetCompensateScattering(ui->CompensateScattering->isChecked());
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnChangedSavingSettings()
{
  if (m_ControlInterface.IsNull()) { return; }

  mitk::USDiPhASDeviceCustomControls::SavingSettings settings;

  settings.saveBeamformed = ui->SaveBeamformed->isChecked();
  settings.saveRaw = ui->SaveRaw->isChecked();

  m_ControlInterface->SetSavingSettings(settings);
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

  int tgcMin = ui->TimeGainCompensationMinSlider->value();
  int tgcMax = ui->TimeGainCompensationMaxSlider->value();
  if (tgcMin > tgcMax) {
    ui->TimeGainCompensationMinSlider->setValue(tgcMax);
    MITK_INFO << "User tried to set tgcMin>tgcMax.";
  }
  QString text("TGC min = " + QString::fromStdString(std::to_string(ui->TimeGainCompensationMinSlider->value())));
  ui->TimeGainCompensationMinLabel->setText(text);
  m_ControlInterface->SetTGCMin(ui->TimeGainCompensationMinSlider->value());
}
void QmitkUSControlsCustomDiPhASDeviceWidget::OnTGCMaxChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  int tgcMin = ui->TimeGainCompensationMinSlider->value();
  int tgcMax = ui->TimeGainCompensationMaxSlider->value();
  if (tgcMin > tgcMax) {
    ui->TimeGainCompensationMaxSlider->setValue(tgcMin);
    MITK_INFO << "User tried to set tgcMin>tgcMax.";
  }
  QString text("TGC max = "+QString::fromStdString(std::to_string(ui->TimeGainCompensationMaxSlider->value())));
  ui->TimeGainCompensationMaxLabel->setText(text);
  m_ControlInterface->SetTGCMax(ui->TimeGainCompensationMaxSlider->value());
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnDataTypeChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  QString DataType = ui->DataType->currentText();
  if (DataType == "Image Data") {
    m_ControlInterface->SetDataType(mitk::USDiPhASDeviceCustomControls::DataType::Image_uChar);
  }
  else if (DataType == "Beamformed Data") {
    m_ControlInterface->SetDataType(mitk::USDiPhASDeviceCustomControls::DataType::Beamformed_Short);
  }
}

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
  if (m_OldReconstructionLines == 0)
    m_OldReconstructionLines = ui->ReconstructedLines->value();

  m_ControlInterface->SetReconstructedLines(ui->ReconstructedLines->value());

  ui->PitchOfTransducer->setValue(ui->PitchOfTransducer->value()*((double)m_OldReconstructionLines / (double)ui->ReconstructedLines->value()));
  m_OldReconstructionLines = ui->ReconstructedLines->value();
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
