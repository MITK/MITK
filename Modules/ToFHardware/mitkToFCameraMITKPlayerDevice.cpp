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
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkToFCameraMITKPlayerController.h"
#include "mitkRealTimeClock.h"

#include <iostream>
#include <fstream>
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>

namespace mitk
{
ToFCameraMITKPlayerDevice::ToFCameraMITKPlayerDevice() :
  m_DistanceDataBuffer(NULL), m_AmplitudeDataBuffer(NULL), m_IntensityDataBuffer(NULL), m_RGBDataBuffer(NULL)
{
  m_Controller = ToFCameraMITKPlayerController::New();
}

ToFCameraMITKPlayerDevice::~ToFCameraMITKPlayerDevice()
{
  DisconnectCamera();
  CleanUpDataBuffers();
}

bool ToFCameraMITKPlayerDevice::OnConnectCamera()
{
  bool ok = m_Controller->OpenCameraConnection();
  if (ok)
  {
    this->m_CaptureWidth = m_Controller->GetCaptureWidth();
    this->m_CaptureHeight = m_Controller->GetCaptureHeight();
    this->m_RGBImageWidth = m_Controller->GetRGBCaptureWidth();
    this->m_RGBImageHeight = m_Controller->GetRGBCaptureHeight();
    this->m_PixelNumber = m_Controller->GetPixelNumber();
    this->m_RGBPixelNumber = m_Controller->GetRGBPixelNumber();

    if(m_RGBPixelNumber != m_PixelNumber)
      this->SetBoolProperty("RGBImageHasDifferentResolution", true);


    AllocatePixelArrays();
    AllocateDataBuffers();

    m_CameraConnected = true;
  }
  return ok;
}

bool ToFCameraMITKPlayerDevice::DisconnectCamera()
{
  bool ok =  m_Controller->CloseCameraConnection();
  if (ok)
  {
    m_CameraConnected = false;
    m_PropertyList->Clear();
  }
  return ok;
}

void ToFCameraMITKPlayerDevice::StartCamera()
{
  if (m_CameraConnected)
  {
    // get the first image
    this->m_Controller->UpdateCamera();
    this->m_ImageMutex->Lock();
    this->m_Controller->GetDistances(this->m_DistanceDataBuffer[this->m_FreePos]);
    this->m_Controller->GetAmplitudes(this->m_AmplitudeDataBuffer[this->m_FreePos]);
    this->m_Controller->GetIntensities(this->m_IntensityDataBuffer[this->m_FreePos]);
    this->m_Controller->GetRgb(this->m_RGBDataBuffer[this->m_FreePos]);
    this->m_FreePos = (this->m_FreePos+1) % this->m_BufferSize;
    this->m_CurrentPos = (this->m_CurrentPos+1) % this->m_BufferSize;
    this->m_ImageSequence++;
    this->m_ImageMutex->Unlock();

    this->m_CameraActiveMutex->Lock();
    this->m_CameraActive = true;
    this->m_CameraActiveMutex->Unlock();
    this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->Acquire, this);
    // wait a little to make sure that the thread is started
    itksys::SystemTools::Delay(10);
  }
  else
  {
    MITK_INFO<<"Camera not connected";
  }
}

void ToFCameraMITKPlayerDevice::UpdateCamera()
{
  m_Controller->UpdateCamera();
}

