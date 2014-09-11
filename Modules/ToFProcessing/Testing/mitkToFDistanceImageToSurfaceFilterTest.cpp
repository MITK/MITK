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

#include <mitkTestingMacros.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>

#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageGenerator.h>
#include <mitkSurface.h>
#include <mitkToFProcessingCommon.h>
#include <mitkNumericTypes.h>
#include <mitkToFTestingCommon.h>
#include <mitkIOUtil.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

/**
 *  @brief Test for the class "ToFDistanceImageToSurfaceFilter".
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
  mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(dimX,dimY);
  //initialize intrinsic parameters with some arbitrary values
  ToFScalarType focalLengthX = 295.78960;
  ToFScalarType focalLengthY = 296.348535;
  ToFPoint2D focalLengthXY;
  focalLengthXY[0]=focalLengthX;
  focalLengthXY[1]=focalLengthY;
  ToFScalarType k1=-0.36,k2=-0.14,p1=0.001,p2=-0.00;
  ToFPoint2D principalPoint;
  principalPoint[0] = 103.576546;
  principalPoint[1] = 100.1532;
  mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
  cameraIntrinsics->SetFocalLength(focalLengthX,focalLengthY);
  cameraIntrinsics->SetPrincipalPoint(principalPoint[0],principalPoint[1]);
  cameraIntrinsics->SetDistorsionCoeffs(k1,k2,p1,p2);
  // test SetCameraIntrinsics()
  filter->SetCameraIntrinsics(cameraIntrinsics);
  MITK_TEST_CONDITION_REQUIRED((focalLengthX==filter->GetCameraIntrinsics()->GetFocalLengthX()),"Testing SetCameraIntrinsics with focalLength");
  ToFPoint2D pp;
  pp[0] = filter->GetCameraIntrinsics()->GetPrincipalPointX();
  pp[1] = filter->GetCameraIntrinsics()->GetPrincipalPointY();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing SetCameraIntrinsics with principalPoint()");
  // test SetInterPixelDistance()
  ToFPoint2D interPixelDistance;
  interPixelDistance[0] = 0.04564;
  interPixelDistance[1] = 0.0451564;
  filter->SetInterPixelDistance(interPixelDistance);
  ToFPoint2D ipD = filter->GetInterPixelDistance();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(ipD,interPixelDistance),"Testing Set/GetInterPixelDistance()");

  // test SetReconstructionMode()
  filter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance);
  MITK_TEST_CONDITION_REQUIRED(filter->GetReconstructionMode() == mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance,"Testing Set/GetReconstructionMode()");

  // test Set/GetInput()
  filter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED((image==filter->GetInput()),"Testing Set/GetInput()");

  // test filter without subset (without interpixeldistance)
  MITK_INFO<<"Test filter with subset without interpixeldistance ";
  filter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::WithOutInterPixelDistance);
    MITK_TEST_CONDITION_REQUIRED(filter->GetReconstructionMode() == mitk::ToFDistanceImageToSurfaceFilter::WithOutInterPixelDistance,"Testing Set/GetReconstructionMode()");

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
      itk::Index<2> index = {{ i, j }};
      float distance = 0.0;

      try
      {
        mitk::ImagePixelReadAccessor<float,2> readAccess(image, image->GetSliceData());
        distance = readAccess.GetPixelByIndex(index);
      }
      catch(mitk::Exception& e)
      {
          MITK_ERROR << "Image read exception!" << e.what();
      }


      ToFPoint3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLengthX,focalLengthY,principalPoint[0],principalPoint[1]);
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
      pointSetsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(pointSetsEqual,"Testing filter without subset");

  // test filter without subset (with interpixeldistance)
  MITK_INFO<<"Test filter with subset with interpixeldistance ";
  filter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance);
  MITK_TEST_CONDITION_REQUIRED(filter->GetReconstructionMode() == mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance,"Testing Set/GetReconstructionMode()");
  // calculate focal length considering inter pixel distance
  ToFScalarType focalLength = (focalLengthX*interPixelDistance[0]+focalLengthY*interPixelDistance[1])/2.0;
  expectedResult = vtkSmartPointer<vtkPoints>::New();
  expectedResult->SetDataTypeToDouble();
  counter = 0;
  point = new double[3];
//  MITK_INFO<<"Test";
//  MITK_INFO<<"focal: "<<focalLength;
//  MITK_INFO<<"inter: "<<interPixelDistance;
//  MITK_INFO<<"prinicipal: "<<principalPoint;
  for (unsigned int j=0; j<dimX; j++)
  {
    for (unsigned int i=0; i<dimY; i++)
    {
        itk::Index<2> index = {{ i, j }};
        float distance = 0.0;
        try
        {
          mitk::ImagePixelReadAccessor<float,2> readAccess(image, image->GetSliceData());
          distance = readAccess.GetPixelByIndex(index);
        }
        catch(mitk::Exception& e)
        {
            MITK_ERROR << "Image read exception!" << e.what();
        }
      ToFPoint3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(i,j,distance,focalLength,interPixelDistance,principalPoint);
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
  filter->Modified();
  filter->Update();
  resultSurface = filter->GetOutput();
  result = vtkSmartPointer<vtkPoints>::New();
  result->SetDataTypeToDouble();
  result = resultSurface->GetVtkPolyData()->GetPoints();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetNumberOfPoints()==result->GetNumberOfPoints()),"Test if number of points in surface is equal");
  pointSetsEqual = true;
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


  //Backwardtransformation test without interpixeldistance
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
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(expectedPoint,focalLengthXY,principalPoint);

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(resultPoint,focalLengthXY,principalPoint);

    if (!mitk::Equal(expectedPointBackward,resultPointBackward))
    {
//      MITK_INFO << i;
//      MITK_INFO<<"expected: "<<expectedPoint;
//      MITK_INFO<<"result: "<<resultPoint;
      backwardTransformationsPointsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(backwardTransformationsPointsEqual,"Testing backward transformation without interpixeldistance");

  //Backwardtransformation test with interpixeldistance
  backwardTransformationsPointsEqual = true;
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
        mitk::ToFProcessingCommon::CartesianToIndexCoordinatesWithInterpixdist(expectedPoint,focalLength,interPixelDistance,principalPoint);

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinatesWithInterpixdist(resultPoint,focalLength,interPixelDistance,principalPoint);

    if (!mitk::Equal(expectedPointBackward,resultPointBackward))
    {
//      MITK_INFO << i;
//      MITK_INFO<<"expected: "<<expectedPoint;
//      MITK_INFO<<"result: "<<resultPoint;
      backwardTransformationsPointsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(backwardTransformationsPointsEqual,"Testing backward transformation with interpixeldistance");


  //Backwardtransformation test compare to original input without interpixeldistance
  bool compareToInput = true;
  for (unsigned int i=0; i<result->GetNumberOfPoints(); i++)
  {
    double* res = result->GetPoint(i);

    ToFPoint3D resultPoint;
    resultPoint[0] = res[0];
    resultPoint[1] = res[1];
    resultPoint[2] = res[2];

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinates(resultPoint,focalLengthXY,principalPoint);

    itk::Index<2> index = {{ (int) (resultPointBackward[0]+0.5), (int) (resultPointBackward[1]+0.5) }};
    float distanceBackward = 0.0;

    try
    {
      mitk::ImagePixelReadAccessor<float,2> readAccess(image, image->GetSliceData());
      distanceBackward = readAccess.GetPixelByIndex(index);
    }
    catch(mitk::Exception& e)
    {
        MITK_ERROR << "Image read exception!" << e.what();
    }

    if (!mitk::Equal(distanceBackward,(float) resultPointBackward[2]))
    {
      MITK_INFO<<"expected: " << resultPointBackward[2];
      MITK_INFO<<"result: "<< distanceBackward;
      compareToInput = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(compareToInput,"Testing backward transformation compared to original image without interpixeldistance");


  //Backwardtransformation test compare to original input with interpixeldistance
  compareToInput = true;
  for (unsigned int i=0; i<result->GetNumberOfPoints(); i++)
  {
    double* res = result->GetPoint(i);

    ToFPoint3D resultPoint;
    resultPoint[0] = res[0];
    resultPoint[1] = res[1];
    resultPoint[2] = res[2];

    ToFPoint3D resultPointBackward =
        mitk::ToFProcessingCommon::CartesianToIndexCoordinatesWithInterpixdist(resultPoint,focalLength,interPixelDistance,principalPoint);

    itk::Index<2> pixelIndex = {{ (int) (resultPointBackward[0]+0.5), (int) (resultPointBackward[1]+0.5) }};
    float distanceBackward = 0.0;
    try
    {
      mitk::ImagePixelReadAccessor<float,2> readAccess(image, image->GetSliceData());
      distanceBackward = readAccess.GetPixelByIndex(pixelIndex);
    }
    catch(mitk::Exception& e)
    {
        MITK_ERROR << "Image read exception!" << e.what();
    }

    if (!mitk::Equal(distanceBackward, (float) resultPointBackward[2]))
    {
//      MITK_INFO<<"expected: "<< image->GetPixelValueByIndex(pixelIndex);
//      MITK_INFO<<"result: "<< resultPoint;
      compareToInput = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(compareToInput,"Testing backward transformation compared to original image with interpixeldistance");

  //clean up
  delete point;
  //  expectedResult->Delete();

  MITK_TEST_END();

}


