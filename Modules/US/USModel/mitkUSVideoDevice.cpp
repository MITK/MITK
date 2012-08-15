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


mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
  m_Source = mitk::USImageVideoSource::New();
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
  m_Source = mitk::USImageVideoSource::New();
}

mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
  m_Source = mitk::USImageVideoSource::New();
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
  m_Source = mitk::USImageVideoSource::New();
}

mitk::USVideoDevice::~USVideoDevice()
{
  
}

std::string mitk::USVideoDevice::GetDeviceClass(){
  return "org.mitk.modules.us.USVideoDevice";
}


bool mitk::USVideoDevice::OnConnection()
{
  if (m_SourceIsFile){
    m_Source->SetVideoFileInput(m_FilePath);
  } else {
     m_Source->SetCameraInput(m_DeviceID);
  }
  return true;
}

bool mitk::USVideoDevice::OnDisconnection()
{
    // TODO Implement Disconnection Behaviour
  return true;
}


bool mitk::USVideoDevice::OnActivation()
{
    // TODO Implement Activation Behaviour
  return true;
}  


void mitk::USVideoDevice::OnDeactivation()
{
    // TODO Implement Deactivation Behaviour
}

void mitk::USVideoDevice::GenerateData()
{
  mitk::USImage::Pointer result;
  result = m_Source->GetNextImage();
  
  // Set Metadata
  result->SetMetadata(this->m_Metadata);
  //Apply Transformation
  this->ApplyCalibration(result);
  // Set Output
  this->SetNthOutput(0, result);
}
