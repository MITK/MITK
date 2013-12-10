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
#include <mitkToFDebugHelper.h>

//Taken from official Microsoft SDK samples. Should never be public or part of the class,
//because it is just for cleaning up.
// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease != NULL)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}

namespace mitk
{
  class Kinect2Controller::Kinect2ControllerPrivate
  {
  public:
    Kinect2ControllerPrivate();
    ~Kinect2ControllerPrivate();

    bool ErrorText(unsigned int error);
    // Depth reader
    IKinectSensor* m_pKinectSensor;
    IDepthFrameReader* m_pDepthFrameReader;
    IColorFrameReader* m_pColorFrameReader;
    IDepthFrame* m_pDepthFrame;
    IColorFrame* m_pColorFrame;
    RGBQUAD* m_pColorRGBX;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    bool m_UseIR; ///< flag indicating whether IR image is used or not

    int m_DepthCaptureWidth;
    int m_DepthCaptureHeight;

    int m_RGBCaptureWidth;
    int m_RGBCaptureHeight;
    float* m_Distances;
    unsigned char* m_Colors;
    unsigned int m_BufferSize;
  };

  Kinect2Controller::Kinect2ControllerPrivate::Kinect2ControllerPrivate():
    m_pKinectSensor(NULL),
    m_pDepthFrameReader(NULL),
    m_pColorFrameReader(NULL),
    m_pDepthFrame(NULL),
    m_pColorFrame(NULL),
    m_pColorRGBX(NULL),
    m_ConnectionCheck(false),
    m_UseIR(false),
    m_DepthCaptureWidth(512),
    m_DepthCaptureHeight(424),
    m_RGBCaptureWidth(1920),
    m_RGBCaptureHeight(1080),
    m_Distances(NULL),
    m_Colors(NULL),
    m_BufferSize(1920*1080*3)
  {
    // create heap storage for color pixel data in RGBX format
    m_pColorRGBX = new RGBQUAD[m_RGBCaptureWidth * m_RGBCaptureHeight];
    m_Distances = new float[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_Colors = new unsigned char[m_BufferSize];
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
        // Initialize the Kinect and get the depth and color reader
        IDepthFrameSource* pDepthFrameSource = NULL;
        IColorFrameSource* pColorFrameSource = NULL;

        hr = d->m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
          hr = d->m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
        }

        if (SUCCEEDED(hr))
        {
          hr = pDepthFrameSource->OpenReader(&d->m_pDepthFrameReader);
        }

        if (SUCCEEDED(hr))
        {
          hr = d->m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
        }

        if (SUCCEEDED(hr))
        {
          hr = pColorFrameSource->OpenReader(&d->m_pColorFrameReader);
        }

        SafeRelease(pColorFrameSource);
        SafeRelease(pDepthFrameSource);
      }

