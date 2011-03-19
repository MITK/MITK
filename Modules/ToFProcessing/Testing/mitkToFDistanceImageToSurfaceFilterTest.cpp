/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
//#include <mitkToFSurfaceGenerationFilter.h>

#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkToFProcessingCommon.h>
#include <mitkVector.h>
#include <mitkToFTestingCommon.h>

//#include <itkImage.h>
//#include <itkImageRegionIterator.h>
//#include <itkMersenneTwisterRandomVariateGenerator.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

/**Documentation
 *  test for the class "ToFDistanceImageToSurfaceFilter".
 */

typedef mitk::ToFProcessingCommon::ToFPoint2D ToFPoint2D;
typedef mitk::ToFProcessingCommon::ToFPoint3D ToFPoint3D;
typedef mitk::ToFProcessingCommon::ToFScalarType ToFScalarType;

int mitkToFDistanceImageToSurfaceFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFDistanceImageToSurfaceFilter");
  mitk::ToFDistanceImageToSurfaceFilter::Pointer filter = mitk::ToFDistanceImageToSurfaceFilter::New();
  // create test image
  unsigned int dimX =204;
  unsigned int dimY =204;
  mitk::Image::Pointer image = mitk::ToFTestingCommon::CreateTestImage(dimX,dimY);
  //initialize intrinsic parameters with some arbitrary values
  ToFScalarType focalLength = 13.654368;
  ToFPoint2D interPixelDistance;
  interPixelDistance[0] = 0.04564;
  interPixelDistance[1] = 0.0451564;
  ToFPoint2D principalPoint;
  principalPoint[0] = 103.576546;
  principalPoint[1] = 100.1532;

  mitk::PinholeCameraModel::Pointer camera = mitk::PinholeCameraModel::New();
  camera->SetFocalLength(focalLength);
  camera->SetPrincipalPoint(principalPoint);
  camera->SetInterPixelDistance(interPixelDistance);

  filter->SetCameraModel(camera);

  MITK_TEST_CONDITION_REQUIRED((focalLength==filter->GetCameraModel()->GetFocalLength()),"Testing Set/GetFocalLength()");
  filter->GetCameraModel()->SetInterPixelDistance(interPixelDistance);
  MITK_TEST_CONDITION_REQUIRED((interPixelDistance==filter->GetCameraModel()->GetInterPixelDistance()),"Testing Set/GetInterPixelDistance()");
  filter->GetCameraModel()->SetPrincipalPoint(principalPoint);
  ToFPoint2D pp = filter->GetCameraModel()->GetPrincipalPoint();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing Set/GetPrincipalPoint()");
  // test SetIntrinsicParameters()
  mitk::PinholeCameraModel::Pointer cameraIntrinics = mitk::PinholeCameraModel::New();
  ToFPoint2D imageArea;
  imageArea[0] = dimX*interPixelDistance[0];
  imageArea[1] = dimY*interPixelDistance[1];
  cameraIntrinics->SetImageArea(imageArea);
  cameraIntrinics->SetFocalLength(focalLength);
  ToFPoint2D pixelSize;
  pixelSize[0] = interPixelDistance[0];
  pixelSize[1] = interPixelDistance[1];
  cameraIntrinics->SetInterPixelDistance(pixelSize);
  ToFPoint2D imageShift;
  imageShift[0] = (principalPoint[0] - dimX/2)*interPixelDistance[0];
  imageShift[1] = (principalPoint[1] - dimX/2)*interPixelDistance[1];
  cameraIntrinics->SetImageShift(imageShift);
  cameraIntrinics->SetPrincipalPoint(principalPoint);
  filter->SetCameraModel(cameraIntrinics);
  MITK_TEST_CONDITION_REQUIRED((focalLength==filter->GetCameraModel()->GetFocalLength()),"Testing SetIntrinsicParameters() - focal length");
  MITK_TEST_CONDITION_REQUIRED((interPixelDistance==filter->GetCameraModel()->GetInterPixelDistance()),"Testing SetIntrinsicParameters() - inter pixel distance");
  pp = filter->GetCameraModel()->GetPrincipalPoint();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing SetIntrinsicParameters() - principal point");

  // test Set/GetInput()
  filter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED((image==filter->GetInput()),"Testing Set/GetInput()");

  // test filter without subset
  MITK_INFO<<"Test filter ";
  vtkSmartPointer<vtkPoints> expectedResult = vtkSmartPointer<vtkPoints>::New();
  expectedResult->SetDataTypeToDouble();
  unsigned int counter = 0;
  double* point = new double[3];
