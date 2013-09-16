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


// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>



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
  // TODO: implement initialization, if necessary
  return true;
}

bool mitk::USTelemedDevice::OnConnection()
{
  // create main Usgfw2 library object
  HRESULT hr;

  hr = CoCreateInstance(CLSID_Usgfw2, NULL, CLSCTX_INPROC_SERVER, IID_IUsgfw2,(LPVOID*) &m_UsgMainInterface);
  if (FAILED(hr))
  {
    SAFE_RELEASE(m_UsgMainInterface);
    MITK_ERROR("USDevice")("USTelemedDevice") << "Error at connecting to ultrasound device (" << hr << ").";
    return false;
  }

  m_ControlsProbes->SetIsActive(true);

  if ( m_ControlsProbes->GetProbesCount() < 1 )
  {
    MITK_WARN("USDevice")("USTelemedDevice") << "No probe found.";
    return false;
  }

  m_ControlsProbes->SelectProbe(0); // select first probe as a default

  return true;
}

bool mitk::USTelemedDevice::OnDisconnection()
{
  m_ControlsBMode->SetIsActive(false);
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
  return this->StopScanning();
}

void mitk::USTelemedDevice::GenerateData()
{
  mitk::USImage::Pointer result;
  result = m_Image;

  // Set Metadata
  result->SetMetadata(this->m_Metadata);
  // Apply Transformation
  this->ApplyCalibration(result);
  // Set Output
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

/*bool mitk::USTelemedDevice::CreateProbesCollection()
  {
    IUnknown* tmp_obj = NULL;
    HRESULT hr;

    hr = m_UsgMainInterface->get_ProbesCollection(&tmp_obj);
    if (FAILED(hr) || ! tmp_obj)
    {
      MITK_ERROR << "Error on getting probes collection (" << hr << ").";
      return false;
    }

    SAFE_RELEASE(m_ProbesCollection);
    hr = tmp_obj->QueryInterface(IID_IUsgCollection,(void**)&m_ProbesCollection);
    SAFE_RELEASE(tmp_obj);
    if (FAILED(hr) || ! m_ProbesCollection)
    {
      MITK_ERROR << "Error on querying interface for probes collection (" << hr << ").";
      return false;
    }

    return true;
  }*/

/*bool mitk::USTelemedDevice::SelectProbe(int index)
{
  HRESULT hr;

  LONG probes_count = 0;
  hr = m_ProbesCollection->get_Count(&probes_count);
  if (FAILED(hr))
  {
    MITK_ERROR << "Could not get probes count (" << hr << ").";
    return false;
  }

  if (probes_count <= index)
  {
    MITK_ERROR << "Probe " << index << " does not exist.";
    return false;
  }

  IUnknown* tmp_obj = NULL;
  hr = m_ProbesCollection->Item(index,&tmp_obj);
  if (FAILED(hr))
  {
    MITK_ERROR << "Could not get probe with index " << index << ".";
    return false;
  }

  SAFE_RELEASE(m_Probe);
  hr = tmp_obj->QueryInterface(IID_IProbe,(void**)&m_Probe);
  SAFE_RELEASE(tmp_obj);

  if ( FAILED(hr) || ! m_Probe )
  {
    SAFE_RELEASE(m_Probe);
    MITK_ERROR << "Error on querying interface for selected probe.";
    return false;
  }

  // create main ultrasound scanning object for selected probe
  SAFE_RELEASE(m_UsgDataView);
  hr = m_UsgMainInterface->CreateDataView(m_Probe, &m_UsgDataView);
  if (FAILED(hr) || ! m_UsgDataView)
  {
    MITK_ERROR << "Could not create data view for selected probe.";
    return false;
  }

  BSTR probeName;
  m_Probe->get_Name(&probeName);

  MITK_INFO << "Connected probe with name: " << telemed::ConvertWcharToString(probeName);

  return true;
}*/

bool mitk::USTelemedDevice::StopScanning()
{
  HRESULT hr;
  hr = m_UsgDataView->put_ScanState(SCAN_STATE_STOP);

  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Stop scanning failed (" << hr << ").";
  }

  return !FAILED(hr);
}

IUsgfw2* mitk::USTelemedDevice::GetUsgMainInterface()
{
  return m_UsgMainInterface;
}

/*void mitk::USTelemedDevice::SetActiveProbe(IProbe* probe)
{
  m_Probe = probe;

  // create main ultrasound scanning object for selected probe
  SAFE_RELEASE(m_UsgDataView);
  HRESULT hr = m_UsgMainInterface->CreateDataView(m_Probe, &m_UsgDataView);
  if (FAILED(hr) || ! m_UsgDataView) { mitkThrow() << "Could not create data view for selected probe."; }


}*/

void mitk::USTelemedDevice::SetActiveDataView(IUsgDataView* usgDataView)
{
  m_UsgDataView = usgDataView;
  if ( ! m_ImageSource->CreateAndConnectConverterPlugin(m_UsgDataView, SCAN_MODE_B)) { return; }

  m_ControlsBMode->SetUsgDataView(m_UsgDataView);
}