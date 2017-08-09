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

#include "mitkUSDiPhASDeviceCustomControls.h"

mitk::USDiPhASDeviceCustomControls::USDiPhASDeviceCustomControls(itk::SmartPointer<USDevice> device)
  : mitk::USAbstractControlInterface(device.GetPointer()), m_IsActive(false), silentUpdate(false)
{
}

mitk::USDiPhASDeviceCustomControls::~USDiPhASDeviceCustomControls()
{
}

void mitk::USDiPhASDeviceCustomControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USDiPhASDeviceCustomControls::GetIsActive()
{
  return m_IsActive;
}

void mitk::USDiPhASDeviceCustomControls::passGUIOut(std::function<void(QString)> /*callback*/) {}

void mitk::USDiPhASDeviceCustomControls::SetSilentUpdate(bool silent)
{
  silentUpdate = silent;
}

bool mitk::USDiPhASDeviceCustomControls::GetSilentUpdate()
{
  return silentUpdate;
}

//Set Functions

void mitk::USDiPhASDeviceCustomControls::SetCompensateEnergy(bool compensate)
{
  this->OnSetCompensateEnergy(compensate);
}

void mitk::USDiPhASDeviceCustomControls::SetUseBModeFilter(bool isSet)
{
  this->OnSetUseBModeFilter(isSet);
}

void mitk::USDiPhASDeviceCustomControls::SetRecord(bool record)
{
  this->OnSetRecord(record);
}

void mitk::USDiPhASDeviceCustomControls::SetVerticalSpacing(float mm)
{
  this->OnSetVerticalSpacing(mm);
}

void  mitk::USDiPhASDeviceCustomControls::SetScatteringCoefficient(float coeff)
{
  this->OnSetScatteringCoefficient(coeff);
}
void  mitk::USDiPhASDeviceCustomControls::SetCompensateScattering(bool compensate)
{
  this->OnSetCompensateScattering(compensate);
}

void mitk::USDiPhASDeviceCustomControls::SetSavingSettings(SavingSettings settings)
{
  this->OnSetSavingSettings(settings);
}

//Transmit
void mitk::USDiPhASDeviceCustomControls::SetTransmitPhaseLength(double us)
{
  this->OnSetTransmitPhaseLength(us);
}

void mitk::USDiPhASDeviceCustomControls::SetExcitationFrequency(double MHz)
{
  this->OnSetExcitationFrequency(MHz);
}

void mitk::USDiPhASDeviceCustomControls::SetTransmitEvents(int events)
{
  this->OnSetTransmitEvents(events);
}

void mitk::USDiPhASDeviceCustomControls::SetVoltage(int voltage)
{
  this->OnSetVoltage(voltage);
}

void mitk::USDiPhASDeviceCustomControls::SetMode(bool interleaved)
{
  this->OnSetMode(interleaved);
}

//Receive
void mitk::USDiPhASDeviceCustomControls::SetScanDepth(double mm)
{
  this->OnSetScanDepth(mm);
}

void mitk::USDiPhASDeviceCustomControls::SetAveragingCount(int count)
{
  this->OnSetAveragingCount(count);
}

void mitk::USDiPhASDeviceCustomControls::SetTGCMin(int min)
{
  this->OnSetTGCMin(min);
}

void mitk::USDiPhASDeviceCustomControls::SetTGCMax(int max)
{
  this->OnSetTGCMax(max);
  
}

void mitk::USDiPhASDeviceCustomControls::SetDataType(DataType type)
{
  this->OnSetDataType(type);
}

//Beamforming
void mitk::USDiPhASDeviceCustomControls::SetPitch(double mm)
{
  this->OnSetPitch(mm);
}

void mitk::USDiPhASDeviceCustomControls::SetReconstructedSamples(int samples)
{
  this->OnSetReconstructedSamples(samples);
}

void mitk::USDiPhASDeviceCustomControls::SetReconstructedLines(int lines)
{
  this->OnSetReconstructedLines(lines);
}

void mitk::USDiPhASDeviceCustomControls::SetSpeedOfSound(int mps)
{
  this->OnSetSpeedOfSound(mps);
}

//Bandpass
void mitk::USDiPhASDeviceCustomControls::SetBandpassEnabled(bool bandpass)
{
  this->OnSetBandpassEnabled(bandpass);
}

void mitk::USDiPhASDeviceCustomControls::SetLowCut(double MHz)
{
  this->OnSetLowCut(MHz);
}

void mitk::USDiPhASDeviceCustomControls::SetHighCut(double MHz)
{
  this->OnSetHighCut(MHz);
}


//OnSetDummies

void mitk::USDiPhASDeviceCustomControls::OnSetCompensateEnergy(bool /*compensate*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetUseBModeFilter(bool /*isSet*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetRecord(bool /*record*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetVerticalSpacing(float /*mm*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetScatteringCoefficient(float /*coeff*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetCompensateScattering(bool /*compensate*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetSavingSettings(SavingSettings /*settings*/) {}
//Transmit
void mitk::USDiPhASDeviceCustomControls::OnSetTransmitPhaseLength(double /*ms*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetExcitationFrequency(double /*MHz*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetTransmitEvents(int /*events*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetVoltage(int /*voltage*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetMode(bool /*interleaved*/) {}
//Receive
void mitk::USDiPhASDeviceCustomControls::OnSetScanDepth(double /*mm*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetAveragingCount(int /*count*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetTGCMin(int /*min*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetTGCMax(int /*max*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetDataType(DataType /*type*/) {}
//Beamforming
void mitk::USDiPhASDeviceCustomControls::OnSetPitch(double /*mm*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetReconstructedSamples(int /*samples*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetReconstructedLines(int /*lines*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetSpeedOfSound(int /*mps*/) {}
//Bandpass
void mitk::USDiPhASDeviceCustomControls::OnSetBandpassEnabled(bool /*bandpass*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetLowCut(double /*MHz*/) {}
void mitk::USDiPhASDeviceCustomControls::OnSetHighCut(double /*MHz*/) {}
