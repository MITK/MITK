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

//Kinect V2 SDK
#include <Kinect.h>

//VTK
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkMath.h>

#include <mitkVector.h>

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

    //bool ErrorText(unsigned int error);
    IKinectSensor* m_pKinectSensor;///<Kinect V2 sensor object
    IMultiSourceFrameReader* m_pMultiSourceFrameReader;///< Multiframe reader to read all frames at once
    ICoordinateMapper* m_pCoordinateMapper;///< Coordinate mapper allows for computation of wolrd coordinates and texture mapping
    RGBQUAD* m_pColorRGBX; ///< RGBX color format, to copy the color image

    bool m_ConnectionCheck; ///< check if camera is connected or not

    int m_DepthCaptureWidth; ///< Width of the depth image
    int m_DepthCaptureHeight; ///< Height of the depth image

    int m_RGBCaptureWidth;///< Width of the RGB image
    int m_RGBCaptureHeight;///< Height of the RGB image
    size_t m_RGBBufferSize;///< Size of the RGB buffer in byte (one unsigned char per color per pixel)
    size_t m_DepthBufferSize; ///< Size of the depth buffer in byte (one float per pixel)

    CameraSpacePoint* m_CameraCoordinates; ///< 3D world coordinate points of the Kinect V2 SDK
    ColorSpacePoint* m_ColorPoints;///< Texture coordinates of the Kinect V2 SDK
    vtkSmartPointer<vtkPolyData> m_PolyData;///< Conversion of m_CameraCoordinates to vtkPolyData

    double m_TriangulationThreshold; ///< Threshold to cut off vertices from triangulation
    bool m_GenerateTriangularMesh; ///< Apply triangulation or not
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
    m_RGBBufferSize(1920*1080*3),
    m_DepthBufferSize(sizeof(float)*512*424),
    m_CameraCoordinates(NULL),
    m_ColorPoints(NULL),
    m_PolyData(NULL),
    m_TriangulationThreshold(0.0),
    m_GenerateTriangularMesh(false)
  {
    // create heap storage for color pixel data in RGBX format
    m_pColorRGBX = new RGBQUAD[m_RGBCaptureWidth * m_RGBCaptureHeight];
    //initialize 3D world coordinates and texture coordinates
    m_CameraCoordinates = new CameraSpacePoint[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_ColorPoints = new ColorSpacePoint[m_DepthCaptureWidth * m_DepthCaptureHeight];
    m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  }

  KinectV2Controller::KinectV2ControllerPrivate::~KinectV2ControllerPrivate()
  {
    MITK_INFO << "~KinectV2ControllerPrivate";
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
    //Acquire lastest frame updates the camera and for
    //unknown reasons I cannot use it here in UpdateCamera()
    //without resulting in random crashes of the app.
    return true;
  }

  void KinectV2Controller::GetDistances(float* distances)
  {
    if(!InitializeMultiFrameReader())
    {
      MITK_ERROR << "Unable to initialize MultiFrameReader";
      return;
    }
    IMultiSourceFrame* pMultiSourceFrame = NULL;
    IDepthFrame* pDepthFrame = NULL;

    HRESULT hr = -1; //SDK error format

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
      UINT nDepthBufferSize = 0;
      UINT16 *pDepthBuffer = NULL;

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
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
          ++pDepthBuffer;

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
      }
      else
      {
        MITK_ERROR << "AccessUnderlyingBuffer";
      }
    }
    SafeRelease(pDepthFrame);
    SafeRelease(pMultiSourceFrame);

    if( hr != -1 && !SUCCEEDED(hr) )
    {
      //The thread gets here, if the data is requested faster than the device can deliver it.
      //This may happen from time to time.
      MITK_DEBUG << "HR result false in KinectV2Controller::GetDistances()";
      return;
    }
  }

  void KinectV2Controller::GetRgb(unsigned char* rgb)
  {
    if(!InitializeMultiFrameReader())
    {
      MITK_ERROR << "Unable to initialize MultiFrameReader";
      return;
    }
    IMultiSourceFrame* pMultiSourceFrame = NULL;
    IColorFrame* pColorFrame = NULL;

    HRESULT hr = -1;

    static DWORD lastTime = 0;
    DWORD currentTime = GetTickCount();
    //Check if we do not request data faster than 30 FPS. Kinect V2 can only deliver 30 FPS.
    if( unsigned int(currentTime - lastTime) > 33 )
    {
      hr = d->m_pMultiSourceFrameReader->AcquireLatestFrame(&pMultiSourceFrame);
      lastTime = currentTime;
    }

    ColorImageFormat imageFormat = ColorImageFormat_None;
    UINT nColorBufferSize = 0;
    RGBQUAD *pColorBuffer = NULL;
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
    SafeRelease(pColorFrame);
    SafeRelease(pMultiSourceFrame);

    if( hr != -1 && !SUCCEEDED(hr) )
    {
      //The thread gets here, if the data is requested faster than the device can deliver it.
      //This may happen from time to time.
      MITK_DEBUG << "HR result false in KinectV2Controller::GetRgb()";
    }
  }

  void KinectV2Controller::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    if(!InitializeMultiFrameReader())
    {
      MITK_ERROR << "Unable to initialize MultiFrameReader";
      return;
    }

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
      UINT16 *pInfraRedBuffer = NULL;

      ColorImageFormat imageFormat = ColorImageFormat_None;
      UINT nColorBufferSize = 0;
      RGBQUAD *pColorBuffer = NULL;

      if (SUCCEEDED(hr))
      {
        hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pDepthBuffer);
      }
      if (SUCCEEDED(hr))
      {
        hr = pInfraRedFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pInfraRedBuffer);
      }
      if (SUCCEEDED(hr))
      {
        UINT pointCount = d->m_DepthCaptureWidth * d->m_DepthCaptureHeight;
        d->m_pCoordinateMapper->MapDepthFrameToCameraSpace(pointCount, pDepthBuffer, pointCount, d->m_CameraCoordinates);
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();

        const double meterfactor = 1000.0;

        vtkSmartPointer<vtkFloatArray> textureCoordinates = vtkSmartPointer<vtkFloatArray>::New();
        vtkSmartPointer<vtkIdList> vertexIdList = vtkSmartPointer<vtkIdList>::New();
        vertexIdList->Allocate(pointCount);
        vertexIdList->SetNumberOfIds(pointCount);
        for(unsigned int i = 0; i < pointCount; ++i)
        {
          vertexIdList->SetId(i, 0);
        }

        std::vector<bool> isPointValid;
        isPointValid.resize(pointCount);
        //Allocate the object once else it would automatically allocate new memory
        //for every vertex and perform a copy which is expensive.
        vertexIdList->Allocate(pointCount);
        vertexIdList->SetNumberOfIds(pointCount);
        textureCoordinates->SetNumberOfComponents(2);
        textureCoordinates->Allocate(pointCount);

        d->m_pCoordinateMapper->MapDepthFrameToColorSpace(pointCount, pDepthBuffer, pointCount, d->m_ColorPoints);

        for(int j = 0; j < d->m_DepthCaptureHeight; ++j)
        {
          for(int i = 0; i < d->m_DepthCaptureWidth; ++i)
          {
            unsigned int pixelID = i+j*d->m_DepthCaptureWidth;
            unsigned int inverseid = (d->m_DepthCaptureWidth - i - 1) + j*d->m_DepthCaptureWidth;

            distances[inverseid] = static_cast<float>(*pDepthBuffer);
            amplitudes[inverseid] = static_cast<float>(*pInfraRedBuffer);
            ++pDepthBuffer;
            ++pInfraRedBuffer;

            if (d->m_CameraCoordinates[pixelID].Z<=mitk::eps)
            {
              isPointValid[pixelID] = false;
            }
            else
            {
              isPointValid[pixelID] = true;

              //VTK would insert empty points into the polydata if we use
              //points->InsertPoint(pixelID, cartesianCoordinates.GetDataPointer()).
              //If we use points->InsertNextPoint(...) instead, the ID's do not
              //correspond to the image pixel ID's. Thus, we have to save them
              //in the vertexIdList.
              //Kinect SDK delivers world coordinates in meters, so we have to
              //convert to mm for MITK.
              vertexIdList->SetId(pixelID, points->InsertNextPoint(-d->m_CameraCoordinates[pixelID].X*meterfactor, -d->m_CameraCoordinates[pixelID].Y*meterfactor, d->m_CameraCoordinates[pixelID].Z*meterfactor));

              if (d->m_GenerateTriangularMesh)
              {
                if((i >= 1) && (j >= 1))
                {
                  //This little piece of art explains the ID's:
                  //
                  // P(x_1y_1)---P(xy_1)
                  // |           |
                  // |           |
                  // |           |
                  // P(x_1y)-----P(xy)
                  //
                  //We can only start triangulation if we are at vertex (1,1),
                  //because we need the other 3 vertices near this one.
                  //To go one pixel line back in the image array, we have to
                  //subtract 1x xDimension.
                  vtkIdType xy = pixelID;
                  vtkIdType x_1y = pixelID-1;
                  vtkIdType xy_1 = pixelID-d->m_DepthCaptureWidth;
                  vtkIdType x_1y_1 = xy_1-1;

                  //Find the corresponding vertex ID's in the saved vertexIdList:
                  vtkIdType xyV = vertexIdList->GetId(xy);
                  vtkIdType x_1yV = vertexIdList->GetId(x_1y);
                  vtkIdType xy_1V = vertexIdList->GetId(xy_1);
                  vtkIdType x_1y_1V = vertexIdList->GetId(x_1y_1);

                  if (isPointValid[xy]&&isPointValid[x_1y]&&isPointValid[x_1y_1]&&isPointValid[xy_1]) // check if points of cell are valid
                  {
                    double pointXY[3], pointX_1Y[3], pointXY_1[3], pointX_1Y_1[3];

                    points->GetPoint(xyV, pointXY);
                    points->GetPoint(x_1yV, pointX_1Y);
                    points->GetPoint(xy_1V, pointXY_1);
                    points->GetPoint(x_1y_1V, pointX_1Y_1);


                    if( (mitk::Equal(d->m_TriangulationThreshold, 0.0)) || ((vtkMath::Distance2BetweenPoints(pointXY, pointX_1Y) <= d->m_TriangulationThreshold)
                      && (vtkMath::Distance2BetweenPoints(pointXY, pointXY_1) <= d->m_TriangulationThreshold)
                      && (vtkMath::Distance2BetweenPoints(pointX_1Y, pointX_1Y_1) <= d->m_TriangulationThreshold)
                      && (vtkMath::Distance2BetweenPoints(pointXY_1, pointX_1Y_1) <= d->m_TriangulationThreshold)))
                    {
                      polys->InsertNextCell(3);
                      polys->InsertCellPoint(x_1yV);
                      polys->InsertCellPoint(xyV);
                      polys->InsertCellPoint(x_1y_1V);

                      polys->InsertNextCell(3);
                      polys->InsertCellPoint(x_1y_1V);
                      polys->InsertCellPoint(xyV);
                      polys->InsertCellPoint(xy_1V);
                    }
                    else
                    {
                      //We dont want triangulation, but we want to keep the vertex
                      vertices->InsertNextCell(1);
                      vertices->InsertCellPoint(xyV);
                    }
                  }
                }
              }
              else
              {
                //We dont want triangulation, we only want vertices
                vertices->InsertNextCell(1);
                vertices->InsertCellPoint(vertexIdList->GetId(pixelID));
              }

              ColorSpacePoint colorPoint = d->m_ColorPoints[pixelID];
              // retrieve the depth to color mapping for the current depth pixel
              int colorInDepthX = (int)(floor(colorPoint.X + 0.5));
              int colorInDepthY = (int)(floor(colorPoint.Y + 0.5));

              float xNorm = -static_cast<float>(colorInDepthX)/d->m_RGBCaptureWidth;
              float yNorm = static_cast<float>(colorInDepthY)/d->m_RGBCaptureHeight;

              // make sure the depth pixel maps to a valid point in color space
              if ( colorInDepthX >= 0 && colorInDepthX < d->m_RGBCaptureWidth && colorInDepthY >= 0 && colorInDepthY < d->m_RGBCaptureHeight )
              {
                textureCoordinates->InsertTuple2(vertexIdList->GetId(pixelID), xNorm, yNorm);
              }
              else
              {
                textureCoordinates->InsertTuple2(vertexIdList->GetId(pixelID), 0, 0);
              }
            }
          }
        }

        d->m_PolyData = vtkSmartPointer<vtkPolyData>::New();
        d->m_PolyData->SetPoints(points);
        d->m_PolyData->SetVerts(vertices);
        d->m_PolyData->SetPolys(polys);
        d->m_PolyData->GetPointData()->SetTCoords(textureCoordinates);
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
          for(int j = 0; j < d->m_RGBCaptureHeight; ++j)
          {
            for(int i = 0; i < d->m_RGBCaptureWidth; ++i)
            {
              //the buffer has the size of 3*ResolutionX/Y (one for each color value)
              //thats why die id is multiplied by 3.
              unsigned int id = ((d->m_RGBCaptureWidth - i - 1) + j*d->m_RGBCaptureWidth)*3;
              //convert from BGR to RGB
              rgb[id+0] = pColorBuffer->rgbRed;
              rgb[id+1] = pColorBuffer->rgbGreen;
              rgb[id+2] = pColorBuffer->rgbBlue;
              ++pColorBuffer;
            }
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
      MITK_DEBUG << "HR result false in KinectV2Controller::GetAllData()";
    }
  }

  void KinectV2Controller::GetAmplitudes( float* amplitudes )
  {
    if(!InitializeMultiFrameReader())
    {
      MITK_ERROR << "Unable to initialize MultiFrameReader";
      return;
    }

    IMultiSourceFrame* pMultiSourceFrame = NULL;
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
      UINT16 *pInfraRedBuffer = NULL;

      if (SUCCEEDED(hr))
      {
        hr = pInfraRedFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &pInfraRedBuffer);
      }
      if (SUCCEEDED(hr))
      {
        for(int i = 0; i < d->m_DepthCaptureHeight*d->m_DepthCaptureWidth; ++i)
        {
          amplitudes[i] = static_cast<float>(*pInfraRedBuffer);
          ++pInfraRedBuffer;
        }
      }
      else
      {
        MITK_ERROR << "AccessUnderlyingBuffer";
      }
    }
    SafeRelease(pInfraRedFrame);
    SafeRelease(pMultiSourceFrame);

    if( hr != -1 && !SUCCEEDED(hr) )
    {
      //The thread gets here, if the data is requested faster than the device can deliver it.
      //This may happen from time to time.
      MITK_DEBUG << "HR result false in KinectV2Controller::GetAmplitudes()";
    }
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

  void KinectV2Controller::SetGenerateTriangularMesh(bool flag)
  {
    d->m_GenerateTriangularMesh = flag;
  }

  void KinectV2Controller::SetTriangulationThreshold(double triangulationThreshold)
  {
    this->d->m_TriangulationThreshold = triangulationThreshold * triangulationThreshold;
  }
}