ITK_THREAD_RETURN_TYPE ToFCameraMITKPlayerDevice::Acquire(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  ToFCameraMITKPlayerDevice* toFCameraDevice = (ToFCameraMITKPlayerDevice*)pInfo->UserData;
  if (toFCameraDevice!=NULL)
  {
    mitk::RealTimeClock::Pointer realTimeClock;
    realTimeClock = mitk::RealTimeClock::New();
    int n = 100;
    double t1, t2;
    t1 = realTimeClock->GetCurrentStamp();
    bool overflow = false;
    bool printStatus = false;
    while (toFCameraDevice->IsCameraActive())
    {
      // update the ToF camera
      toFCameraDevice->UpdateCamera();
      // get image data from controller and write it to the according buffer
      toFCameraDevice->m_Controller->GetDistances(toFCameraDevice->m_DistanceDataBuffer[toFCameraDevice->m_FreePos]);
      toFCameraDevice->m_Controller->GetAmplitudes(toFCameraDevice->m_AmplitudeDataBuffer[toFCameraDevice->m_FreePos]);
      toFCameraDevice->m_Controller->GetIntensities(toFCameraDevice->m_IntensityDataBuffer[toFCameraDevice->m_FreePos]);
      toFCameraDevice->m_Controller->GetRgb(toFCameraDevice->m_RGBDataBuffer[toFCameraDevice->m_FreePos]);
      toFCameraDevice->Modified();
      toFCameraDevice->m_ImageMutex->Lock();
      toFCameraDevice->m_FreePos = (toFCameraDevice->m_FreePos+1) % toFCameraDevice->m_BufferSize;
      toFCameraDevice->m_CurrentPos = (toFCameraDevice->m_CurrentPos+1) % toFCameraDevice->m_BufferSize;
      toFCameraDevice->m_ImageSequence++;
      if (toFCameraDevice->m_FreePos == toFCameraDevice->m_CurrentPos)
      {
        overflow = true;
      }
      if (toFCameraDevice->m_ImageSequence % n == 0)
      {
        printStatus = true;
      }
      toFCameraDevice->m_ImageMutex->Unlock();
      if (overflow)
      {
        overflow = false;
      }
      // print current framerate
      if (printStatus)
      {
        t2 = realTimeClock->GetCurrentStamp() - t1;
        MITK_INFO << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << toFCameraDevice->m_ImageSequence;
        t1 = realTimeClock->GetCurrentStamp();
        printStatus = false;
      }
    }  // end of while loop
  }
  return ITK_THREAD_RETURN_VALUE;
}

void ToFCameraMITKPlayerDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  // write amplitude image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    amplitudeArray[i] = this->m_AmplitudeDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex->Unlock();
}

void ToFCameraMITKPlayerDevice::GetIntensities(float* intensityArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  // write intensity image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    intensityArray[i] = this->m_IntensityDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex->Unlock();
}

void ToFCameraMITKPlayerDevice::GetDistances(float* distanceArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  // write distance image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    distanceArray[i] = this->m_DistanceDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex->Unlock();
}

