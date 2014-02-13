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
#include <mitkSurface.h>

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

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
    ICoordinateMapper* m_pCoordinateMapper;
    RGBQUAD* m_pColorRGBX;

    bool m_ConnectionCheck; ///< check if camera is connected or not

    int m_DepthCaptureWidth;
    int m_DepthCaptureHeight;

    int m_RGBCaptureWidth;
    int m_RGBCaptureHeight;
    float* m_Distances;
    float* m_Amplitudes;
    unsigned char* m_Colors;
    size_t m_RGBBufferSize;
    size_t m_DepthBufferSize;

    //mitk::Surface::Pointer m_Surface;
    CameraSpacePoint* m_CameraCoordinates;
    vtkSmartPointer<vtkPolyData> m_PolyData;

    ColorSpacePoint* m_ColorPoints;
  };

  KinectV2Controller::KinectV2ControllerPrivate::KinectV2ControllerPrivate():
    m_pKinectSensor(NULL),
    m_pMultiSourceFrameReader(NULL),
    m_pCoordinateMapper(NULL),
    m_pColorRGBX(NULL),
    m_ConnectionCheck(false),
    m_DepthCaptureWidth(512),
    m_DepthCaptureHeight(424),
    m_RGBCaptureWidth(1920),
    m_RGBCaptureHeight(1080),
    m_Distances(NULL),
    m_Amplitudes(NULL),
    m_Colors(NULL),
    m_RGBBufferSize(1920*1080*3),
    m_DepthBufferSize(sizeof(float)*512*424),
    //m_Surface(NULL),
    m_CameraCoordinates(NULL),
    m_PolyData(NULL),
    m_ColorPoints(NULL)
  {
    // create heap storage for color pixel data in RGBX format
    m_pColorRGBX = new RGBQUAD[m_RGBCaptureWidth * m_RGBCaptureHeight];
    m_Distances = new float[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_Amplitudes = new float[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_Colors = new unsigned char[m_RGBBufferSize];

    //m_Surface = mitk::Surface::New();
    m_CameraCoordinates = new CameraSpacePoint[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_PolyData = vtkSmartPointer<vtkPolyData>::New();
    m_ColorPoints = new ColorSpacePoint[m_DepthCaptureWidth * m_DepthCaptureHeight];
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
   return true;
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
    if(InitializeMultiFrameReader())
    {

      IMultiSourceFrame* pMultiSourceFrame = NULL;
      IDepthFrame* pDepthFrame = NULL;
      IColorFrame* pColorFrame = NULL;
      IInfraredFrame* pInfraRedFrame = NULL;

      HRESULT hr = -1;

      static DWORD lastTime = 0;

      DWORD currentTime = GetTickCount();

      //Check if we do not request data faster than 30 FPS. Kinect V2 can only deliver 30 FPS.
      if( unsigned int(currentTime - lastTime) > 33 )
      {
        hr = d->m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);
        lastTime = currentTime;
      }

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
        IInfraredFrameReference* pInfraredFrameReference = NULL;

        hr = pMultiSourceFrame->get_InfraredFrameReference(&pInfraredFrameReference);
        if (SUCCEEDED(hr))
        {
          hr = pInfraredFrameReference->AcquireFrame(&pInfraRedFrame);
        }

        SafeRelease(pInfraredFrameReference);
      }

      if (SUCCEEDED(hr))
      {
        UINT nDepthBufferSize = 0;
        UINT16 *pDepthBuffer = NULL;
        UINT16 *pIntraRedBuffer = NULL;

        ColorImageFormat imageFormat = ColorImageFormat_None;
        UINT nColorBufferSize = 0;
        RGBQUAD *pColorBuffer = NULL;

        if (SUCCEEDED(hr))
        {
          hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
        }
        if (SUCCEEDED(hr))
        {
          hr = pInfraRedFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pIntraRedBuffer);
        }
        if (SUCCEEDED(hr))
        {
          UINT pointCount = d->m_DepthCaptureWidth * d->m_DepthCaptureHeight;
          d->m_pCoordinateMapper->MapDepthFrameToCameraSpace(pointCount, pDepthBuffer, pointCount, d->m_CameraCoordinates);
          vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
          vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
          vtkSmartPointer<vtkFloatArray> textureCoordinates = vtkSmartPointer<vtkFloatArray>::New();
          textureCoordinates->SetNumberOfComponents(2);
          textureCoordinates->Allocate(pointCount);

          d->m_pCoordinateMapper->MapDepthFrameToColorSpace(pointCount, pDepthBuffer, pointCount, d->m_ColorPoints);

          for(int i = 0; i < d->m_DepthCaptureHeight*d->m_DepthCaptureWidth; ++i)
          {
            vtkIdType id = points->InsertNextPoint(d->m_CameraCoordinates[i].X, d->m_CameraCoordinates[i].Y, d->m_CameraCoordinates[i].Z);
            vertices->InsertNextCell(1);
            vertices->InsertCellPoint(id);
            distances[i] = static_cast<float>(*pDepthBuffer);
            amplitudes[i] = static_cast<float>(*pIntraRedBuffer);
            ++pDepthBuffer;
            ++pIntraRedBuffer;

            ColorSpacePoint colorPoint = d->m_ColorPoints[i];
            // retrieve the depth to color mapping for the current depth pixel
            int colorInDepthX = (int)(floor(colorPoint.X + 0.5));
            int colorInDepthY = (int)(floor(colorPoint.Y + 0.5));

            float xNorm = static_cast<float>(colorInDepthX)/d->m_RGBCaptureWidth;
            float yNorm = static_cast<float>(colorInDepthY)/d->m_RGBCaptureHeight;

            // make sure the depth pixel maps to a valid point in color space
            if ( colorInDepthX >= 0 && colorInDepthX < d->m_RGBCaptureWidth && colorInDepthY >= 0 && colorInDepthY < d->m_RGBCaptureHeight )
            {
              textureCoordinates->InsertTuple2(id, xNorm, yNorm);
            }
          }
          d->m_PolyData = vtkSmartPointer<vtkPolyData>::New();
          d->m_PolyData->SetPoints(points);
          d->m_PolyData->SetVerts(vertices);
          d->m_PolyData->GetPointData()->SetTCoords(textureCoordinates);
          d->m_PolyData->Modified();
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
              rgb[i+0] = pColorBuffer->rgbRed;
              rgb[i+1] = pColorBuffer->rgbGreen;
              rgb[i+2] = pColorBuffer->rgbBlue;
              ++pColorBuffer;
            }
          }
        }
      }

      SafeRelease(pDepthFrame);
      SafeRelease(pColorFrame);
      SafeRelease(pInfraRedFrame);
      SafeRelease(pMultiSourceFrame);

      if( hr != -1 && !SUCCEEDED(hr) )
      {
        //The thread gets here, if the data is requested faster than the device can deliver it.
        //This may happen from time to time.
        return;
      }
      return;
    }
    MITK_ERROR << "Unable to initialize MultiFrameReader";
  }

  void KinectV2Controller::GetAmplitudes( float* amplitudes )
  {
    memcpy( amplitudes, d->m_Amplitudes, d->m_DepthBufferSize);
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

  vtkSmartPointer<vtkPolyData> KinectV2Controller::GetVtkPolyData()
  {
    return d->m_PolyData;
  }
}
