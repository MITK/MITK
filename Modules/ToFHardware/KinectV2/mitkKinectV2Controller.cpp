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
#include "mitkKinectV2Controller.h"
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
  class KinectV2Controller::KinectV2ControllerPrivate
  {
  public:
    KinectV2ControllerPrivate();
    ~KinectV2ControllerPrivate();

    bool ErrorText(unsigned int error);
    IKinectSensor* m_pKinectSensor;
    IMultiSourceFrameReader*m_pMultiSourceFrameReader;
    IDepthFrame* m_pDepthFrame;
    IColorFrame* m_pColorFrame;
    ICoordinateMapper* m_pCoordinateMapper;

    RGBQUAD* m_pColorRGBX;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    bool m_UseIR; ///< flag indicating whether IR image is used or not

    int m_DepthCaptureWidth;
    int m_DepthCaptureHeight;

    int m_RGBCaptureWidth;
    int m_RGBCaptureHeight;
    float* m_Distances;
    unsigned char* m_Colors;
    size_t m_RGBBufferSize;
    size_t m_DepthBufferSize;
  };

  KinectV2Controller::KinectV2ControllerPrivate::KinectV2ControllerPrivate():
    m_pKinectSensor(NULL),
    m_pMultiSourceFrameReader(NULL),
    m_pDepthFrame(NULL),
    m_pColorFrame(NULL),
    m_pCoordinateMapper(NULL),
    m_pColorRGBX(NULL),
    m_ConnectionCheck(false),
    m_UseIR(false),
    m_DepthCaptureWidth(512),
    m_DepthCaptureHeight(424),
    m_RGBCaptureWidth(1920),
    m_RGBCaptureHeight(1080),
    m_Distances(NULL),
    m_Colors(NULL),
    m_RGBBufferSize(1920*1080*3),
    m_DepthBufferSize(sizeof(float)*512*424)
  {
    // create heap storage for color pixel data in RGBX format
    m_pColorRGBX = new RGBQUAD[m_RGBCaptureWidth * m_RGBCaptureHeight];
    m_Distances = new float[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_Colors = new unsigned char[m_RGBBufferSize];
  }

  KinectV2Controller::KinectV2ControllerPrivate::~KinectV2ControllerPrivate()
  {
    MITK_INFO << "~KinectV2ControllerPrivate";
  }

  bool KinectV2Controller::KinectV2ControllerPrivate::ErrorText(unsigned int error)
  {
    return true;
  }

  KinectV2Controller::KinectV2Controller(): d(new KinectV2ControllerPrivate)
  {
  }

  KinectV2Controller::~KinectV2Controller()
  {
    MITK_INFO << "~KinectV2Controller";
    delete d;
  }

  bool KinectV2Controller::OpenCameraConnection()
  {
    if (!d->m_ConnectionCheck)
    {
      HRESULT hr;
      d->m_ConnectionCheck = true;

      hr = GetDefaultKinectSensor(&d->m_pKinectSensor);

      if (FAILED(hr))
      {
        d->m_ConnectionCheck = false;
      }
      else
      {
        hr = d->m_pKinectSensor->get_CoordinateMapper(&d->m_pCoordinateMapper);
        if (FAILED(hr))
        {
          d->m_ConnectionCheck = false;
        }
        hr = d->m_pKinectSensor->Open();
      }

      if (!d->m_pKinectSensor || FAILED(hr))
      {
        d->m_ConnectionCheck = false;
        MITK_WARN << "No Kinect 2 ready!";
      }
      else
      {
        MITK_INFO << "Kinect 2 succesfully connected";
      }
    }
    return d->m_ConnectionCheck;
  }

  bool KinectV2Controller::InitializeMultiFrameReader()
  {
    //check if it is already initialized
    if((d->m_ConnectionCheck) && (d->m_pMultiSourceFrameReader))
    {
      return true;
    }
    else //initialize the frame reader
    {
      HRESULT hr = d->m_pKinectSensor->OpenMultiSourceFrameReader(
        FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color | FrameSourceTypes::FrameSourceTypes_Infrared,
        &d->m_pMultiSourceFrameReader);
      if (SUCCEEDED(hr) && (d->m_pMultiSourceFrameReader))
      {
        MITK_INFO << "KinectV2 MultiFrameReader initialized";
        return true;
      }
    }
    return false;
  }


  bool KinectV2Controller::CloseCameraConnection()
  {
    // done with depth frame reader
    MITK_INFO << "CloseConnection";
    SafeRelease(d->m_pMultiSourceFrameReader);

    // close the Kinect Sensor
    if(d->m_pKinectSensor)
    {
      d->m_pKinectSensor->Close();
    }

    SafeRelease(d->m_pKinectSensor);
    d->m_ConnectionCheck = false;
    return true;
  }

  bool KinectV2Controller::UpdateCamera()
  {
    if(InitializeMultiFrameReader())
    {

      IMultiSourceFrame* pMultiSourceFrame = NULL;
      IDepthFrame* pDepthFrame = NULL;
      IColorFrame* pColorFrame = NULL;

      HRESULT hr = d->m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);

      if (SUCCEEDED(hr))
      {
        IDepthFrameReference* pDepthFrameReference = NULL;

        hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
        if (SUCCEEDED(hr))
        {
          hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
        }

        SafeRelease(pDepthFrameReference);
      }

      if (SUCCEEDED(hr))
      {
        IColorFrameReference* pColorFrameReference = NULL;

        hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
        if (SUCCEEDED(hr))
        {
          hr = pColorFrameReference->AcquireFrame(&pColorFrame);
        }

        SafeRelease(pColorFrameReference);
      }

      if (SUCCEEDED(hr))
      {
        UINT nDepthBufferSize = 0;
        UINT16 *pDepthBuffer = NULL;

        ColorImageFormat imageFormat = ColorImageFormat_None;
        UINT nColorBufferSize = 0;
        RGBQUAD *pColorBuffer = NULL;

        if (SUCCEEDED(hr))
        {
          hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
        }
        if (SUCCEEDED(hr))
        {
          for(int i = 0; i < d->m_DepthCaptureHeight*d->m_DepthCaptureWidth; ++i)
          {
            float depth = static_cast<float>(*pDepthBuffer);
            d->m_Distances[i] = depth;
            ++pDepthBuffer;
          }
        }
        else
        {
          MITK_ERROR << "AccessUnderlyingBuffer";
        }

        // get color frame data
        if (SUCCEEDED(hr))
        {
          hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
        }

        if (SUCCEEDED(hr))
        {
          if (imageFormat == ColorImageFormat_Bgra)
          {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
          }
          else if (d->m_pColorRGBX)
          {
            pColorBuffer = d->m_pColorRGBX;
            nColorBufferSize = d->m_RGBCaptureWidth * d->m_RGBCaptureHeight * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
          }
          else
          {
            hr = E_FAIL;
          }
          if (SUCCEEDED(hr))
          {
            for(int i = 0; i < d->m_RGBBufferSize; i+=3)
            {
              //convert from BGR to RGB
              d->m_Colors[i+0] = pColorBuffer->rgbRed;
              d->m_Colors[i+1] = pColorBuffer->rgbGreen;
              d->m_Colors[i+2] = pColorBuffer->rgbBlue;
              ++pColorBuffer;
            }
          }
        }
      }
      else
      {
        MITK_ERROR << hr;
        //MITK_ERROR << "UpdateCamera() AcquireFrame - Is the 'KinectService' App running in the background? Did you connect the device properly?";
        return false;
      }
      SafeRelease(pDepthFrame);
      SafeRelease(pColorFrame);
      SafeRelease(pMultiSourceFrame);
      MITK_INFO << "SUCCESS";
      return true;
    }
    MITK_ERROR << "Unable to initialize MultiFrameReader";
    return false;
  }

  void KinectV2Controller::GetDistances(float* distances)
  {
    memcpy(distances, d->m_Distances, d->m_DepthBufferSize);
  }

  void KinectV2Controller::GetRgb(unsigned char* rgb)
  {
    memcpy(rgb, d->m_Colors, d->m_RGBBufferSize);
  }

  void KinectV2Controller::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    this->GetDistances(distances);
    this->GetRgb(rgb);
  }

  void KinectV2Controller::GetAmplitudes( float* amplitudes )
  {
  }

  void KinectV2Controller::GetIntensities( float* intensities )
  {
  }

  int KinectV2Controller::GetRGBCaptureWidth() const
  {
    return d->m_RGBCaptureWidth;
  }

  int KinectV2Controller::GetRGBCaptureHeight() const
  {
    return d->m_RGBCaptureHeight;
  }

  int KinectV2Controller::GetDepthCaptureWidth() const
  {
    return d->m_DepthCaptureWidth;
  }

  int KinectV2Controller::GetDepthCaptureHeight() const
  {
    return d->m_DepthCaptureHeight;
  }

  bool KinectV2Controller::GetUseIR() const
  {
    return d->m_UseIR;
  }
  void KinectV2Controller::SetUseIR(bool useIR)
  {
    if (d->m_UseIR!=useIR)
    {
      d->m_UseIR = useIR;
      this->Modified();
    }
  }
}