//  MITK_INFO<<"Test";
//  MITK_INFO<<"focal: "<<focalLength;
//  MITK_INFO<<"inter: "<<interPixelDistance;
//  MITK_INFO<<"prinicipal: "<<principalPoint;
  for (unsigned int j=0; j<dimX; j++)
  {
    for (unsigned int i=0; i<dimY; i++)
    {
      mitk::Index3D index;
      index[0] = i;
      index[1] = j;
      index[2] = 0;
      ToFScalarType distance = image->GetPixelValueByIndex(index);
      ToFPoint3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLength,interPixelDistance,principalPoint);
//      if ((i==0)&&(j==0))
//      {
//        MITK_INFO<<"Distance test: "<<distance;
//        MITK_INFO<<"coordinate test: "<<coordinate;
//      }
      point[0] = coordinate[0];
      point[1] = coordinate[1];
      point[2] = coordinate[2];
      unsigned int pointID = index[0] + index[1]*dimY;
      //MITK_INFO<<"id: "<<pointID;
      //MITK_INFO<<"counter: "<<counter;
      if (distance!=0)
      {
        expectedResult->InsertPoint(pointID,point);
      }
      counter++;
    }
  }
  filter->Update();
  mitk::Surface::Pointer resultSurface = filter->GetOutput();
  vtkSmartPointer<vtkPoints> result = vtkSmartPointer<vtkPoints>::New();
  result->SetDataTypeToDouble();
  result = resultSurface->GetVtkPolyData()->GetPoints();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetNumberOfPoints()==result->GetNumberOfPoints()),"Test if number of points in surface is equal");
  bool pointSetsEqual = true;
  for (unsigned int i=0; i<expectedResult->GetNumberOfPoints(); i++)
  {
    double* expected = expectedResult->GetPoint(i);
    double* res = result->GetPoint(i);

    ToFPoint3D expectedPoint;
    expectedPoint[0] = expected[0];
    expectedPoint[1] = expected[1];
    expectedPoint[2] = expected[2];
    ToFPoint3D resultPoint;
    resultPoint[0] = res[0];
    resultPoint[1] = res[1];
    resultPoint[2] = res[2];

    if (!mitk::Equal(expectedPoint,resultPoint))
    {
//      MITK_INFO << i;
      MITK_INFO<<"expected: "<<expectedPoint;
      MITK_INFO<<"result: "<<resultPoint;
      pointSetsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(pointSetsEqual,"Testing filter without subset");

  //Backwardtransformation test
  bool backwardTransformationsPointsEqual = true;
  for (unsigned int i=0; i<expectedResult->GetNumberOfPoints(); i++)
  {
    double* expected = expectedResult->GetPoint(i);
    double* res = result->GetPoint(i);

    ToFPoint3D expectedPoint;
    expectedPoint[0] = expected[0];
    expectedPoint[1] = expected[1];
    expectedPoint[2] = expected[2];
    ToFPoint3D resultPoint;
    resultPoint[0] = res[0];
    resultPoint[1] = res[1];
    resultPoint[2] = res[2];

    ToFPoint3D expectedPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(expectedPoint,
                                                               filter->GetCameraModel()->GetFocalLength(),
                                                               filter->GetCameraModel()->GetInterPixelDistance(),
                                                               filter->GetCameraModel()->GetPrincipalPoint());

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(resultPoint,
                                                               filter->GetCameraModel()->GetFocalLength(),
                                                               filter->GetCameraModel()->GetInterPixelDistance(),
                                                               filter->GetCameraModel()->GetPrincipalPoint());

    if (!mitk::Equal(expectedPointBackward,resultPointBackward))
    {
//      MITK_INFO << i;
//      MITK_INFO<<"expected: "<<expectedPoint;
//      MITK_INFO<<"result: "<<resultPoint;
      backwardTransformationsPointsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(backwardTransformationsPointsEqual,"Testing backward transformation");

  //Backwardtransformation test compare to original input
  bool compareToInput = true;
  for (unsigned int i=0; i<result->GetNumberOfPoints(); i++)
  {
    double* res = result->GetPoint(i);

    ToFPoint3D resultPoint;
    resultPoint[0] = res[0];
    resultPoint[1] = res[1];
    resultPoint[2] = res[2];

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(resultPoint,
                                                               filter->GetCameraModel()->GetFocalLength(),
                                                               filter->GetCameraModel()->GetInterPixelDistance(),
                                                               filter->GetCameraModel()->GetPrincipalPoint());

    mitk::Index3D pixelIndex;
    pixelIndex[0] = (int) (resultPointBackward[0]+0.5);
    pixelIndex[1] = (int) (resultPointBackward[1]+0.5);
    pixelIndex[2] = 0;

    if (!mitk::Equal(image->GetPixelValueByIndex(pixelIndex),resultPointBackward[2]))
    {
//      MITK_INFO<<"expected: "<< image->GetPixelValueByIndex(pixelIndex);
//      MITK_INFO<<"result: "<< resultPoint;
      compareToInput = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(compareToInput,"Testing backward transformation compared to original image");

  //clean up
  delete point;
  //  expectedResult->Delete();

  MITK_TEST_END();

}


