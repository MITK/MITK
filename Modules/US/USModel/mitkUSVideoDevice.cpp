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
#include "mitkImageReadAccessor.h"

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
  if (m_ThreadID >= 0)
  {
    m_MultiThreader->TerminateThread(m_ThreadID);
  }
}

void mitk::USVideoDevice::Init()
{
  m_ThreadID = -1; // initialize with invalid id

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
  SetSourceCropArea();
  return true;
}

bool mitk::USVideoDevice::OnDisconnection()
{
  if (m_IsActive) this->Deactivate();
  return true;
}

bool mitk::USVideoDevice::OnActivation()
{
  // make sure that video device is ready before aquiring images
  if ( ! m_Source->GetIsReady() )
  {
    MITK_WARN("mitkUSDevice")("mitkUSVideoDevice") << "Could not activate us video device. Check if video grabber is configured correctly.";
    return false;
  }

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
  m_ImageMutex->Lock();

  if ( m_Image.IsNull() || ! m_Image->IsInitialized() ) { m_ImageMutex->Unlock(); return; }

  mitk::USImage::Pointer result = this->GetOutput();

  if ( ! result->IsInitialized() )
  {
    result->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
  }

  mitk::ImageReadAccessor inputReadAccessor(m_Image.GetPointer(), m_Image->GetSliceData(0,0,0));
  result->SetSlice(inputReadAccessor.GetData());

  m_ImageMutex->Unlock();

  // Set Metadata
  result->SetMetadata(this->m_Metadata);
  //Apply Transformation
  this->ApplyCalibration(result);
  // Set Output
  this->SetNthOutput(0, result);
}

void mitk::USVideoDevice::GrabImage()
{
  mitk::USImage::Pointer image = m_Source->GetNextImage();

  this->m_ImageMutex->Lock();
  m_Image = image;
  this->m_ImageMutex->Unlock();

  //this->SetNthOutput(0, m_Image);
  //this->Modified();
}

void mitk::USVideoDevice::SetSourceCropArea()
{
if (this->m_Source.IsNotNull())
  {
    if((m_CropArea.cropBottom==0)&&
       (m_CropArea.cropTop==0)&&
       (m_CropArea.cropLeft==0)&&
       (m_CropArea.cropRight==0))
      {this->m_Source->RemoveRegionOfInterest();}
    else
      {
      int right = m_Source->GetImageWidth() - m_CropArea.cropRight;
      int bottom = m_Source->GetImageHeight() - m_CropArea.cropBottom;
      this->m_Source->SetRegionOfInterest(m_CropArea.cropLeft,
                                          m_CropArea.cropTop,
                                          right,
                                          bottom);
      }
  }
else
  {MITK_WARN << "Cannot set crop are, source is not initialized!";}
}

void mitk::USVideoDevice::SetCropArea(mitk::USDevice::USImageCropArea newArea)
{
m_CropArea = newArea;
MITK_INFO << "Set Crop Area L:" << m_CropArea.cropLeft << " R:" << m_CropArea.cropRight << " T:" << m_CropArea.cropTop << " B:" << m_CropArea.cropBottom;
if (m_IsConnected) SetSourceCropArea();
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

