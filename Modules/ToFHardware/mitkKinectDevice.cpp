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
#include "mitkRealTimeClock.h"

#include "itkMultiThreader.h"
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

  //----------------------------J´s Implementation of Device Names?--------------------------------------------
  /*!
  \brief Defining the Device´s Name+Suffix, here for the Kinect.
  */
  std::string GetDeviceName()
  {
    //int kinectnumber = 1;
    std::stringstream out;
    out << "Microsoft Kinect Device ";
    //out << kinectnumber;
    //kinectnumber= kinectnumber++;
    return out.str();
  }
  //----------------------------------------------------------------------------------------------------------------------


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
        delete [] m_IntensityArray;
        delete [] m_DistanceArray;
        delete [] m_AmplitudeArray;

        for(int i=0; i<this->m_MaxBufferSize; i++)
        {
          delete[] this->m_DistanceDataBuffer[i];
          delete[] this->m_AmplitudeDataBuffer[i];
          delete[] this->m_IntensityDataBuffer[i];
          delete[] this->m_RGBDataBuffer[i];
        }
        delete[] this->m_DistanceDataBuffer;
        delete[] this->m_AmplitudeDataBuffer;
        delete[] this->m_IntensityDataBuffer;
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

        /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        TODO Buffer Handling currently only works for buffer size 1
        !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
        //toFCameraDevice->m_ImageSequence++;
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
  //  void KinectDevice::ResetBuffer(int bufferSize)
  //  {
  //    this->m_BufferSize = bufferSize;
  //    this->m_CurrentPos = -1;
  //    this->m_FreePos = 0;
  //  }

  void KinectDevice::GetAmplitudes(float* amplitudeArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      // 1) copy the image buffer
      // 2) Flip around y- axis (vertical axis)

      /*
      this->m_Controller->GetAmplitudes(this->m_SourceDataBuffer[this->m_CurrentPos], this->m_AmplitudeArray);
      for (int i=0; i<this->m_CaptureHeight; i++)
      {
      for (int j=0; j<this->m_CaptureWidth; j++)
      {
      amplitudeArray[i*this->m_CaptureWidth+j] = this->m_AmplitudeArray[(i+1)*this->m_CaptureWidth-1-j];
      }
      }
      */
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

  void KinectDevice::GetIntensities(float* intensityArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      // 1) copy the image buffer
      // 2) Flip around y- axis (vertical axis)

      /*
      this->m_Controller->GetIntensities(this->m_SourceDataBuffer[this->m_CurrentPos], this->m_IntensityArray);
      for (int i=0; i<this->m_CaptureHeight; i++)
      {
      for (int j=0; j<this->m_CaptureWidth; j++)
      {
      intensityArray[i*this->m_CaptureWidth+j] = this->m_IntensityArray[(i+1)*this->m_CaptureWidth-1-j];
      }
      }
      */
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

  void KinectDevice::GetDistances(float* distanceArray, int& imageSequence)
  {
    m_ImageMutex->Lock();
    if (m_CameraActive)
    {
      // 1) copy the image buffer
      // 2) convert the distance values from m to mm
      // 3) Flip around y- axis (vertical axis)

      /*
      this->m_Controller->GetDistances(this->m_SourceDataBuffer[this->m_CurrentPos], this->m_DistanceArray);
      for (int i=0; i<this->m_CaptureHeight; i++)
      {
      for (int j=0; j<this->m_CaptureWidth; j++)
      {
      distanceArray[i*this->m_CaptureWidth+j] = 1000 * this->m_DistanceArray[(i+1)*this->m_CaptureWidth-1-j];
      }
      }
      */
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        distanceArray[i] = this->m_DistanceDataBuffer[this->m_CurrentPos][i]; // * 1000
      }
      imageSequence = this->m_ImageSequence;
    }
    else
    {
      MITK_WARN("ToF") << "Warning: Data can only be acquired if camera is active.";
    }
    m_ImageMutex->Unlock();
  }

  void KinectDevice::GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
    int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray)
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

      // write image data to float arrays
      for (int i=0; i<this->m_PixelNumber; i++)
      {
        distanceArray[i] = this->m_DistanceDataBuffer[pos][i];
        amplitudeArray[i] = this->m_AmplitudeDataBuffer[pos][i];
        intensityArray[i] = this->m_IntensityDataBuffer[pos][i];
        rgbDataArray[i*3] = this->m_RGBDataBuffer[pos][i*3];
        rgbDataArray[i*3+1] = this->m_RGBDataBuffer[pos][i*3+1];
        rgbDataArray[i*3+2] = this->m_RGBDataBuffer[pos][i*3+2];
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
