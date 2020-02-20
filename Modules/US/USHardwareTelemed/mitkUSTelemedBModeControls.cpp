/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSTelemedBModeControls.h"
#include "mitkUSTelemedDevice.h"
#include <mitkException.h>

#define TELEMED_FREQUENCY_FACTOR 1000000

mitk::USTelemedBModeControls::USTelemedBModeControls(itk::SmartPointer<USTelemedDevice> device)
: mitk::USControlInterfaceBMode(device.GetPointer()),
  m_UsgDataView(0), m_PowerControl(0), m_FrequencyControl(0),
  m_DepthControl(0), m_GainControl(0), m_RejectionControl(0),
  m_DynamicRangeControl(0),
  m_Active(false), m_PowerSteps(new double[3]),
  m_GainSteps(new double[3]), m_RejectionSteps(new double[3]),
  m_DynamicRangeSteps(new double[3])
{
}

mitk::USTelemedBModeControls::~USTelemedBModeControls()
{
  this->ReleaseControls();

  delete[] m_PowerSteps;
  delete[] m_GainSteps;
  delete[] m_RejectionSteps;
  delete[] m_DynamicRangeSteps;
}

void mitk::USTelemedBModeControls::SetUsgDataView( Usgfw2Lib::IUsgDataView* usgDataView)
{
  m_UsgDataView = usgDataView;
}

void mitk::USTelemedBModeControls::ReinitializeControls( )
{
  this->ReleaseControls();
  this->CreateControls();
}

void mitk::USTelemedBModeControls::SetIsActive(bool active)
{
  if (active)
  {
    HRESULT hr = m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_STOP);
    if (FAILED(hr)) { mitkThrow() << "Could not stop scanning (" << hr << ")."; }

    // make sure that current scan mode is b mode now
    hr = m_UsgDataView->put_ScanMode(Usgfw2Lib::SCAN_MODE_B);
    if (FAILED(hr)) { mitkThrow() << "Could not set scan mode b (" << hr << ")."; }

    hr = m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_RUN);
    if (FAILED(hr)) { mitkThrow() << "Could not start scanning (" << hr << ")."; }

    this->CreateControls();

    m_Active = true;
  }
  else
  {
    this->ReleaseControls();

    m_Active = false;
  }
}

bool mitk::USTelemedBModeControls::GetIsActive( )
{
  // get scan mode, because scan mode must be B mode
  // for this interface being active
  ULONG scanMode;
  HRESULT hr = m_UsgDataView->get_ScanMode(&scanMode);
  if (FAILED(hr)) { mitkThrow() << "Could not get scan mode (" << hr << ")."; }

  return m_Active && scanMode == Usgfw2Lib::SCAN_MODE_B;
}

double mitk::USTelemedBModeControls::GetScanningFrequencyAPI( )
{
  RETURN_TelemedValue(m_FrequencyControl);
}

double mitk::USTelemedBModeControls::GetScanningFrequency( )
{
  return this->GetScanningFrequencyAPI() / TELEMED_FREQUENCY_FACTOR;
}

void mitk::USTelemedBModeControls::OnSetScanningFrequency( double value )
{
  SET_TelemedValue(m_FrequencyControl, value * TELEMED_FREQUENCY_FACTOR);
}

std::vector<double> mitk::USTelemedBModeControls::GetScanningFrequencyValues( )
{
  RETURN_TelemedAvailableValuesWithFactor(m_FrequencyControl, TELEMED_FREQUENCY_FACTOR);
}

double mitk::USTelemedBModeControls::GetScanningPower( )
{
  RETURN_TelemedValue(m_PowerControl);
}

void mitk::USTelemedBModeControls::OnSetScanningPower(double value)
{
  SET_TelemedValue(m_PowerControl, value);
}

double mitk::USTelemedBModeControls::GetScanningPowerMin()
{
  return m_PowerSteps[0];
}

double mitk::USTelemedBModeControls::GetScanningPowerMax()
{
  return m_PowerSteps[1];
}

double mitk::USTelemedBModeControls::GetScanningPowerTick()
{
  return m_PowerSteps[2];
}

double mitk::USTelemedBModeControls::GetScanningDepth()
{
  RETURN_TelemedValue(m_DepthControl);
}

void mitk::USTelemedBModeControls::OnSetScanningDepth(double value)
{
  SET_TelemedValue(m_DepthControl,value);
}

