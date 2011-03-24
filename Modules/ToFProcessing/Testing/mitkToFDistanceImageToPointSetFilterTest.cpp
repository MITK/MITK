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
#include <mitkToFDistanceImageToPointSetFilter.h>

#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkToFProcessingCommon.h>
#include <mitkVector.h>
#include <mitkPinholeCameraModel.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

/**Documentation
*  test for the class "ToFDistanceImageToPointSetFilter".
*/
mitk::PointSet::Pointer CreateTestPointSet()
{
  mitk::PointSet::Pointer subSet = mitk::PointSet::New();
  mitk::Point3D point;
  point[0] = 10;
  point[1] = 20;
  point[2] = 0;
  subSet->InsertPoint(0,point);
  point[0] = 100;
  point[1] = 150;
  point[2] = 0;
  subSet->InsertPoint(1,point);
  point[0] = 110;
  point[1] = 30;
  point[2] = 0;
  subSet->InsertPoint(2,point);
  point[0] = 40;
  point[1] = 200;
  point[2] = 0;
  subSet->InsertPoint(3,point);
  return subSet;
}

inline static mitk::Image::Pointer CreateTestImageWithPointSet(mitk::ScalarType pixelValue, unsigned int dimX, unsigned int dimY, mitk::PointSet::Pointer subSet)
{
  typedef itk::Image<mitk::ScalarType,2> ItkImageType2D;
  typedef itk::ImageRegionIterator<ItkImageType2D> ItkImageRegionIteratorType2D;

  ItkImageType2D::Pointer image = ItkImageType2D::New();
  ItkImageType2D::IndexType start;
  start[0] = 0;
  start[1] = 0;
  ItkImageType2D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;
  ItkImageType2D::RegionType region;
  region.SetSize(size);
  region.SetIndex( start);
  ItkImageType2D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  image->SetRegions( region );
  image->SetSpacing ( spacing );
  image->Allocate();

  //Obtaining image data from ToF camera//

  //Correlate inten values to PixelIndex//
  ItkImageRegionIteratorType2D imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while (!imageIterator.IsAtEnd())
  {
    imageIterator.Set(pixelValue);
    ++imageIterator;
  }
  // distances varying from pixelValue
  std::vector<mitk::ScalarType> distances;
  distances.push_back(50);
  distances.push_back(500);
  distances.push_back(2050);
  distances.push_back(300);
  // set the pixel values for the subset
  for (unsigned int i=0; i<subSet->GetSize(); i++)
  {
    mitk::Point3D point = subSet->GetPoint(i);
    ItkImageType2D::IndexType index;
    index[0] = point[0];
    index[1] = point[1];
    mitk::ScalarType distance = distances.at(i);
    image->SetPixel(index,distance);
  }
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::CastToMitkImage(image,mitkImage);
  return mitkImage;
}

int mitkToFDistanceImageToPointSetFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFDistanceImageToPointSetFilter");

  mitk::ToFDistanceImageToPointSetFilter::Pointer filter = mitk::ToFDistanceImageToPointSetFilter::New();
  //create test sub set
  MITK_INFO<<"Create test pointset";
  mitk::PointSet::Pointer subSet = CreateTestPointSet();
  //create test image
  unsigned int dimX = 204;
  unsigned int dimY = 204;
  MITK_INFO<<"Create test image";
  mitk::Image::Pointer image = CreateTestImageWithPointSet(1000.0f,dimX,dimY,subSet);
  //initialize intrinsic parameters
  mitk::ScalarType focalLength = 10;
  mitk::Point2D interPixelDistance;
  interPixelDistance[0] = 0.05;
  interPixelDistance[1] = 0.05;
  mitk::Point2D principalPoint;
  principalPoint[0] = 100;
  principalPoint[1] = 100;

  mitk::PinholeCameraModel::Pointer camera = mitk::PinholeCameraModel::New();
  camera->SetFocalLength(focalLength);
  camera->SetPrincipalPoint(principalPoint);
  camera->SetInterPixelDistance(interPixelDistance);


  // test setter/getter of intrinsic parameters
  filter->SetCameraModel(camera);
  MITK_TEST_CONDITION_REQUIRED((focalLength==filter->GetCameraModel()->GetFocalLength()),"Testing Set/GetFocalLength()");
//  filter->SetInterPixelDistance(interPixelDistance);
  MITK_TEST_CONDITION_REQUIRED((interPixelDistance==filter->GetCameraModel()->GetInterPixelDistance()),"Testing Set/GetInterPixelDistance()");
