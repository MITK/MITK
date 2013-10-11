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

#include "mitkUSTelemedDevice.h"

#include "mitkUSTelemedSDKHeader.h"

mitk::USTelemedDevice::USTelemedDevice(std::string manufacturer, std::string model)
: mitk::USDevice(manufacturer, model),
  m_ControlsProbes(mitk::USTelemedProbesControls::New()),
  m_ControlsBMode(mitk::USTelemedBModeControls::New()),
  m_ControlsDoppler(mitk::USTelemedDopplerControls::New()),
  m_ImageSource(mitk::USTelemedImageSource::New()), m_UsgMainInterface(0),
  m_Probe(0), m_UsgDataView(0), m_ProbesCollection(0)
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));

  m_ControlsProbes->SetTelemedDevice(this);
}

mitk::USTelemedDevice::~USTelemedDevice()
{
}

std::string mitk::USTelemedDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USTelemedDevice";
}

mitk::USControlInterfaceBMode::Pointer mitk::USTelemedDevice::GetControlInterfaceBMode()
{
  return m_ControlsBMode.GetPointer();
}

mitk::USControlInterfaceProbes::Pointer mitk::USTelemedDevice::GetControlInterfaceProbes()
{
  return m_ControlsProbes.GetPointer();
};

mitk::USControlInterfaceDoppler::Pointer mitk::USTelemedDevice::GetControlInterfaceDoppler()
{
  return m_ControlsDoppler.GetPointer();
};

bool mitk::USTelemedDevice::OnInitialization()
{
  // there is no initialization necessary for this class
  return true;
}

bool mitk::USTelemedDevice::OnConnection()
{
  // create main Telemed API COM library object
  HRESULT hr;

  hr = CoCreateInstance(CLSID_Usgfw2, NULL, CLSCTX_INPROC_SERVER, IID_IUsgfw2,(LPVOID*) &m_UsgMainInterface);
  if (FAILED(hr))
  {
    SAFE_RELEASE(m_UsgMainInterface);
    MITK_ERROR("USDevice")("USTelemedDevice") << "Error at connecting to ultrasound device (" << hr << ").";
    return false;
  }

  // probe controls are available now
  m_ControlsProbes->SetIsActive(true);

  if ( m_ControlsProbes->GetProbesCount() < 1 )
  {
    MITK_WARN("USDevice")("USTelemedDevice") << "No probe found.";
    return false;
  }

  // select first probe as a default
  m_ControlsProbes->SelectProbe(0);

  return true;
}

bool mitk::USTelemedDevice::OnDisconnection()
{
  // control objects cannot be active anymore
  m_ControlsBMode->SetIsActive(false);
  m_ControlsDoppler->SetIsActive(false);
  m_ControlsProbes->SetIsActive(false);

  ReleaseUsgControls();

  return true;
}

bool mitk::USTelemedDevice::OnActivation()
{
  // set scan mode b as default for activation -
  // control interfaces can override this later
  HRESULT hr = m_UsgDataView->put_ScanMode(SCAN_MODE_B);
  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Could not set scan mode b (" << hr << ").";
    return false;
  }

  // start ultrasound scanning with selected scan mode
  hr = m_UsgDataView->put_ScanState(SCAN_STATE_RUN);
  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Start scanning failed (" << hr << ").";
    return false;
  }

  return true;
}

bool mitk::USTelemedDevice::OnDeactivation()
{
  this->StopScanning();
  return true;
}

void mitk::USTelemedDevice::GenerateData()
{
  mitk::USImage::Pointer result;
  result = m_Image;

  // set Metadata
  result->SetMetadata(this->m_Metadata);
  // apply Transformation
  this->ApplyCalibration(result);
  // set Output
  this->SetNthOutput(0, result);
}

mitk::USImageSource::Pointer mitk::USTelemedDevice::GetUSImageSource()
{
  return m_ImageSource.GetPointer();
}

void mitk::USTelemedDevice::ReleaseUsgControls()
{
  if (m_UsgDataView) { this->StopScanning(); };

  SAFE_RELEASE(m_UsgMainInterface);
  SAFE_RELEASE(m_Probe);
  SAFE_RELEASE(m_UsgDataView);
  SAFE_RELEASE(m_ProbesCollection);
}

void mitk::USTelemedDevice::StopScanning()
{
  HRESULT hr;
  hr = m_UsgDataView->put_ScanState(SCAN_STATE_STOP);

  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Stop scanning failed (" << hr << ").";
    mitkThrow() << "Stop scanning failed (" << hr << ").";
  }
}

IUsgfw2* mitk::USTelemedDevice::GetUsgMainInterface()
{
  return m_UsgMainInterface;
}

void mitk::USTelemedDevice::SetActiveDataView(IUsgDataView* usgDataView)
{
  // scan converter plugin is conected to IUsgDataView -> a new plugin
  // must be created when changing IUsgDataView
  m_UsgDataView = usgDataView;
  if ( ! m_ImageSource->CreateAndConnectConverterPlugin(m_UsgDataView, SCAN_MODE_B)) { return; }

  // b mode control object must know about active data view
  m_ControlsBMode->SetUsgDataView(m_UsgDataView);
}