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

#include "mitkUSDiPhASCustomControls.h"
#include <cmath>

mitk::USDiPhASCustomControls::USDiPhASCustomControls(USDiPhASDevice* device)
  : mitk::USDiPhASDeviceCustomControls(device), m_IsActive(false), m_device(device), currentBeamformingAlgorithm((int)Beamforming::PlaneWaveCompound)
{
}

mitk::USDiPhASCustomControls::~USDiPhASCustomControls()
{
}

void mitk::USDiPhASCustomControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USDiPhASCustomControls::GetIsActive()
{
  return m_IsActive;
}

void mitk::USDiPhASCustomControls::passGUIOut(std::function<void(QString)> callback)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  callback("initializing");
  imageSource->SetGUIOutput(callback);
}

// OnSet methods

void mitk::USDiPhASCustomControls::OnSetCompensateEnergy(bool compensate)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->ModifyEnergyCompensation(compensate);
}

void mitk::USDiPhASCustomControls::OnSetUseBModeFilter(bool isSet)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->ModifyUseBModeFilter(isSet);
}

void mitk::USDiPhASCustomControls::OnSetRecord(bool record)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->SetRecordingStatus(record);
}

void mitk::USDiPhASCustomControls::OnSetVerticalSpacing(float mm)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->SetVerticalSpacing(mm);
}

void mitk::USDiPhASCustomControls::OnSetScatteringCoefficient(float coeff)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->ModifyScatteringCoefficient(coeff);
}

void mitk::USDiPhASCustomControls::OnSetCompensateScattering(bool compensate)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->ModifyCompensateForScattering(compensate);
}

void mitk::USDiPhASCustomControls::OnSetSavingSettings(SavingSettings settings)
{
  mitk::USDiPhASImageSource* imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  imageSource->SetSavingSettings(settings);
}

//Transmit
void mitk::USDiPhASCustomControls::OnSetTransmitPhaseLength(double us)
{
  m_device->GetScanMode().transmitPhaseLengthSeconds = us/1000000;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetExcitationFrequency(double MHz)
{
  m_device->SetBursts(round(((120 / MHz) - 2) / 2));
  m_device->UpdateScanmode();
  // b = (c/f - 2) * 1/2, where c is the internal clock, f the wanted frequency, b the burst count
}

void mitk::USDiPhASCustomControls::OnSetTransmitEvents(int events)
{
  m_device->GetScanMode().transmitEventsCount = events;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetVoltage(int voltage)
{
  m_device->GetScanMode().voltageV = voltage;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetMode(bool interleaved)
{
  m_device->SetInterleaved(interleaved);
  m_device->UpdateScanmode();
}

//Receive
void mitk::USDiPhASCustomControls::OnSetScanDepth(double mm)
{
  auto& scanMode = m_device->GetScanMode();
  float time = 2 * (0.001 * (mm)) / scanMode.averageSpeedOfSound;
  float timeInMicroSeconds = floor(time *1e6); // this is necessary because sub-microsecond accuracy causes undefined behaviour
  m_device->GetScanMode().receivePhaseLengthSeconds = timeInMicroSeconds*1e-6;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetAveragingCount(int count)
{
  m_device->GetScanMode().averagingCount = count;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetTGCMin(int min)
{
  auto& scanMode = m_device->GetScanMode();
  char range = scanMode.tgcdB[7] - min;
  for (int tgc = 0; tgc < 7; ++tgc)
    scanMode.tgcdB[tgc] = round(tgc*range / 7 + min);

  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetTGCMax(int max)
{
  auto& scanMode = m_device->GetScanMode();
  char range = max - scanMode.tgcdB[0];
  for (int tgc = 1; tgc < 8; ++tgc)
    scanMode.tgcdB[tgc] = round(tgc*range / 7 + scanMode.tgcdB[0]);

  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetDataType(DataType type)
{
  auto& scanMode = m_device->GetScanMode();
  auto imageSource = dynamic_cast<mitk::USDiPhASImageSource*>(m_device->GetUSImageSource().GetPointer());
  switch (type) {
    case DataType::Image_uChar : {
      scanMode.transferBeamformedData = false;
      scanMode.transferImageData = true;
      m_device->UpdateScanmode();
      imageSource->ModifyDataType(DataType::Image_uChar);
      break; 
    }
    case DataType::Beamformed_Short : {
      scanMode.transferBeamformedData = true;
      scanMode.transferImageData = false;
      m_device->UpdateScanmode();
      imageSource->ModifyDataType(DataType::Beamformed_Short);
      break;
    }

    default: 
      MITK_INFO << "Unknown Data Type requested";  
      break;
  }
}
// 0= image; 1= beamformed

//Beamforming
void mitk::USDiPhASCustomControls::OnSetPitch(double mm)
{
  m_device->GetScanMode().reconstructedLinePitchMmOrAngleDegree = mm;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetReconstructedSamples(int samples)
{
  m_device->GetScanMode().reconstructionSamplesPerLine = samples;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetReconstructedLines(int lines)
{
  m_device->GetScanMode().reconstructionLines = lines;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetSpeedOfSound(int mps)
{
  m_device->GetScanMode().averageSpeedOfSound = mps;
  m_device->SetInterleaved(m_device->IsInterleaved()); //update transmit parameters
  m_device->UpdateScanmode();
}

//Bandpass
void mitk::USDiPhASCustomControls::OnSetBandpassEnabled(bool bandpass)
{
  m_device->GetScanMode().bandpassApply = bandpass;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetLowCut(double MHz)
{
  m_device->GetScanMode().bandpassFrequencyLowHz = MHz*1000*1000;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetHighCut(double MHz)
{
  m_device->GetScanMode().bandpassFrequencyHighHz = MHz*1000*1000;
  m_device->UpdateScanmode();
}