//  filter->SetPrincipalPoint(principalPoint);
  mitk::Point2D pp = filter->GetCameraModel()->GetPrincipalPoint();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing Set/GetPrincipalPoint()");
  // test SetIntrinsicParameters()
  mitk::PinholeCameraModel::Pointer cameraIntrinics = mitk::PinholeCameraModel::New();
  mitk::Point2D imageArea;
  imageArea[0] = dimX*interPixelDistance[0];
  imageArea[1] = dimY*interPixelDistance[1];
  cameraIntrinics->SetImageArea(imageArea);
  cameraIntrinics->SetFocalLength(10);
  mitk::Point2D pixelSize;
  pixelSize[0] = interPixelDistance[0];
  pixelSize[1] = interPixelDistance[1];
  cameraIntrinics->SetInterPixelDistance(pixelSize);
  mitk::Point2D imageShift;
  imageShift[0] = (principalPoint[0] - dimX/2)*interPixelDistance[0];
  imageShift[1] = (principalPoint[1] - dimX/2)*interPixelDistance[1];
  cameraIntrinics->SetImageShift(imageShift);
  filter->SetCameraModel(cameraIntrinics);
  MITK_TEST_CONDITION_REQUIRED((focalLength==filter->GetCameraModel()->GetFocalLength()),"Testing SetIntrinsicParameters() - focal length");
  MITK_TEST_CONDITION_REQUIRED((interPixelDistance==filter->GetCameraModel()->GetInterPixelDistance()),"Testing SetIntrinsicParameters() - inter pixel distance");
  pp = camera->GetPrincipalPoint();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing SetIntrinsicParameters() - principal point");

  // test Set/GetInput()
  filter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED((image==filter->GetInput()),"Testing Set/GetInput()");

  // test filter without subset
  MITK_INFO<<"Test filter without subset";
  mitk::PointSet::Pointer expectedResult = mitk::PointSet::New();
  unsigned int counter = 0;
  for (unsigned int i=0; i<dimX; i++)
  {
    for (unsigned int j=0; j<dimY; j++)
    {
      mitk::Index3D index;
      index[0] = i;
      index[1] = j;
      index[2] = 0;
      mitk::ScalarType distance = image->GetPixelValueByIndex(index);
      mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,cameraIntrinics->GetFocalLength(),cameraIntrinics->GetInterPixelDistance(),
                                                                                        cameraIntrinics->GetPrincipalPoint());
      expectedResult->InsertPoint(counter,coordinate);
      counter++;
    }
  }
  filter->Update();
  mitk::PointSet::Pointer result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  bool pointSetsEqual = true;
  for (unsigned int i=0; i<expectedResult->GetSize(); i++)
  {
    mitk::Point3D expectedPoint = expectedResult->GetPoint(i);
    mitk::Point3D resultPoint = result->GetPoint(i);
    if (!mitk::Equal(expectedPoint,resultPoint))
    {
      MITK_INFO << "erwartet " << expectedPoint;
      MITK_INFO << "result " << resultPoint;
      pointSetsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(pointSetsEqual,"Testing filter without subset");

  // test filter with subset
  MITK_INFO<<"Test filter with subset";
  filter = mitk::ToFDistanceImageToPointSetFilter::New();
  filter->SetInput(image);
  filter->SetCameraModel(cameraIntrinics);
  expectedResult = mitk::PointSet::New();
  counter = 0;
  for (unsigned int i=0; i<subSet->GetSize(); i++)
  {
    mitk::Point3D point = subSet->GetPoint(i);
    mitk::Index3D index;
    index[0] = point[0];
    index[1] = point[1];
    index[2] = 0;
    mitk::ScalarType distance = image->GetPixelValueByIndex(index);
    mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(point[0],point[1],
                                                                                      distance,cameraIntrinics->GetFocalLength(),cameraIntrinics->GetInterPixelDistance(),
                                                                                      cameraIntrinics->GetPrincipalPoint());
    expectedResult->InsertPoint(counter,coordinate);
    counter++;
  }
  filter->SetSubset(subSet);
  filter->Modified();
  filter->Update();
  result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  pointSetsEqual = true;
  for (unsigned int i=0; i<expectedResult->GetSize(); i++)
  {
    mitk::Point3D expectedPoint = expectedResult->GetPoint(i);
    mitk::Point3D resultPoint = result->GetPoint(i);
    if (!mitk::Equal(expectedPoint,resultPoint))
    {
      pointSetsEqual = false;
    }
  }
  MITK_TEST_CONDITION_REQUIRED(pointSetsEqual,"Testing filter without subset");
  MITK_TEST_END();

}

