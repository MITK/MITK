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
#include <string>


mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model, true)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_Source = mitk::USImageVideoSource::New();
  m_Source->SetCameraInput(videoDeviceNumber);
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model, true)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_Source = mitk::USImageVideoSource::New();
  m_Source->SetVideoFileInput(videoFilePath);
}

mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata, true)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_Source = mitk::USImageVideoSource::New();
  m_Source->SetCameraInput(videoDeviceNumber);
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata, true)
{
  this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);
  m_Source = mitk::USImageVideoSource::New();
  m_Source->SetVideoFileInput(videoFilePath);
}

mitk::USVideoDevice::~USVideoDevice()
{
  
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