std::vector<double> mitk::USTelemedBModeControls::GetScanningDepthValues()
{
  RETURN_TelemedAvailableValues(m_DepthControl);
}

double mitk::USTelemedBModeControls::GetScanningGain( )
{
  RETURN_TelemedValue(m_GainControl);
}

void mitk::USTelemedBModeControls::OnSetScanningGain(double value)
{
  SET_TelemedValue(m_GainControl, value);
}

double mitk::USTelemedBModeControls::GetScanningGainMin()
{
  return m_GainSteps[0];
}

double mitk::USTelemedBModeControls::GetScanningGainMax()
{
  return m_GainSteps[1];
}

double mitk::USTelemedBModeControls::GetScanningGainTick()
{
  return m_GainSteps[2];
}

double mitk::USTelemedBModeControls::GetScanningRejection()
{
  RETURN_TelemedValue(m_RejectionControl);
}

void mitk::USTelemedBModeControls::OnSetScanningRejection(double value)
{
  SET_TelemedValue(m_RejectionControl, value);
}

double mitk::USTelemedBModeControls::GetScanningRejectionMin( )
{
  return m_RejectionSteps[0];
}

double mitk::USTelemedBModeControls::GetScanningRejectionMax( )
{
  return m_RejectionSteps[1];
}

double mitk::USTelemedBModeControls::GetScanningRejectionTick( )
{
  return m_RejectionSteps[2];
}

double mitk::USTelemedBModeControls::GetScanningDynamicRange( )
{
  RETURN_TelemedValue(m_DynamicRangeControl);
}

void mitk::USTelemedBModeControls::OnSetScanningDynamicRange( double value )
{
  SET_TelemedValue(m_DynamicRangeControl, value);
}

double mitk::USTelemedBModeControls::GetScanningDynamicRangeMin( )
{
  return m_DynamicRangeSteps[0];
}

double mitk::USTelemedBModeControls::GetScanningDynamicRangeMax( )
{
  return m_DynamicRangeSteps[1];
}

double mitk::USTelemedBModeControls::GetScanningDynamicRangeTick( )
{
  return m_DynamicRangeSteps[2];
}

void mitk::USTelemedBModeControls::CreateControls()
{
  // create frequency control
  CREATE_TelemedControl(m_FrequencyControl, m_UsgDataView, Usgfw2Lib::IID_IUsgProbeFrequency2, Usgfw2Lib::IUsgProbeFrequency2, Usgfw2Lib::SCAN_MODE_B);

  // create power control
  CREATE_TelemedControl(m_PowerControl, m_UsgDataView, Usgfw2Lib::IID_IUsgPower, Usgfw2Lib::IUsgPower, Usgfw2Lib::SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_PowerControl, m_PowerSteps); // get min, max and tick for gain

  // create B mode depth control
  CREATE_TelemedControl(m_DepthControl, m_UsgDataView, Usgfw2Lib::IID_IUsgDepth, Usgfw2Lib::IUsgDepth, Usgfw2Lib::SCAN_MODE_B);

  // create B mode gain control
  CREATE_TelemedControl(m_GainControl, m_UsgDataView, Usgfw2Lib::IID_IUsgGain, Usgfw2Lib::IUsgGain, Usgfw2Lib::SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_GainControl, m_GainSteps); // get min, max and tick for gain

  // create B mode rejection control
  CREATE_TelemedControl(m_RejectionControl, m_UsgDataView, Usgfw2Lib::IID_IUsgRejection2, Usgfw2Lib::IUsgRejection2, Usgfw2Lib::SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_RejectionControl, m_RejectionSteps); // get min, max and tick for rejection

  // create B mode dynamic range control
  CREATE_TelemedControl(m_DynamicRangeControl, m_UsgDataView, Usgfw2Lib::IID_IUsgDynamicRange, Usgfw2Lib::IUsgDynamicRange, Usgfw2Lib::SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_DynamicRangeControl, m_DynamicRangeSteps); // get min, max and tick for dynamic range
}

void mitk::USTelemedBModeControls::ReleaseControls()
{
  // remove all controls and delete their objects
  SAFE_RELEASE(m_PowerControl);
  SAFE_RELEASE(m_FrequencyControl);
  SAFE_RELEASE(m_DepthControl);
  SAFE_RELEASE(m_GainControl);
  SAFE_RELEASE(m_RejectionControl);
  SAFE_RELEASE(m_DynamicRangeControl);
}
