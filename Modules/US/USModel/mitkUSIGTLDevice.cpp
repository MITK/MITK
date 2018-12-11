/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY{} without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkUSIGTLDevice.h>

mitk::USIGTLDevice::USIGTLDevice(std::string manufacturer, std::string model,
  std::string host, int port, bool server)
  : mitk::USDevice(manufacturer, model), m_Host(host), m_Port(port)
{
  m_ControlInterfaceCustom = mitk::USVideoDeviceCustomControls::New(this);
  if (server)
  {
    m_Device = mitk::IGTLServer::New(true);
  }
  else
  {
    m_Device = mitk::IGTLClient::New(true);
  }
  m_Device->SetPortNumber(m_Port);
  m_Device->SetHostname(m_Host);
  m_Device->SetName(manufacturer + " - " + model);

  m_TransformDeviceSource = mitk::IGTLTrackingDataDeviceSource::New();
  m_TransformDeviceSource->SetIGTLDevice(m_Device);
  m_TransformDeviceSource->RegisterAsMicroservice();

  m_DeviceSource = mitk::IGTL2DImageDeviceSource::New();
  m_DeviceSource->SetIGTLDevice(m_Device);
  m_DeviceSource->RegisterAsMicroservice();

  m_Filter = mitk::IGTLMessageToUSImageFilter::New();
  m_Filter->SetNumberOfExpectedOutputs(1);
  m_Filter->ConnectTo(m_DeviceSource);
}

std::string mitk::USIGTLDevice::GetDeviceClass() { return "IGTL Client"; }

mitk::USImageSource::Pointer mitk::USIGTLDevice::GetUSImageSource()
{
  return m_Filter.GetPointer();
}

mitk::USAbstractControlInterface::Pointer mitk::USIGTLDevice::GetControlInterfaceCustom()
{
  return m_ControlInterfaceCustom.GetPointer();
}

void mitk::USIGTLDevice::UnregisterOnService()
{
  m_DeviceSource->UnRegisterMicroservice();
  m_TransformDeviceSource->UnRegisterMicroservice();
  mitk::USDevice::UnregisterOnService();
}

std::vector<mitk::USProbe::Pointer> mitk::USIGTLDevice::GetAllProbes()
{
  if (m_Probes.empty())
  {
    MITK_INFO << "No probes exist for this USVideDevice. Empty vector is returned";
  }
  return m_Probes;
}

void mitk::USIGTLDevice::DeleteAllProbes()
{
  m_Probes.clear();
}

mitk::USProbe::Pointer mitk::USIGTLDevice::GetCurrentProbe()
{
  if (m_CurrentProbe.IsNotNull())
  {
    return m_CurrentProbe;
  }
  else
  {
    return nullptr;
  }
}

void mitk::USIGTLDevice::AddNewProbe(mitk::USProbe::Pointer probe)
{
  m_Probes.push_back(probe);
}

mitk::USProbe::Pointer mitk::USIGTLDevice::GetProbeByName(std::string name)
{
  for (std::vector<mitk::USProbe::Pointer>::iterator it = m_Probes.begin(); it != m_Probes.end(); it++)
  {
    if (name.compare((*it)->GetName()) == 0)
      return (*it);
  }
  MITK_INFO << "No probe with given name " << name << " was found.";
  return nullptr; //no matching probe was found so 0 is returned
}

void mitk::USIGTLDevice::RemoveProbeByName(std::string name)
{
  for (std::vector<mitk::USProbe::Pointer>::iterator it = m_Probes.begin(); it != m_Probes.end(); it++)
  {
    if (name.compare((*it)->GetName()) == 0)
    {
      m_Probes.erase(it);
      return;
    }
  }
  MITK_INFO << "No Probe with given name " << name << " was found";
}

void mitk::USIGTLDevice::SetDefaultProbeAsCurrentProbe()
{
  if (m_Probes.size() == 0)
  {
    std::string name = "default";
    mitk::USProbe::Pointer defaultProbe = mitk::USProbe::New(name);
    m_Probes.push_back(defaultProbe);
  }

  m_CurrentProbe = m_Probes.at(0);
  MITK_INFO << "SetDefaultProbeAsCurrentProbe()";
  this->ProbeChanged(m_CurrentProbe->GetName());
}

void mitk::USIGTLDevice::SetCurrentProbe(std::string probename)
{
  m_CurrentProbe = this->GetProbeByName(probename);
  MITK_INFO << "SetCurrentProbe() " << probename;
}

void mitk::USIGTLDevice::SetSpacing(double xSpacing, double ySpacing)
{
  mitk::Vector3D spacing;
  spacing[0] = xSpacing;
  spacing[1] = ySpacing;
  spacing[2] = 1;
  MITK_INFO << "Spacing: " << spacing;

  if (m_CurrentProbe.IsNotNull())
  {
    m_CurrentProbe->SetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth(), spacing);
  }
  else
  {
    MITK_WARN << "Cannot set spacing. Current ultrasound probe not set.";
  }
}

bool mitk::USIGTLDevice::OnInitialization() { return true; }

bool mitk::USIGTLDevice::OnConnection()
{
  if (m_Device->GetState() == mitk::IGTLDevice::IGTLDeviceState::Running ||
      m_Device->GetState() == mitk::IGTLDevice::IGTLDeviceState::Ready)
  {
    MITK_INFO << "Device is ready or running. So return true";
    return true;
  }
  return m_Device->OpenConnection();
}

bool mitk::USIGTLDevice::OnDisconnection()
{
  return m_Device->CloseConnection();
}

bool mitk::USIGTLDevice::OnActivation()
{
  if (m_Device->GetState() == mitk::IGTLDevice::IGTLDeviceState::Running )
  {
    MITK_INFO << "Device is running. So return true";
    return true;
  }
  return m_Device->StartCommunication();
}

bool mitk::USIGTLDevice::OnDeactivation()
{
  return m_Device->StopCommunication();
}

void mitk::USIGTLDevice::GenerateData()
{
  Superclass::GenerateData();
  if (m_ImageVector.size() == 0 || this->GetNumberOfIndexedOutputs() == 0)
  {
    return;
  }

  m_ImageMutex->Lock();
  auto& image = m_ImageVector[0];
  if (image.IsNotNull() && image->IsInitialized() && m_CurrentProbe.IsNotNull())
  {
    //MITK_INFO << "Spacing CurrentProbe: " << m_CurrentProbe->GetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth());
    image->GetGeometry()->SetSpacing(m_CurrentProbe->GetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth()));
    this->GetOutput(0)->SetGeometry(image->GetGeometry());
  }
  m_ImageMutex->Unlock();
}
