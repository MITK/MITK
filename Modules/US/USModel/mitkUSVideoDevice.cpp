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
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
}

mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
}

mitk::USVideoDevice::~USVideoDevice()
{
  
}

std::string mitk::USVideoDevice::GetDeviceClass(){
  return "org.mitk.modules.us.USVideoDevice";
}


bool mitk::USVideoDevice::OnConnection()
{
  m_Source = mitk::USImageVideoSource::New();
  if (m_SourceIsFile){
    m_Source->SetVideoFileInput(m_FilePath);
  } else {
     m_Source->SetCameraInput(m_DeviceID);
  }
  return true;
}

/**
* \brief Is called during the disconnection process.
*  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
*/
bool mitk::USVideoDevice::OnDisconnection()
{
    // TODO Implement Disconnection Behaviour
  return true;
}

/**
* \brief Is called during the activation process. After this method is finsihed, the device should be generating images
*/
bool mitk::USVideoDevice::OnActivation()
{
    // TODO Implement Activation Behaviour
  return true;
}  


/**
* \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
*/
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
  // Set Output
  this->SetNthOutput(0, result);
}
