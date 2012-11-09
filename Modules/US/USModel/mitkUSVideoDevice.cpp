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
  Init();
  m_SourceIsFile = false;
  m_DeviceID = videoDeviceNumber;
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
}

mitk::USVideoDevice::USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata) : mitk::USDevice(metadata)
{
  Init();
  m_SourceIsFile = true;
  m_FilePath = videoFilePath;
}

mitk::USVideoDevice::~USVideoDevice()
{

}

void mitk::USVideoDevice::Init()
{
  m_Source = mitk::USImageVideoSource::New();
  //this->SetNumberOfInputs(1);
  this->SetNumberOfOutputs(1);

  // mitk::USImage::Pointer output = mitk::USImage::New();
  // output->Initialize();
  this->SetNthOutput(0, this->MakeOutput(0));

  this->m_MultiThreader = itk::MultiThreader::New();
  this->m_ImageMutex = itk::FastMutexLock::New();
  this->m_CameraActiveMutex= itk::FastMutexLock::New();
  m_IsActive = false;
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
  if (m_IsActive) this->Deactivate();
  return true;
}


bool mitk::USVideoDevice::OnActivation()
{
  MITK_INFO << "Activated UsVideoDevice!";
  this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->Acquire, this);
  return true;
}


void mitk::USVideoDevice::OnDeactivation()
{
   // happens automatically when m_Active is set to false
}

void mitk::USVideoDevice::GenerateData()
{
  mitk::USImage::Pointer result;
  result = m_Image;

  // Set Metadata
  result->SetMetadata(this->m_Metadata);
  //Apply Transformation
  this->ApplyCalibration(result);
  // Set Output
  this->SetNthOutput(0, result);
}

void mitk::USVideoDevice::GrabImage()
{
  m_Image = m_Source->GetNextImage();
  //this->SetNthOutput(0, m_Image);
  //this->Modified();
}

ITK_THREAD_RETURN_TYPE mitk::USVideoDevice::Acquire(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  mitk::USVideoDevice * device = (mitk::USVideoDevice *) pInfo->UserData;
  while (device->GetIsActive())
  {
    device->GrabImage();
  }
  return ITK_THREAD_RETURN_VALUE;
}

