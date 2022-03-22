/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkToFCameraMITKPlayerController.h"
#include "mitkRealTimeClock.h"

#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>

namespace mitk
{
ToFCameraMITKPlayerDevice::ToFCameraMITKPlayerDevice() :
  m_DistanceDataBuffer(nullptr), m_AmplitudeDataBuffer(nullptr), m_IntensityDataBuffer(nullptr), m_RGBDataBuffer(nullptr)
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
    this->m_ImageMutex.lock();
    this->m_Controller->GetDistances(this->m_DistanceDataBuffer[this->m_FreePos]);
    this->m_Controller->GetAmplitudes(this->m_AmplitudeDataBuffer[this->m_FreePos]);
    this->m_Controller->GetIntensities(this->m_IntensityDataBuffer[this->m_FreePos]);
    this->m_Controller->GetRgb(this->m_RGBDataBuffer[this->m_FreePos]);
    this->m_FreePos = (this->m_FreePos+1) % this->m_BufferSize;
    this->m_CurrentPos = (this->m_CurrentPos+1) % this->m_BufferSize;
    this->m_ImageSequence++;
    this->m_ImageMutex.unlock();

    this->m_CameraActiveMutex.lock();
    this->m_CameraActive = true;
    this->m_CameraActiveMutex.unlock();
    m_Thread = std::thread(&ToFCameraMITKPlayerDevice::Acquire, this);
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

void ToFCameraMITKPlayerDevice::Acquire()
{
  mitk::RealTimeClock::Pointer realTimeClock;
  realTimeClock = mitk::RealTimeClock::New();
  int n = 100;
  double t1, t2;
  t1 = realTimeClock->GetCurrentStamp();
  bool overflow = false;
  bool printStatus = false;
  while (this->IsCameraActive())
  {
    // update the ToF camera
    this->UpdateCamera();
    // get image data from controller and write it to the according buffer
    m_Controller->GetDistances(m_DistanceDataBuffer[m_FreePos]);
    m_Controller->GetAmplitudes(m_AmplitudeDataBuffer[m_FreePos]);
    m_Controller->GetIntensities(m_IntensityDataBuffer[m_FreePos]);
    m_Controller->GetRgb(m_RGBDataBuffer[m_FreePos]);
    this->Modified();
    m_ImageMutex.lock();
    m_FreePos = (m_FreePos+1) % m_BufferSize;
    m_CurrentPos = (m_CurrentPos+1) % m_BufferSize;
    m_ImageSequence++;
    if (m_FreePos == m_CurrentPos)
    {
      overflow = true;
    }
    if (m_ImageSequence % n == 0)
    {
      printStatus = true;
    }
    m_ImageMutex.unlock();
    if (overflow)
    {
      overflow = false;
    }
    // print current framerate
    if (printStatus)
    {
      t2 = realTimeClock->GetCurrentStamp() - t1;
      MITK_INFO << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << m_ImageSequence;
      t1 = realTimeClock->GetCurrentStamp();
      printStatus = false;
    }
  }  // end of while loop
}

void ToFCameraMITKPlayerDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
{
  m_ImageMutex.lock();
  // write amplitude image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    amplitudeArray[i] = this->m_AmplitudeDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex.unlock();
}

void ToFCameraMITKPlayerDevice::GetIntensities(float* intensityArray, int& imageSequence)
{
  m_ImageMutex.lock();
  // write intensity image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    intensityArray[i] = this->m_IntensityDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex.unlock();
}

void ToFCameraMITKPlayerDevice::GetDistances(float* distanceArray, int& imageSequence)
{
  m_ImageMutex.lock();
  // write distance image data to float array
  for (int i=0; i<this->m_PixelNumber; i++)
  {
    distanceArray[i] = this->m_DistanceDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex.unlock();
}

void ToFCameraMITKPlayerDevice::GetRgb(unsigned char* rgbArray, int& imageSequence)
{
  m_ImageMutex.lock();
  // write intensity image data to unsigned char array
  for (int i=0; i<this->m_RGBPixelNumber*3; i++)
  {
    rgbArray[i] = this->m_RGBDataBuffer[this->m_CurrentPos][i];
  }
  imageSequence = this->m_ImageSequence;
  m_ImageMutex.unlock();
}

void ToFCameraMITKPlayerDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* /*sourceDataArray*/,
                                             int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
{
  m_ImageMutex.lock();

  //check for empty buffer
  if (this->m_ImageSequence < 0)
  {
    // buffer empty
    MITK_INFO << "Buffer empty!! ";
    capturedImageSequence = this->m_ImageSequence;
    m_ImageMutex.unlock();
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
  m_ImageMutex.unlock();
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
