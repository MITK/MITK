/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkToFCameraMITKPlayerController.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"
#include <itksys/SystemTools.hxx>

namespace mitk
{
  ToFCameraMITKPlayerDevice::ToFCameraMITKPlayerDevice() : 
  m_DistanceDataBuffer(NULL), m_AmplitudeDataBuffer(NULL), m_IntensityDataBuffer(NULL)
  {
    m_Controller = ToFCameraMITKPlayerController::New();
  }

  ToFCameraMITKPlayerDevice::~ToFCameraMITKPlayerDevice()
  {
    DisconnectCamera();
    CleanUpDataBuffers();
  }

  bool ToFCameraMITKPlayerDevice::ConnectCamera()
  {
    bool ok = m_Controller->OpenCameraConnection();
    if (ok)
    {
      this->m_CaptureWidth = m_Controller->GetCaptureWidth();
      this->m_CaptureHeight = m_Controller->GetCaptureHeight();
      this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;

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

  void ToFCameraMITKPlayerDevice::StopCamera()
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
    itksys::SystemTools::Delay(100);
  }

  bool ToFCameraMITKPlayerDevice::IsCameraActive()
  {
    m_CameraActiveMutex->Lock();
    bool ok = m_CameraActive;
    m_CameraActiveMutex->Unlock();
    return ok;
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
        toFCameraDevice->Modified();
        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         TODO Buffer Handling currently only works for buffer size 1
         !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        toFCameraDevice->m_ImageMutex->Lock();
        toFCameraDevice->m_FreePos = (toFCameraDevice->m_FreePos+1) % toFCameraDevice->m_BufferSize;
        toFCameraDevice->m_CurrentPos = (toFCameraDevice->m_CurrentPos+1) % toFCameraDevice->m_BufferSize;
        toFCameraDevice->m_ImageSequence++;
        if (toFCameraDevice->m_FreePos == toFCameraDevice->m_CurrentPos)
        {
          // buffer overflow
          //MITK_INFO << "Buffer overflow!! ";
          //toFCameraDevice->m_CurrentPos = (toFCameraDevice->m_CurrentPos+1) % toFCameraDevice->m_BufferSize;
          //toFCameraDevice->m_ImageSequence++;
          overflow = true;
        }
        if (toFCameraDevice->m_ImageSequence % n == 0)
        {
          printStatus = true;
        }
        toFCameraDevice->m_ImageMutex->Unlock();
        if (overflow)
        {
          //itksys::SystemTools::Delay(10);
          overflow = false;
        }
        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         END TODO Buffer Handling currently only works for buffer size 1
         !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

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

//    TODO: Buffer size currently set to 1. Once Buffer handling is working correctly, method may be reactivated
//  void ToFCameraMITKPlayerDevice::ResetBuffer(int bufferSize)
//  {
//    this->m_BufferSize = bufferSize;
//    this->m_CurrentPos = -1;
//    this->m_FreePos = 0;
//  }

  void ToFCameraMITKPlayerDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    /*!!!!!!!!!!!!!!!!!!!!!!
      TODO Buffer handling???
    !!!!!!!!!!!!!!!!!!!!!!!!*/
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
    /*!!!!!!!!!!!!!!!!!!!!!!
      TODO Buffer handling???
    !!!!!!!!!!!!!!!!!!!!!!!!*/
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
    /*!!!!!!!!!!!!!!!!!!!!!!
      TODO Buffer handling???
    !!!!!!!!!!!!!!!!!!!!!!!!*/
    // write distance image data to float array
    for (int i=0; i<this->m_PixelNumber; i++)
    {
      distanceArray[i] = this->m_DistanceDataBuffer[this->m_CurrentPos][i];
    }
    imageSequence = this->m_ImageSequence;
    m_ImageMutex->Unlock();
  }

  void ToFCameraMITKPlayerDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* /*sourceDataArray*/,
    int requiredImageSequence, int& capturedImageSequence)
  {
    /*!!!!!!!!!!!!!!!!!!!!!!
      TODO Document this method!
    !!!!!!!!!!!!!!!!!!!!!!!!*/
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

    if(this->m_DistanceDataBuffer&&this->m_AmplitudeDataBuffer&&this->m_IntensityDataBuffer)
    {
      // write image data to float arrays
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        distanceArray[i] = this->m_DistanceDataBuffer[pos][i];
        amplitudeArray[i] = this->m_AmplitudeDataBuffer[pos][i];
        intensityArray[i] = this->m_IntensityDataBuffer[pos][i];
      }
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
    GetStringProperty(propertyValue, strValue);
    if (strcmp(propertyKey, "DistanceImageFileName") == 0)
    {
      myController->SetDistanceImageFileName(strValue);
    }
    else if (strcmp(propertyKey, "AmplitudeImageFileName") == 0)
    {
      myController->SetAmplitudeImageFileName(strValue);
    }
    else if (strcmp(propertyKey, "IntensityImageFileName") == 0)
    {
      myController->SetIntensityImageFileName(strValue);
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
  }
}
