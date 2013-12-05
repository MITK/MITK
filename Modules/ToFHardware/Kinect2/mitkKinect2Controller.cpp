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
#include "mitkKinect2Controller.h"
#include <Kinect.h>
#include "stdafx.h"

namespace mitk
{
  class Kinect2Controller::Kinect2ControllerPrivate
  {
  public:
    Kinect2ControllerPrivate();
    ~Kinect2ControllerPrivate();

    bool ErrorText(unsigned int error);
    // Depth reader
    IDepthFrameReader* m_pDepthFrameReader;
    IKinectSensor* m_pKinectSensor;
    RGBQUAD* m_pDepthRGBX;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    bool m_UseIR; ///< flag indicating whether IR image is used or not

    unsigned int m_CaptureWidth; ///< image width
    unsigned int m_CaptureHeight; ///< image height

    int m_DepthCaptureWidth;
    int m_DepthCaptureHeight;

    int m_RGBCaptureWidth;
    int m_RGBCaptureHeight;
  };

  Kinect2Controller::Kinect2ControllerPrivate::Kinect2ControllerPrivate():
    m_pKinectSensor(NULL),
    m_pDepthFrameReader(NULL),
    m_pDepthRGBX(NULL),
    m_ConnectionCheck(false),
    m_UseIR(false),
    m_CaptureWidth(640),
    m_CaptureHeight(480),
    m_DepthCaptureWidth(512),
    m_DepthCaptureHeight(424),
    m_RGBCaptureWidth(1920),
    m_RGBCaptureHeight(1080)
  {
  }

  Kinect2Controller::Kinect2ControllerPrivate::~Kinect2ControllerPrivate()
  {
        MITK_INFO << "~Kinect2ControllerPrivate";
  }

  bool Kinect2Controller::Kinect2ControllerPrivate::ErrorText(unsigned int error)
  {
    return true;
  }

  Kinect2Controller::Kinect2Controller(): d(new Kinect2ControllerPrivate)
  {
  }

  Kinect2Controller::~Kinect2Controller()
  {
    MITK_INFO << "~Kinect2Controller";
    delete d;
  }

  bool Kinect2Controller::OpenCameraConnection()
  {
    HRESULT hr;
    if (!d->m_ConnectionCheck)
    {
      hr = GetDefaultKinectSensor(&d->m_pKinectSensor);

      if (FAILED(hr))
      {
        d->m_ConnectionCheck = false;
      }

      if (d->m_pKinectSensor)
      {
        // Initialize the Kinect and get the depth reader
        IDepthFrameSource* pDepthFrameSource = NULL;

        hr = d->m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
          hr = d->m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
        }

        if (SUCCEEDED(hr))
        {
          hr = pDepthFrameSource->OpenReader(&d->m_pDepthFrameReader);
        }

        SafeRelease(pDepthFrameSource);
      }

      if (!d->m_pKinectSensor || FAILED(hr))
      {
        d->m_ConnectionCheck = false;
        MITK_INFO << "No Kinect 2 ready!";
      }
      d->m_ConnectionCheck = true;
    }
    if((d->m_ConnectionCheck) && (d->m_pDepthFrameReader))
    {
      MITK_INFO << "Kinect 2 succesfully connected";
    }
    return d->m_ConnectionCheck;
  }

  bool Kinect2Controller::CloseCameraConnection()
  {
    // done with depth frame reader
    MITK_INFO << "CloseConnection";
    //SafeRelease(d->m_pDepthFrameReader);

    //// close the Kinect Sensor
    //if(d->m_pKinectSensor)
    //{
    //  d->m_pKinectSensor->Close();
    //}

    //SafeRelease(d->m_pKinectSensor);
    return true;
  }

  bool Kinect2Controller::UpdateCamera()
  {
    if (!d->m_pDepthFrameReader)
    {
      return false;
    }
    IDepthFrame* pDepthFrame = NULL;
    HRESULT hr = d->m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

    if (SUCCEEDED(hr))
    {
      INT64 nTime = 0;
      IFrameDescription* pFrameDescription = NULL;
      int nWidth = 0;
      int nHeight = 0;
      USHORT nDepthMinReliableDistance = 0;
      USHORT nDepthMaxReliableDistance = 0;
      UINT nBufferSize = 0;
      UINT16 *pBuffer = NULL;

      hr = pDepthFrame->get_RelativeTime(&nTime);

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
      }

      if (SUCCEEDED(hr))
      {
        hr = pFrameDescription->get_Width(&nWidth);
      }

      if (SUCCEEDED(hr))
      {
        hr = pFrameDescription->get_Height(&nHeight);
      }

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
      }

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
      }

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
      }

      SafeRelease(pFrameDescription);
    }
    SafeRelease(pDepthFrame);

    return true;
  }

  void Kinect2Controller::GetDistances(float* distances)
  {
    if (!d->m_pDepthFrameReader)
    {
      MITK_INFO << "Framereader ist null";
      return;
    }
    IDepthFrame* pDepthFrame = NULL;
    HRESULT hr = d->m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

    if (SUCCEEDED(hr))
    {
      INT64 nTime = 0;
      IFrameDescription* pFrameDescription = NULL;
      int nWidth = 0;
      int nHeight = 0;
      USHORT nDepthMinReliableDistance = 0;
      USHORT nDepthMaxReliableDistance = 0;
      UINT nBufferSize = 0;
      UINT16 *pBuffer = NULL;

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
      }

      if (SUCCEEDED(hr))
      {
        for(int i = 0; i < d->m_DepthCaptureHeight*d->m_DepthCaptureWidth; ++i)
        {
          float depth = static_cast<float>(*pBuffer);
          distances[i] = depth;
          ++pBuffer;
        }
      }
      else
      {
        MITK_ERROR << "err";
      }

    }
  }

  void Kinect2Controller::GetRgb(unsigned char* rgb)
  {
  }

  void Kinect2Controller::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    //this->GetDistances(distances);
  }

  void Kinect2Controller::GetAmplitudes( float* amplitudes )
  {
  }

  void Kinect2Controller::GetIntensities( float* intensities )
  {
  }
  unsigned int Kinect2Controller::GetCaptureWidth() const
  {
    return d->m_CaptureWidth;
  }

  unsigned int Kinect2Controller::GetCaptureHeight() const
  {
    return d->m_CaptureHeight;
  }

  bool Kinect2Controller::GetUseIR() const
  {
    return d->m_UseIR;
  }
  void Kinect2Controller::SetUseIR(bool useIR)
  {
    if (d->m_UseIR!=useIR)
    {
      d->m_UseIR = useIR;
      this->Modified();
    }
  }
}
