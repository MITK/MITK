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
#include "mitkToFCameraPMDRawDataDevice.h"
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"
#include <itksys/SystemTools.hxx>

namespace mitk
{
  ToFCameraPMDRawDataDevice::ToFCameraPMDRawDataDevice() :
    m_SourceDataBuffer(NULL), m_SourceDataArray(NULL), m_ShortSourceData(NULL)
  {
    m_RawDataSource = ThreadedToFRawDataReconstruction::New();

  }

  ToFCameraPMDRawDataDevice::~ToFCameraPMDRawDataDevice()
  {
    this->CleanUpSourceData();
  }

  bool ToFCameraPMDRawDataDevice::ConnectCamera()
  {
    bool ok = false;
    if (m_Controller.IsNotNull())
    {
      ok = m_Controller->OpenCameraConnection();
      if (ok)
      {
        this->m_CaptureWidth = m_Controller->GetCaptureWidth();
        this->m_CaptureHeight = m_Controller->GetCaptureHeight();
        this->m_SourceDataSize = m_Controller->GetSourceDataStructSize();
        this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;

        // allocate buffers
        this->AllocatePixelArrays();
        this->AllocateSourceData();

        m_CameraConnected = true;
      }
    }
    return ok;
  }

  bool ToFCameraPMDRawDataDevice::DisconnectCamera()
  {
    bool ok = false;
    if (m_Controller)
    {
      ok =  m_Controller->CloseCameraConnection();
      if (ok)
      {
        m_CameraConnected = false;
      }
    }
    return ok;
  }

  void ToFCameraPMDRawDataDevice::StartCamera()
  {
    if (m_CameraConnected)
    {
      // get the first image
      this->m_Controller->UpdateCamera();
      
      this->m_ImageMutex->Lock();
      this->m_Controller->GetSourceData(this->m_SourceDataBuffer[this->m_FreePos]);
      this->m_FreePos = (this->m_FreePos+1) % this->m_BufferSize;
      this->m_CurrentPos = (this->m_CurrentPos+1) % this->m_BufferSize;
      this->m_ImageSequence++;
      this->m_ImageMutex->Unlock();

      this->m_CameraActiveMutex->Lock();
      this->m_CameraActive = true;
      this->m_CameraActiveMutex->Unlock();

      this->m_ThreadID = this->m_MultiThreader->SpawnThread(this->Acquire, this);
      // wait a little to make sure that the thread is started
      itksys::SystemTools::Delay(100);
    }
    else
    {
      MITK_INFO<<"Camera not connected";
    }
  }

  void ToFCameraPMDRawDataDevice::StopCamera()
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

  bool ToFCameraPMDRawDataDevice::IsCameraActive()
  {
    m_CameraActiveMutex->Lock();
    bool ok = m_CameraActive;
    m_CameraActiveMutex->Unlock();
    return ok;
  }

  void ToFCameraPMDRawDataDevice::UpdateCamera()
  {
    if (m_Controller)
    {
      m_Controller->UpdateCamera();
    }
  }