void ToFCameraMITKPlayerDevice::GetRgb(unsigned char* rgbArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  // write intensity image data to unsigned char array
  for (int i=0; i<this->m_RGBPixelNumber*3; i++)
  {
    rgbArray[i] = this->m_RGBDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex->Unlock();
}

void ToFCameraMITKPlayerDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* /*sourceDataArray*/,
                                             int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
{
  m_ImageMutex->Lock();

  //check for empty buffer
  if (this->m_ImageSequence < 0)
  {
    // buffer empty
    MITK_INFO << "Buffer empty!! ";
    capturedImageSequence = this->m_ImageSequence;
    m_ImageMutex->Unlock();
    return;
  }
  // determine position of image in buffer
  int pos = 0;
  if ((requiredImageSequence < 0) || (requiredImageSequence > this->m_ImageSequence))
  {
    capturedImageSequence = this->m_ImageSequence;
    pos = this->m_CurrentPos;
  }
  else if (requiredImageSequence <= this->m_ImageSequence - this->m_BufferSize)
  {
    capturedImageSequence = (this->m_ImageSequence - this->m_BufferSize) + 1;
    pos = (this->m_CurrentPos + 1) % this->m_BufferSize;
  }
  else // (requiredImageSequence > this->m_ImageSequence - this->m_BufferSize) && (requiredImageSequence <= this->m_ImageSequence)
  {
    capturedImageSequence = requiredImageSequence;
    pos = (this->m_CurrentPos + (10-(this->m_ImageSequence - requiredImageSequence))) % this->m_BufferSize;
  }

  if(this->m_DistanceDataBuffer&&this->m_AmplitudeDataBuffer&&this->m_IntensityDataBuffer&&this->m_RGBDataBuffer)
  {
    // write image data to float arrays
    memcpy(distanceArray, this->m_DistanceDataBuffer[pos], this->m_PixelNumber * sizeof(float));
    memcpy(amplitudeArray, this->m_AmplitudeDataBuffer[pos], this->m_PixelNumber * sizeof(float));
    memcpy(intensityArray, this->m_IntensityDataBuffer[pos], this->m_PixelNumber * sizeof(float));
    memcpy(rgbDataArray, this->m_RGBDataBuffer[pos], this->m_RGBPixelNumber * 3 * sizeof(unsigned char));
  }
  m_ImageMutex->Unlock();
}

void ToFCameraMITKPlayerDevice::SetInputFileName(std::string inputFileName)
{
  this->m_InputFileName = inputFileName;
  this->m_Controller->SetInputFileName(inputFileName);
}

void ToFCameraMITKPlayerDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
{
  this->m_PropertyList->SetProperty(propertyKey, propertyValue);

  ToFCameraMITKPlayerController::Pointer myController = dynamic_cast<mitk::ToFCameraMITKPlayerController*>(this->m_Controller.GetPointer());

  std::string strValue;
  GetStringProperty(propertyKey, strValue);
  if (strcmp(propertyKey, "DistanceImageFileName") == 0)
  {
    myController->SetDistanceImageFileName(strValue);
  }
  else if (strcmp(propertyKey, "AmplitudeImageFileName") == 0)
  {
    std::ifstream amplitudeImage(strValue.c_str());
    if(amplitudeImage)
    {
      this->m_PropertyList->SetBoolProperty("HasAmplitudeImage", true);
      myController->SetAmplitudeImageFileName(strValue);
    }
    else
    {
      MITK_WARN << "File " << strValue << " does not exist!";
    }
  }
  else if (strcmp(propertyKey, "IntensityImageFileName") == 0)
  {
    std::ifstream intensityImage(strValue.c_str());
    if(intensityImage)
    {
      this->m_PropertyList->SetBoolProperty("HasIntensityImage", true);
      myController->SetIntensityImageFileName(strValue);
    }
    else
    {
      MITK_WARN << "File " << strValue << " does not exist!";
    }
  }
  else if (strcmp(propertyKey, "RGBImageFileName") == 0)
  {
    std::ifstream intensityImage(strValue.c_str());
    if(intensityImage)
    {
      this->m_PropertyList->SetBoolProperty("HasRGBImage", true);
      myController->SetRGBImageFileName(strValue);
    }
    else
    {
      MITK_WARN << "File " << strValue << " does not exist!";
    }
  }
}

void ToFCameraMITKPlayerDevice::CleanUpDataBuffers()
{
  if (m_DistanceDataBuffer)
  {
    for(int i=0; i<this->m_MaxBufferSize; i++)
    {
      delete[] this->m_DistanceDataBuffer[i];
    }
    delete[] this->m_DistanceDataBuffer;
  }
  if (m_AmplitudeDataBuffer)
  {
    for(int i=0; i<this->m_MaxBufferSize; i++)
    {
      delete[] this->m_AmplitudeDataBuffer[i];
    }
    delete[] this->m_AmplitudeDataBuffer;
  }
  if (m_IntensityDataBuffer)
  {
    for(int i=0; i<this->m_MaxBufferSize; i++)
    {
      delete[] this->m_IntensityDataBuffer[i];
    }
    delete[] this->m_IntensityDataBuffer;
  }
  if (m_RGBDataBuffer)
  {
    for(int i=0; i<this->m_MaxBufferSize; i++)
    {
      delete[] this->m_RGBDataBuffer[i];
    }
    delete[] this->m_RGBDataBuffer;
  }
}

void ToFCameraMITKPlayerDevice::AllocateDataBuffers()
{
  // free memory if it was already allocated
  this->CleanUpDataBuffers();
  // allocate buffers
  this->m_DistanceDataBuffer = new float*[this->m_MaxBufferSize];
  for(int i=0; i<this->m_MaxBufferSize; i++)
  {
    this->m_DistanceDataBuffer[i] = new float[this->m_PixelNumber];
  }
  this->m_AmplitudeDataBuffer = new float*[this->m_MaxBufferSize];
  for(int i=0; i<this->m_MaxBufferSize; i++)
  {
    this->m_AmplitudeDataBuffer[i] = new float[this->m_PixelNumber];
  }
  this->m_IntensityDataBuffer = new float*[this->m_MaxBufferSize];
  for(int i=0; i<this->m_MaxBufferSize; i++)
  {
    this->m_IntensityDataBuffer[i] = new float[this->m_PixelNumber];
  }
  this->m_RGBDataBuffer = new unsigned char*[this->m_MaxBufferSize];
  for(int i=0; i<this->m_MaxBufferSize; i++)
  {
    this->m_RGBDataBuffer[i] = new unsigned char[this->m_RGBPixelNumber*3];
  }
}
}
