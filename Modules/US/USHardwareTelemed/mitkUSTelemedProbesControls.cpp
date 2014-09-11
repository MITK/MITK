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

#include "mitkUSTelemedProbesControls.h"
#include "mitkUSTelemedDevice.h"
#include <mitkException.h>

mitk::USTelemedProbesControls::USTelemedProbesControls(itk::SmartPointer<USTelemedDevice> device)
  : mitk::USControlInterfaceProbes(device.GetPointer()),
    m_IsActive(false), m_TelemedDevice(device),
    m_ProbesCollection(0), m_Probe(0)
{
}

mitk::USTelemedProbesControls::~USTelemedProbesControls()
{
  SAFE_RELEASE(m_ProbesCollection);
}

void mitk::USTelemedProbesControls::SetIsActive(bool isActive)
{
  if ( ! m_TelemedDevice )
  {
    MITK_WARN("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Cannot activate probe controls while device is not set.";
    return;
  }

  if ( isActive && m_ProbesCollection == 0 )
  {
    this->CreateProbesCollection();
    this->CreateProbesSet();
  }
  else
  {
  }

  m_IsActive = isActive;
}

bool mitk::USTelemedProbesControls::GetIsActive()
{
  return m_IsActive;
}

std::vector<mitk::USProbe::Pointer> mitk::USTelemedProbesControls::GetProbeSet()
{
  // create a new vector of base class (USProbe) objects, because
  // interface wants a vector of this type
  std::vector<mitk::USProbe::Pointer> usProbes(m_ProbesSet.size(), 0);
  for (unsigned int n = 0; n < m_ProbesSet.size(); ++n)
  {
    usProbes.at(n) = m_ProbesSet.at(n).GetPointer();
  }
  return usProbes;
}

void mitk::USTelemedProbesControls::OnSelectProbe(unsigned int index)
{
  if (index >= m_ProbesSet.size())
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Cannot select probe with index " << index << ". Maximum possible index is " << m_ProbesSet.size()-1 << ".";
    mitkThrow() << "Cannot select probe with index " << index <<
      ". Maximum possible index is " << m_ProbesSet.size()-1 << ".";
  }

  m_TelemedDevice->SetActiveDataView(m_ProbesSet.at(index)->GetUsgDataView());

  m_SelectedProbeIndex = index;
}

void mitk::USTelemedProbesControls::OnSelectProbe(mitk::USProbe::Pointer probe)
{
}

mitk::USProbe::Pointer mitk::USTelemedProbesControls::GetSelectedProbe()
{
  if (m_SelectedProbeIndex >= m_ProbesSet.size())
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
      ". Maximum possible index is " << m_ProbesSet.size()-1 << ".";
    mitkThrow() << "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
      ". Maximum possible index is " << m_ProbesSet.size()-1 << ".";
  }

  return m_ProbesSet.at(m_SelectedProbeIndex).GetPointer();
}

unsigned int mitk::USTelemedProbesControls::GetProbesCount() const
{
  return m_ProbesSet.size();
}

/*void mitk::USTelemedProbesControls::SetTelemedDevice(itk::SmartPointer<USTelemedDevice> telemedDevice)
{
  m_TelemedDevice = telemedDevice;
}*/

void mitk::USTelemedProbesControls::ProbeRemoved(unsigned int index)
{
  MITK_INFO << "Probe removed...";

  if ( m_ProbesSet.size() > index )
  {
    m_ProbesSet.erase(m_ProbesSet.begin() + index);
  }
}

void mitk::USTelemedProbesControls::ProbeAdded(unsigned int index)
{
  MITK_INFO << "Probe arrived...";

  this->CreateProbesCollection();
  this->CreateProbesSet();

  // Activate the added probe, if the added probe is the first probe
  if (m_ProbesSet.size() == 1)
  {
    m_TelemedDevice->SetActiveDataView(m_ProbesSet.at(0)->GetUsgDataView());
  }
}

