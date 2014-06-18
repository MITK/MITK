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
#include "mitkKinectDevice.h"
#include <mitkRealTimeClock.h>

#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>


namespace mitk
{

KinectDevice::KinectDevice()
{
  m_Controller = mitk::KinectController::New();
}

KinectDevice::~KinectDevice()
{
}

bool KinectDevice::OnConnectCamera()
{
  bool ok = false;
  if (m_Controller)
  {
    ok = m_Controller->OpenCameraConnection();
    if (ok)
    {
      this->m_CaptureWidth = m_Controller->GetCaptureWidth();
      this->m_CaptureHeight = m_Controller->GetCaptureHeight();
      this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;

      this->m_RGBImageWidth = m_CaptureWidth;
      this->m_RGBImageHeight = m_CaptureHeight;
      this->m_RGBPixelNumber = this->m_RGBImageWidth * this->m_RGBImageHeight;

      this->SetBoolProperty("HasAmplitudeImage", m_Controller->GetUseIR());
      this->SetBoolProperty("HasRGBImage", !m_Controller->GetUseIR());

      // allocate buffer
      this->m_DistanceArray = new float[this->m_PixelNumber];
      for(int i=0; i<this->m_PixelNumber; i++) {this->m_DistanceArray[i]=0.0;}
      this->m_AmplitudeArray = new float[this->m_PixelNumber];
      for(int i=0; i<this->m_PixelNumber; i++) {this->m_AmplitudeArray[i]=0.0;}

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
      this->m_RGBDataBuffer = new unsigned char*[this->m_MaxBufferSize];
      for (int i=0; i<this->m_MaxBufferSize; i++)
      {
        this->m_RGBDataBuffer[i] = new unsigned char[this->m_PixelNumber*3];
      }

      m_CameraConnected = true;
    }
  }
  return ok;
}

bool KinectDevice::DisconnectCamera()
{
  bool ok = false;
  if (m_Controller)
  {
    ok =  m_Controller->CloseCameraConnection();

    // clean-up only if camera was connected
    if (m_CameraConnected)
    {
      delete [] m_DistanceArray;
      delete [] m_AmplitudeArray;

      for(int i=0; i<this->m_MaxBufferSize; i++)
      {
        delete[] this->m_DistanceDataBuffer[i];
        delete[] this->m_AmplitudeDataBuffer[i];
        delete[] this->m_RGBDataBuffer[i];
      }
      delete[] this->m_DistanceDataBuffer;
      delete[] this->m_AmplitudeDataBuffer;
      delete[] this->m_RGBDataBuffer;

      m_CameraConnected = false;
    }

  }
  return ok;
}

void KinectDevice::StartCamera()
{
  if (m_CameraConnected)
  {
    // get the first image
    this->m_Controller->UpdateCamera();
    this->m_ImageMutex->Lock();
    this->m_Controller->GetAllData(this->m_DistanceDataBuffer[this->m_FreePos],this->m_AmplitudeDataBuffer[this->m_FreePos],this->m_RGBDataBuffer[this->m_FreePos]);
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

void KinectDevice::StopCamera()
{
  m_CameraActiveMutex->Lock();
  m_CameraActive = false;
  m_CameraActiveMutex->Unlock();
  itksys::SystemTools::Delay(100);
  if (m_MultiThreader.IsNotNull())
  {
    m_MultiThreader->TerminateThread(m_ThreadID);
  }
  // wait a little to make sure that the thread is terminated
  itksys::SystemTools::Delay(10);
}

bool KinectDevice::IsCameraActive()
{
  m_CameraActiveMutex->Lock();
  bool ok = m_CameraActive;
  m_CameraActiveMutex->Unlock();
  return ok;
}

void KinectDevice::UpdateCamera()
{
  if (m_Controller)
  {
    m_Controller->UpdateCamera();
  }
}

ITK_THREAD_RETURN_TYPE KinectDevice::Acquire(void* pInfoStruct)
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
  KinectDevice* toFCameraDevice = (KinectDevice*)pInfo->UserData;
  if (toFCameraDevice!=NULL)
  {
    mitk::RealTimeClock::Pointer realTimeClock;
    realTimeClock = mitk::RealTimeClock::New();
    double t1, t2;
    t1 = realTimeClock->GetCurrentStamp();
    int n = 100;
    bool overflow = false;
    bool printStatus = false;
    while (toFCameraDevice->IsCameraActive())
    {
      // update the ToF camera
      toFCameraDevice->UpdateCamera();
      // get the image data from the camera and write it at the next free position in the buffer
      toFCameraDevice->m_ImageMutex->Lock();
      toFCameraDevice->m_Controller->GetAllData(toFCameraDevice->m_DistanceDataBuffer[toFCameraDevice->m_FreePos],toFCameraDevice->m_AmplitudeDataBuffer[toFCameraDevice->m_FreePos],toFCameraDevice->m_RGBDataBuffer[toFCameraDevice->m_FreePos]);
      toFCameraDevice->m_ImageMutex->Unlock();

      // call modified to indicate that cameraDevice was modified
      toFCameraDevice->Modified();
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

void KinectDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  if (m_CameraActive)
  {
      // copy the image buffer
    imageSequence = this->m_ImageSequence;
  }
  else
  {
    MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
  }
  m_ImageMutex->Unlock();
}

void KinectDevice::GetIntensities(float *, int &)
{
}

void KinectDevice::GetDistances(float* distanceArray, int& imageSequence)
{
  m_ImageMutex->Lock();
  if (m_CameraActive)
  {
    imageSequence = this->m_ImageSequence;
  }
  else
  {
    MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
  }
  m_ImageMutex->Unlock();
}

void KinectDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float*, char*,
                                int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
{
  if (m_CameraActive)
  {
    // check for empty buffer
    if (this->m_ImageSequence < 0)
    {
      // buffer empty
      MITK_INFO << "Buffer empty!! ";
      capturedImageSequence = this->m_ImageSequence;
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

    memcpy( distanceArray, this->m_DistanceDataBuffer[pos], m_PixelNumber*sizeof(float));
    if(m_Controller->GetUseIR())
    {
      memcpy( amplitudeArray, this->m_AmplitudeDataBuffer[pos], m_PixelNumber*sizeof(float));
    }
    else
    {
      memcpy( rgbDataArray, this->m_RGBDataBuffer[pos], m_RGBPixelNumber*3*sizeof(unsigned char));
    }
  }
  else
  {
    MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
  }
}

KinectController::Pointer KinectDevice::GetController()
{
  return this->m_Controller;
}

void KinectDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
{
  ToFCameraDevice::SetProperty(propertyKey,propertyValue);
  this->m_PropertyList->SetProperty(propertyKey, propertyValue);
  if (strcmp(propertyKey, "RGB") == 0)
  {
    bool rgb = false;
    GetBoolProperty(propertyKey, rgb);
    m_Controller->SetUseIR(!rgb);
  }
  else if (strcmp(propertyKey, "IR") == 0)
  {
    bool ir = false;
    GetBoolProperty(propertyKey, ir);
    m_Controller->SetUseIR(ir);
  }
}

int KinectDevice::GetRGBCaptureWidth()
{
  return this->GetCaptureWidth();
}

int KinectDevice::GetRGBCaptureHeight()
{
  return this->GetCaptureHeight();
}
}
