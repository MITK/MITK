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

#include "mitkUSTelemedBModeControls.h"
#include <mitkException.h>

mitk::USTelemedBModeControls::USTelemedBModeControls()
: m_UsgDataView(0),
  m_DepthControl(0), m_GainControl(0), m_RejectionControl(0),
  m_Active(false), m_GainSteps(new double[3]), m_RejectionSteps(new double[3])
{
}

mitk::USTelemedBModeControls::~USTelemedBModeControls()
{
  this->ReleaseControls();

  delete[] m_GainSteps;
  delete[] m_RejectionSteps;
}

void mitk::USTelemedBModeControls::SetUsgDataView( IUsgDataView* usgDataView)
{
  m_UsgDataView = usgDataView;
}

/*bool mitk::USTelemedBModeControls::GetIsInitialized()
{
  return m_DepthControl != 0 && m_GainControl != 0 && m_RejectionControl != 0;
}*/

void mitk::USTelemedBModeControls::SetIsActive(bool active)
{
  if (active)
  {
    HRESULT hr = m_UsgDataView->put_ScanState(SCAN_STATE_STOP);
    if (FAILED(hr)) { mitkThrow() << "Could not stop scanning (" << hr << ")."; }

    // make sure that current scan mode is b mode now
    hr = m_UsgDataView->put_ScanMode(SCAN_MODE_B);
    if (FAILED(hr)) { mitkThrow() << "Could not set scan mode b (" << hr << ")."; }

    hr = m_UsgDataView->put_ScanState(SCAN_STATE_RUN);
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

  return m_Active && scanMode == SCAN_MODE_B;
}

double mitk::USTelemedBModeControls::GetScanningDepth()
{
  RETURN_TelemedValue(m_DepthControl);
}

void mitk::USTelemedBModeControls::SetScanningDepth(double value)
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

void mitk::USTelemedBModeControls::SetScanningGain(double value)
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

void mitk::USTelemedBModeControls::SetScanningRejection(double value)
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

void mitk::USTelemedBModeControls::CreateControls()
{
  // create B mode depth control
  CREATE_TelemedControl(m_DepthControl, m_UsgDataView, IID_IUsgDepth, IUsgDepth, SCAN_MODE_B);

  // create B mode gain control
  CREATE_TelemedControl(m_GainControl, m_UsgDataView, IID_IUsgGain, IUsgGain, SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_GainControl, m_GainSteps);

  // create B mode rejection control
  CREATE_TelemedControl(m_RejectionControl, m_UsgDataView, IID_IUsgRejection2, IUsgRejection2, SCAN_MODE_B);
  GETINOUTPUT_TelemedAvailableValuesBounds(m_RejectionControl, m_RejectionSteps);
}

void mitk::USTelemedBModeControls::ReleaseControls()
{
  SAFE_RELEASE(m_DepthControl);
  SAFE_RELEASE(m_GainControl);
  SAFE_RELEASE(m_RejectionControl);
}