  ITK_THREAD_RETURN_TYPE ToFCameraPMDRawDataDevice::Acquire(void* pInfoStruct)
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
    ToFCameraPMDRawDataDevice* toFCameraDevice = (ToFCameraPMDRawDataDevice*)pInfo->UserData;
    if (toFCameraDevice!=NULL)
    {
      mitk::RealTimeClock::Pointer realTimeClock = mitk::RealTimeClock::New();
      double t1, t2;
      t1 = realTimeClock->GetCurrentStamp();
      int n = 100;
      bool overflow = false;
      bool printStatus = false;

      while (toFCameraDevice->IsCameraActive())
      {
        // update the ToF camera
        toFCameraDevice->UpdateCamera();
        // get the source data from the camera and write it at the next free position in the buffer
        // call modified to indicate that cameraDevice was modified
        toFCameraDevice->Modified();

        vtkShortArray* channelData = vtkShortArray::New();
        //toFCameraDevice->m_ImageMutex->Lock();
        toFCameraDevice->m_Controller->GetShortSourceData(toFCameraDevice->m_ShortSourceData);
        toFCameraDevice->GetChannelSourceData( toFCameraDevice->m_ShortSourceData, channelData );
        //toFCameraDevice->m_ImageMutex->Unlock();

        if(!toFCameraDevice->m_RawDataSource->GetInit())
        {
          toFCameraDevice->m_RawDataSource->Initialize(toFCameraDevice->m_CaptureWidth, toFCameraDevice->m_CaptureHeight, 
            toFCameraDevice->m_Controller->GetModulationFrequency(), toFCameraDevice->GetChannelSize());
        }
        toFCameraDevice->m_RawDataSource->SetChannelData(channelData);
        toFCameraDevice->m_RawDataSource->Update();
        toFCameraDevice->m_ImageMutex->Lock();
        toFCameraDevice->m_Controller->GetSourceData(toFCameraDevice->m_SourceDataArray);
        toFCameraDevice->m_RawDataSource->GetAllData(toFCameraDevice->m_DistanceArray, toFCameraDevice->m_AmplitudeArray, toFCameraDevice->m_IntensityArray);
        toFCameraDevice->m_ImageMutex->Unlock();
        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         TODO Buffer Handling currently only works for buffer size 1
         !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


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
        channelData->Delete();

        if (overflow)
        {
          overflow = false;
        }
        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         END TODO Buffer Handling currently only works for buffer size 1
         !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

        // print current framerate
        if (printStatus)
        {
          t2 = realTimeClock->GetCurrentStamp() - t1;
          //MITK_INFO << "t2: " << t2 <<" Time (s) for 1 image: " << (t2/1000) / n << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << toFCameraDevice->m_ImageSequence;
          MITK_INFO << " Framerate (fps): " << n / (t2/1000) << " Sequence: " << toFCameraDevice->m_ImageSequence;
          t1 = realTimeClock->GetCurrentStamp();
          printStatus = false;
        }

      }  // end of while loop
    }
    return ITK_THREAD_RETURN_VALUE;
  }

  //    TODO: Buffer size currently set to 1. Once Buffer handling is working correctly, method may be reactivated
  //  void ToFCameraPMDDevice::ResetBuffer(int bufferSize)
  //  {
  //    this->m_BufferSize = bufferSize;
  //    this->m_CurrentPos = -1;
  //    this->m_FreePos = 0;
  //  }

  void ToFCameraPMDRawDataDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      memcpy(amplitudeArray, this->m_AmplitudeArray, this->m_PixelNumber*sizeof(float));
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataDevice::GetIntensities(float* intensityArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      memcpy(intensityArray, this->m_IntensityArray, this->m_PixelNumber*sizeof(float));
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataDevice::GetDistances(float* distanceArray, int& imageSequence)
  {
    if (m_CameraActive)
    {
      memcpy(distanceArray, this->m_DistanceArray, this->m_PixelNumber*sizeof(float));
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
                                        int requiredImageSequence, int& capturedImageSequence)
  {
    if (m_CameraActive)
    {
      // 1) copy the image buffer
      // 2) convert the distance values from m to mm
      // 3) Flip around y- axis (vertical axis)

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
        //MITK_INFO << "Required image not found! Required: " << requiredImageSequence << " delivered/current: " << this->m_ImageSequence;
      }
      else if (requiredImageSequence <= this->m_ImageSequence - this->m_BufferSize)
      {
        capturedImageSequence = (this->m_ImageSequence - this->m_BufferSize) + 1;
        pos = (this->m_CurrentPos + 1) % this->m_BufferSize;
        //MITK_INFO << "Out of buffer! Required: " << requiredImageSequence << " delivered: " << capturedImageSequence << " current: " << this->m_ImageSequence;
      }
      else // (requiredImageSequence > this->m_ImageSequence - this->m_BufferSize) && (requiredImageSequence <= this->m_ImageSequence)

      {
        capturedImageSequence = requiredImageSequence;
        pos = (this->m_CurrentPos + (10-(this->m_ImageSequence - requiredImageSequence))) % this->m_BufferSize;
      }

      memcpy(distanceArray, this->m_DistanceArray, this->m_PixelNumber*sizeof(float));
      memcpy(amplitudeArray, this->m_AmplitudeArray, this->m_PixelNumber*sizeof(float));
      memcpy(intensityArray, this->m_IntensityArray, this->m_PixelNumber*sizeof(float));
      memcpy(sourceDataArray, this->m_SourceDataBuffer[this->m_CurrentPos], this->m_SourceDataSize);
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
  }

  void ToFCameraPMDRawDataDevice::XYAxisFlipImage( float* imageData, float* &flippedData, int xAxis, int yAxis, int dimension )
  {
    int captureWidth = this->GetCaptureWidth();
    int captureHeight = this->GetCaptureHeight();

    //
    //flips image around x- axis (horizontal axis)
    //

    if(xAxis == 1 && yAxis != 1)
    {
      for (int i=0; i<captureHeight; i++)
      {
        for (int j=0; j<captureWidth; j++)
        {
          for (int k=0; k<dimension; k++)
          {
            flippedData[i*captureWidth*dimension+j*dimension+k] = imageData[((captureHeight-1-i)*captureWidth*dimension)+((j)*dimension+k)];
          }
        }
      }
    }
    //
    // flips image around y- axis (vertical axis)
    //

    if(xAxis!=1 && yAxis == 1)
    {
      for (int i=0; i<captureHeight; i++)
      {
        for (int j=0; j<captureWidth; j++)
        {
          for (int k=0; k<dimension; k++)
          {
            flippedData[i*captureWidth*dimension+j*dimension+k] = imageData[(i+1)*captureWidth*dimension-1-j*dimension-k];
          }
        }
      }
    }

    //
    //flips image around x- and y- axis
    //

    else if(xAxis == 1 && yAxis == 1)
    {
      for (int i=0; i<captureHeight; i++)
      {
        for (int j=0; j<captureWidth; j++)
        {
          for (int k=0; k<dimension; k++)
          {
            flippedData[i*captureWidth*dimension+j*dimension+k] = imageData[((captureHeight-i)*(captureWidth)*dimension-1)-(j*dimension-k)];
          }
        }
      }
    }
  }

  ToFCameraPMDController::Pointer ToFCameraPMDRawDataDevice::GetController()
  {
    return this->m_Controller;
  }

  void ToFCameraPMDRawDataDevice::SetProperty( const char *propertyKey, BaseProperty* propertyValue )
  {
    ToFCameraDevice::SetProperty(propertyKey,propertyValue);
    this->m_PropertyList->SetProperty(propertyKey, propertyValue);
    if (strcmp(propertyKey, "ModulationFrequency") == 0)
    {
      int modulationFrequency = 0;
      GetIntProperty(propertyValue, modulationFrequency);
      m_Controller->SetModulationFrequency(modulationFrequency);
    }
    else if (strcmp(propertyKey, "IntegrationTime") == 0)
    {
      int integrationTime = 0;
      GetIntProperty(propertyValue, integrationTime);
      m_Controller->SetIntegrationTime(integrationTime);
    }
  }

  void ToFCameraPMDRawDataDevice::CleanupPixelArrays()
  {
    if (m_IntensityArray)
    {
      delete [] m_IntensityArray;
    }
    if (m_DistanceArray)
    {
      delete [] m_DistanceArray;
    }
    if (m_AmplitudeArray)
    {
      delete [] m_AmplitudeArray;
    }
    if (m_ShortSourceData)
    {
      delete [] m_ShortSourceData;
    }
  }

  void ToFCameraPMDRawDataDevice::AllocatePixelArrays()
  {
    // free memory if it was already allocated
    CleanupPixelArrays();
    // allocate buffer
    this->m_IntensityArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_IntensityArray[i]=0.0;}
    this->m_DistanceArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_DistanceArray[i]=0.0;}
    this->m_AmplitudeArray = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++) {this->m_AmplitudeArray[i]=0.0;}
    this->m_ShortSourceData = new short[this->m_SourceDataSize];
    for(int i=0; i<this->m_SourceDataSize; i++) {this->m_ShortSourceData[i]=0.0;}

  }

  void ToFCameraPMDRawDataDevice::AllocateSourceData()
  {
    // clean up if array and data have already been allocated
    CleanUpSourceData();
    // (re-) allocate memory
    this->m_SourceDataArray = new char[this->m_SourceDataSize];
    for(int i=0; i<this->m_SourceDataSize; i++) {this->m_SourceDataArray[i]=0;}

    this->m_SourceDataBuffer = new char*[this->m_MaxBufferSize];
    for(int i=0; i<this->m_MaxBufferSize; i++)
    {
      this->m_SourceDataBuffer[i] = new char[this->m_SourceDataSize];
    }
  }

  void ToFCameraPMDRawDataDevice::CleanUpSourceData()
  {
    if (m_SourceDataArray)
    {
      delete[] m_SourceDataArray;
    }
    if (m_SourceDataBuffer)
    {
      for(int i=0; i<this->m_MaxBufferSize; i++)
      {
        delete[] this->m_SourceDataBuffer[i];
      }
      delete[] this->m_SourceDataBuffer;
    }
  }
}
