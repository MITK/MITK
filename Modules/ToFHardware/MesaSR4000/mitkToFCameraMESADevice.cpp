/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFCameraMESADevice.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"
#include <itksys/SystemTools.hxx>

namespace mitk
{
  ToFCameraMESADevice::ToFCameraMESADevice()
  {
  }

  ToFCameraMESADevice::~ToFCameraMESADevice()
  {
  }

  bool ToFCameraMESADevice::OnConnectCamera()
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

        // allocate buffer
        this->m_IntensityArray = new float[this->m_PixelNumber];
        for(int i=0; i<this->m_PixelNumber; i++) {this->m_IntensityArray[i]=0.0;}
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
        this->m_IntensityDataBuffer = new float*[this->m_MaxBufferSize];
        for(int i=0; i<this->m_MaxBufferSize; i++)
        {
          this->m_IntensityDataBuffer[i] = new float[this->m_PixelNumber];
        }

        m_CameraConnected = true;
      }
    }
    return ok;
  }

  bool ToFCameraMESADevice::DisconnectCamera()
  {
    bool ok = false;
    if (m_Controller)
    {
      ok =  m_Controller->CloseCameraConnection();

      // clean-up only if camera was connected
      if (m_CameraConnected)
      {
        delete [] m_IntensityArray;
        delete [] m_DistanceArray;
        delete [] m_AmplitudeArray;

        for(int i=0; i<this->m_MaxBufferSize; i++)
        {
          delete[] this->m_DistanceDataBuffer[i];
        }
        delete[] this->m_DistanceDataBuffer;
        for(int i=0; i<this->m_MaxBufferSize; i++)
        {
          delete[] this->m_AmplitudeDataBuffer[i];
        }
        delete[] this->m_AmplitudeDataBuffer;
        for(int i=0; i<this->m_MaxBufferSize; i++)
        {
          delete[] this->m_IntensityDataBuffer[i];
        }
        delete[] this->m_IntensityDataBuffer;

        m_CameraConnected = false;
      }

    }
    return ok;
  }

  void ToFCameraMESADevice::StartCamera()
  {
    if (m_CameraConnected)
    {
      // get the first image
      this->m_Controller->UpdateCamera();
      this->m_ImageMutex->Lock();
      this->m_Controller->GetDistances(this->m_DistanceDataBuffer[this->m_FreePos]);
      this->m_Controller->GetAmplitudes(this->m_AmplitudeDataBuffer[this->m_FreePos]);
      this->m_Controller->GetIntensities(this->m_IntensityDataBuffer[this->m_FreePos]);
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

  void ToFCameraMESADevice::StopCamera()
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

  bool ToFCameraMESADevice::IsCameraActive()
  {
    m_CameraActiveMutex->Lock();
    bool ok = m_CameraActive;
    m_CameraActiveMutex->Unlock();
    return ok;
  }

  void ToFCameraMESADevice::UpdateCamera()
  {
    if (m_Controller)
    {
      m_Controller->UpdateCamera();
    }
  }

  ITK_THREAD_RETURN_TYPE ToFCameraMESADevice::Acquire(void* pInfoStruct)
  {
    /* extract this pointer from Thread Info structure */
    struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
    if (pInfo == nullptr)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    if (pInfo->UserData == nullptr)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    ToFCameraMESADevice* toFCameraDevice = (ToFCameraMESADevice*)pInfo->UserData;
    if (toFCameraDevice!=nullptr)
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
        toFCameraDevice->m_Controller->GetDistances(toFCameraDevice->m_DistanceDataBuffer[toFCameraDevice->m_FreePos]);
        toFCameraDevice->m_Controller->GetAmplitudes(toFCameraDevice->m_AmplitudeDataBuffer[toFCameraDevice->m_FreePos]);
        toFCameraDevice->m_Controller->GetIntensities(toFCameraDevice->m_IntensityDataBuffer[toFCameraDevice->m_FreePos]);
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

  void ToFCameraMESADevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      // copy the image buffer
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        amplitudeArray[i] = this->m_AmplitudeDataBuffer[this->m_CurrentPos][i];
      }
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
    m_ImageMutex->Unlock();
  }

  void ToFCameraMESADevice::GetIntensities(float* intensityArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        intensityArray[i] = this->m_IntensityDataBuffer[this->m_CurrentPos][i];
      }
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
    m_ImageMutex->Unlock();
  }

  void ToFCameraMESADevice::GetDistances(float* distanceArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        distanceArray[i] = this->m_DistanceDataBuffer[this->m_CurrentPos][i];
      }
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
    m_ImageMutex->Unlock();
  }

  void ToFCameraMESADevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
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

      // write image data to float arrays
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        distanceArray[i] = this->m_DistanceDataBuffer[pos][i] /* * 1000 */;
        amplitudeArray[i] = this->m_AmplitudeDataBuffer[pos][i];
        intensityArray[i] = this->m_IntensityDataBuffer[pos][i];
      }
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  ToFCameraMESAController::Pointer ToFCameraMESADevice::GetController()
  {
    return this->m_Controller;
  }

  void ToFCameraMESADevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);
    if (strcmp(propertyKey, "ModulationFrequency") == 0)
    {
      int modulationFrequency = 0;
      GetIntProperty(propertyKey, modulationFrequency);
      m_Controller->SetModulationFrequency(modulationFrequency);
    }
    else if (strcmp(propertyKey, "IntegrationTime") == 0)
    {
      int integrationTime = 0;
      GetIntProperty(propertyKey, integrationTime);
      m_Controller->SetIntegrationTime(integrationTime);
    }
  }
}