      if (!d->m_pKinectSensor || FAILED(hr))
      {
        d->m_ConnectionCheck = false;
        MITK_WARN << "No Kinect 2 ready!";
      }
      d->m_ConnectionCheck = true;
    }
    if((d->m_ConnectionCheck) && (d->m_pDepthFrameReader) && (d->m_pColorFrameReader))
    {
      MITK_INFO << "Kinect 2 succesfully connected";
    }
    return d->m_ConnectionCheck;
  }

  bool Kinect2Controller::CloseCameraConnection()
  {
    // done with depth frame reader
    MITK_INFO << "CloseConnection";
    SafeRelease(d->m_pDepthFrameReader);

    // close the Kinect Sensor
    if(d->m_pKinectSensor)
    {
      d->m_pKinectSensor->Close();
    }

    SafeRelease(d->m_pKinectSensor);
    d->m_ConnectionCheck = false;
    return true;
  }

  bool Kinect2Controller::UpdateCamera()
  {
    MITK_INFO << "UpdateCamera";
    if (!d->m_pDepthFrameReader)
    {
      MITK_ERROR << "UpdateCamera: No DepthFrameReader";
      return false;
    }
    if (!d->m_pColorFrameReader)
    {
      MITK_ERROR << "UpdateCamera: No ColorFrameReader";
      return false;
    }
    d->m_pDepthFrame = NULL;

    HRESULT hr = d->m_pDepthFrameReader->AcquireLatestFrame(&d->m_pDepthFrame);

    if (SUCCEEDED(hr))
    {
      MITK_INFO << "AcquireLatestFrame";
      INT64 nTime = 0;
      IFrameDescription* pFrameDescription = NULL;
      int nWidth = 0;
      int nHeight = 0;
      USHORT nDepthMinReliableDistance = 0;
      USHORT nDepthMaxReliableDistance = 0;
      UINT nBufferSize = 0;
      UINT16 *pBuffer = NULL;

      hr = d->m_pDepthFrame->get_RelativeTime(&nTime);

      if (SUCCEEDED(hr))
      {
        hr = d->m_pDepthFrame->get_FrameDescription(&pFrameDescription);
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
        hr = d->m_pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
      }

      if (SUCCEEDED(hr))
      {
        hr = d->m_pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
      }

      hr = d->m_pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

      if (SUCCEEDED(hr))
      {
        MITK_INFO << "AccessUnderlyingBuffer";
        for(int i = 0; i < d->m_DepthCaptureHeight*d->m_DepthCaptureWidth; ++i)
        {
          float depth = static_cast<float>(*pBuffer);
          d->m_Distances[i] = depth;
          ++pBuffer;
        }
      }
      else
      {
        MITK_ERROR << "AccessUnderlyingBuffer";
      }
      SafeRelease(pFrameDescription);

      //color
      hr = d->m_pColorFrameReader->AcquireLatestFrame(&d->m_pColorFrame);
      if (SUCCEEDED(hr))
      {
        INT64 nTime = 0;
        IFrameDescription* pColorFrameDescription = NULL;
        int nColorWidth = 0;
        int nColorHeight = 0;
        ColorImageFormat imageFormat = ColorImageFormat_None;
        UINT nBufferSize = 0;
        RGBQUAD *pBuffer = NULL;

        hr = d->m_pColorFrame->get_RelativeTime(&nTime);

        if (SUCCEEDED(hr))
        {
          hr = d->m_pColorFrame->get_FrameDescription(&pColorFrameDescription);
        }

        if (SUCCEEDED(hr))
        {
          hr = pColorFrameDescription->get_Width(&nColorWidth);
        }

        if (SUCCEEDED(hr))
        {
          hr = pColorFrameDescription->get_Height(&nColorHeight);
        }

        if (SUCCEEDED(hr))
        {
          hr = d->m_pColorFrame->get_RawColorImageFormat(&imageFormat);
          if (imageFormat == ColorImageFormat_Bgra)
          {
            MITK_INFO << "Format ist ColorImageFormat_Bgra";
          }
          if (d->m_pColorRGBX)
          {
            pBuffer = d->m_pColorRGBX;
            nBufferSize = d->m_RGBCaptureWidth * d->m_RGBCaptureHeight * sizeof(RGBQUAD);
            hr = d->m_pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
            for(int i = 0; i < d->m_BufferSize; i+=3)
            {

              d->m_Colors[i+0] = pBuffer->rgbRed;
              d->m_Colors[i+1] = pBuffer->rgbGreen;
              d->m_Colors[i+2] = pBuffer->rgbBlue;
              ++pBuffer;
            }
          }
        }
        SafeRelease(pColorFrameDescription);
      }
      else
      {
        MITK_ERROR << "UpdateCamera() alles andere";
        return false;
      }
    }
    else
    {
      MITK_ERROR << "UpdateCamera() AcquireLatestFrame - Is the 'KinectService' App running in the background? Did you connect the device properly?";
      return false;
    }

    SafeRelease(d->m_pDepthFrame);
    SafeRelease(d->m_pColorFrame);

    return true;
  }

  void Kinect2Controller::GetDistances(float* distances)
  {
    memcpy(distances, d->m_Distances, sizeof(float)*512*424);
  }

  void Kinect2Controller::GetRgb(unsigned char* rgb)
  {
    memcpy(rgb, d->m_Colors, d->m_BufferSize);
  }

  void Kinect2Controller::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    this->GetDistances(distances);
    this->GetRgb(rgb);
  }

  void Kinect2Controller::GetAmplitudes( float* amplitudes )
  {
  }

  void Kinect2Controller::GetIntensities( float* intensities )
  {
  }

  int Kinect2Controller::GetRGBCaptureWidth() const
  {
    return d->m_RGBCaptureWidth;
  }

  int Kinect2Controller::GetRGBCaptureHeight() const
  {
    return d->m_RGBCaptureHeight;
  }

  int Kinect2Controller::GetDepthCaptureWidth() const
  {
    return d->m_DepthCaptureWidth;
  }

  int Kinect2Controller::GetDepthCaptureHeight() const
  {
    return d->m_DepthCaptureHeight;
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
