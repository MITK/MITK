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
  : mitk::USDiPhASDeviceCustomControls(device), m_IsActive(false), m_device(device)
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

//Transmit
void mitk::USDiPhASCustomControls::OnSetTransmitPhaseLength(double ms)
{
  m_device->GetScanMode().transmitPhaseLengthSeconds = ms*1000;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetExcitationFrequency(double MHz)
{
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

}

//Receive
void mitk::USDiPhASCustomControls::OnSetScanDepth(double mm)
{
  auto scanMode = m_device->GetScanMode();
  int time = 2 * (1000 * mm) / scanMode.averageSpeedOfSound;
  m_device->GetScanMode().receivePhaseLengthSeconds = time;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetAveragingCount(int count)
{
  m_device->GetScanMode().averagingCount = count;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetTGCMin(int min)
{
  auto scanMode = m_device->GetScanMode();
  char range = scanMode.tgcdB[7] - min;
  for (char tgc = 0; tgc < 8; ++tgc){
  scanMode.tgcdB[tgc] = (range*(tgc + 1) / 8);
}
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetTGCMax(int max)
{
  auto scanMode = m_device->GetScanMode();
  char range = max - scanMode.tgcdB[0];
  for (char tgc = 0; tgc < 8; ++tgc)
    scanMode.tgcdB[tgc] = (range*(tgc + 1) / 8);
  m_device->UpdateScanmode();

  for (int i = 0; i < 8; i++)
    MITK_INFO << (int)scanMode.tgcdB[i];
}

void mitk::USDiPhASCustomControls::OnSetDataType(int type)
{
}
// 0= raw; 1= beamformed; 2= imageData;

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
  m_device->GetScanMode().bandpassFrequencyLowHz = MHz;
  m_device->UpdateScanmode();
}

void mitk::USDiPhASCustomControls::OnSetHighCut(double MHz)
{
  m_device->GetScanMode().bandpassFrequencyHighHz = MHz;
  m_device->UpdateScanmode();
}