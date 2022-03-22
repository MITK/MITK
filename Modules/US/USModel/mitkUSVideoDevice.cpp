/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  m_Source = nullptr;
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

void mitk::USVideoDevice::GenerateData()
{
  Superclass::GenerateData();

  if( m_ImageVector.size() == 0 || this->GetNumberOfIndexedOutputs() == 0 )
  {
    return;
  }

  m_ImageMutex.lock();
  auto& image = m_ImageVector[0];
  if( image.IsNotNull() && image->IsInitialized() && m_CurrentProbe.IsNotNull() )
  {
    //MITK_INFO << "Spacing CurrentProbe: " << m_CurrentProbe->GetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth());
    image->GetGeometry()->SetSpacing(m_CurrentProbe->GetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth()));
    this->GetOutput(0)->SetGeometry(image->GetGeometry());
  }
  m_ImageMutex.unlock();
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

void mitk::USVideoDevice::DeleteAllProbes()
{
  m_Probes.clear();
}

mitk::USProbe::Pointer mitk::USVideoDevice::GetCurrentProbe()
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

mitk::USProbe::Pointer mitk::USVideoDevice::GetProbeByName(std::string name)
{
  for (std::vector<mitk::USProbe::Pointer>::iterator it = m_Probes.begin(); it != m_Probes.end(); it++)
  {
    if (name.compare((*it)->GetName()) == 0)
      return (*it);
  }
  MITK_INFO << "No probe with given name " << name << " was found.";
  return nullptr; //no matching probe was found so 0 is returned
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

void mitk::USVideoDevice::SetDefaultProbeAsCurrentProbe()
{
  if( m_Probes.size() == 0 )
  {
    std::string name = "default";
    mitk::USProbe::Pointer defaultProbe = mitk::USProbe::New( name );
    m_Probes.push_back( defaultProbe );
  }

  m_CurrentProbe = m_Probes.at(0);
  MITK_INFO << "SetDefaultProbeAsCurrentProbe()";
  this->ProbeChanged( m_CurrentProbe->GetName() );
}

void mitk::USVideoDevice::SetCurrentProbe(std::string probename)
{
  m_CurrentProbe = this->GetProbeByName( probename );
  MITK_INFO << "SetCurrentProbe() " << probename;
}

void mitk::USVideoDevice::SetSpacing(double xSpacing, double ySpacing)
{
  mitk::Vector3D spacing;
  spacing[0] = xSpacing;
  spacing[1] = ySpacing;
  spacing[2] = 1;
  MITK_INFO << "Spacing: " << spacing;

  if( m_CurrentProbe.IsNotNull() )
  {
    m_CurrentProbe->SetSpacingForGivenDepth(m_CurrentProbe->GetCurrentDepth(), spacing);
  }
  else
  {
    MITK_WARN << "Cannot set spacing. Current ultrasound probe not set.";
  }
}