bool mitk::USTelemedProbesControls::CreateProbesCollection()
{
  IUnknown* tmp_obj = NULL;
  HRESULT hr;

  // get the main API interface from the Telemed device
  Usgfw2Lib::IUsgfw2* usgMainInterface = m_TelemedDevice->GetUsgMainInterface();
  if ( ! usgMainInterface )
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Main interface of Telemed device must not be null.";
    mitkThrow() << "Main interface of Telemed device must not be null.";
  }

  // get probes collection from Telemed API
  hr = usgMainInterface->get_ProbesCollection(&tmp_obj);
  if (FAILED(hr) || ! tmp_obj)
  {
    MITK_WARN("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Error on getting probes collection (" << hr << ").";
    return false;
  }

  // second step for getting probes collection from Telemed API
  SAFE_RELEASE(m_ProbesCollection);
  hr = tmp_obj->QueryInterface(Usgfw2Lib::IID_IUsgCollection,(void**)&m_ProbesCollection);
  SAFE_RELEASE(tmp_obj);
  if (FAILED(hr) || ! m_ProbesCollection)
  {
    MITK_WARN("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Error on querying interface for probes collection (" << hr << ").";
    return false;
  }

  return true;
}

void mitk::USTelemedProbesControls::CreateProbesSet()
{
  if ( ! m_ProbesCollection)
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Cannot get probe set without ProbesCollection being initialized before.";
    mitkThrow() << "Cannot get probe set without ProbesCollection being initialized before.";
  }

  // get number of available probes
  LONG probes_count = 0;
  HRESULT hr = m_ProbesCollection->get_Count(&probes_count);
  if (FAILED(hr)) { mitkThrow() << "Could not get probes count (" << hr << ")."; }

  if ( ! m_TelemedDevice )
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Telemed device must not be null when creating probes set.";
    mitkThrow() << "Telemed device must not be null when creating probes set.";
  }

  // get the main API interface from the Telemed device
  Usgfw2Lib::IUsgfw2* usgMainInterface = m_TelemedDevice->GetUsgMainInterface();
  if ( ! usgMainInterface )
  {
    MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
      << "Usg main interface must not be null when creating probes set.";
    mitkThrow() << "Usg main interface must not be null when creating probes set.";
  }

  // initialize probes set with new vector
  m_ProbesSet = std::vector<mitk::USTelemedProbe::Pointer>(probes_count, 0);

  for (unsigned int n = 0; n < probes_count; ++n)
  {
    // get the probe item from the API collection
    IUnknown* tmp_obj = NULL;
    hr = m_ProbesCollection->Item(n,&tmp_obj);
    if (FAILED(hr))
    {
      MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
        << "Could not get probe with index " << n << ".";
      mitkThrow() << "Could not get probe with index " << n << ".";
    }

    // convert this item to a probe
    Usgfw2Lib::IProbe* probe;
    hr = tmp_obj->QueryInterface(Usgfw2Lib::IID_IProbe,(void**)&probe);
    if (FAILED(hr))
    {
      MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
        << "Error on querying interface for probe with index "<< n << ".";
      mitkThrow() << "Error on querying interface for probe with index "<< n << ".";
    }

    // create main ultrasound scanning object for selected probe
    Usgfw2Lib::IUsgDataView* usgDataView;
    Usgfw2Lib::IUsgDataViewPtr usgDataViewTmp;
    usgDataViewTmp = usgMainInterface->CreateDataView(probe);
    usgDataViewTmp->QueryInterface(Usgfw2Lib::IID_IUsgDataView, (void**)&usgDataView);
    if (! usgDataView)
    {
      MITK_ERROR("USTelemedProbesControls")("USControlInterfaceProbes")
        << "Could not create data view for selected probe.";
      mitkThrow() << "Could not create data view for selected probe.";
    }

    // probe object can be created now from API data
    m_ProbesSet.at(n) = mitk::USTelemedProbe::New(probe, usgDataView);

    SAFE_RELEASE(tmp_obj);
  }
}