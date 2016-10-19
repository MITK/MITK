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

#include "mitkUSVideoDevice.h"
#include "mitkUSVideoDeviceCustomControls.h"

mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model)
{
  Init();
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
  m_FilePath = "";
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model)
{
  Init();
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
}

mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  Init();
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
  m_FilePath = "";
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  Init();
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
}

mitk::USVideoDevice::~USVideoDevice()
{
  //m_Source->UnRegister();
  m_Source = 0;
}

void mitk::USVideoDevice::Init()
{
  m_Source = mitk::USImageVideoSource::New();
  m_ControlInterfaceCustom = mitk::USVideoDeviceCustomControls::New(this);
  //this->SetNumberOfInputs(1);
  this->SetNumberOfIndexedOutputs(1);

  // mitk::USImage::Pointer output = mitk::USImage::New();
  // output->Initialize();
  this->SetNthOutput(0, this->MakeOutput(0));
}

std::string mitk::USVideoDevice::GetDeviceClass()
{
  return mitk::USVideoDevice::GetDeviceClassStatic();
}

std::string mitk::USVideoDevice::GetDeviceClassStatic()
{
  return "org.mitk.modules.us.USVideoDevice";
}

mitk::USAbstractControlInterface::Pointer mitk::USVideoDevice::GetControlInterfaceCustom()
{
  return m_ControlInterfaceCustom.GetPointer();
}

bool mitk::USVideoDevice::OnInitialization()
{
  // nothing to do at initialization of video device
  return true;
}

bool mitk::USVideoDevice::OnConnection()
{
  if (m_SourceIsFile){
    m_Source->SetVideoFileInput(m_FilePath);
  }
  else {
    m_Source->SetCameraInput(m_DeviceID);
  }
  //SetSourceCropArea();
  return true;
}

bool mitk::USVideoDevice::OnDisconnection()
{
  if (m_DeviceState == State_Activated) this->Deactivate();

  m_Source->ReleaseInput();

  return true;
}

bool mitk::USVideoDevice::OnActivation()
{
  // make sure that video device is ready before aquiring images
  if (!m_Source->GetIsReady())
  {
    MITK_WARN("mitkUSDevice")("mitkUSVideoDevice") << "Could not activate us video device. Check if video grabber is configured correctly.";
    return false;
  }

  MITK_INFO << "Activated UsVideoDevice!";
  return true;
}

bool mitk::USVideoDevice::OnDeactivation()
{
  // happens automatically when m_Active is set to false
  return true;
}

void mitk::USVideoDevice::UnregisterOnService()
{
  if (m_DeviceState == State_Activated) { this->Deactivate(); }
  if (m_DeviceState == State_Connected) { this->Disconnect(); }

  mitk::USDevice::UnregisterOnService();
}

mitk::USImageSource::Pointer mitk::USVideoDevice::GetUSImageSource()
{
  return m_Source.GetPointer();
}

std::vector<mitk::USProbe::Pointer> mitk::USVideoDevice::GetAllProbes()
{
  if (m_Probes.empty())
  {
    MITK_INFO << "No probes exist for this USVideDevice. Empty vector is returned";
  }
  return m_Probes;
}

mitk::USProbe::Pointer mitk::USVideoDevice::GetCurrentProbe()
{
  if (m_CurrentProbe.IsNotNull())
  {
    return m_CurrentProbe;
  }
  else
  {
    return 0;
  }
}

mitk::USProbe::Pointer mitk::USVideoDevice::GetProbeByName(std::string name)
{
  for (std::vector<mitk::USProbe::Pointer>::iterator it = m_Probes.begin(); it != m_Probes.end(); it++)
  {
    if (name.compare((*it)->GetName()) == 0)
      return (*it);
  }
  MITK_INFO << "No probe with given name " << name << " was found.";
  return 0; //no matching probe was found so 0 is returned
}

void mitk::USVideoDevice::RemoveProbeByName(std::string name)
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

void mitk::USVideoDevice::AddNewProbe(mitk::USProbe::Pointer probe)
{
  m_Probes.push_back(probe);
}

bool mitk::USVideoDevice::GetIsSourceFile()
{
  return m_SourceIsFile;
}
