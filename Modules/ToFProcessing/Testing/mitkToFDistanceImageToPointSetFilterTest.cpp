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
#include <mitkToFDistanceImageToPointSetFilter.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>

#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>
#include <mitkToFProcessingCommon.h>
#include <mitkToFTestingCommon.h>
#include <mitkNumericTypes.h>
#include <mitkImagePixelReadAccessor.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vtkPolyData.h>

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

std::vector<itk::Index<3> > CreateVectorPointSet()
{
  std::vector<itk::Index<3> > subSet = std::vector<itk::Index<3> >();
  itk::Index<3> point;
  point[0] = 10;
  point[1] = 20;
  point[2] = 0;
  subSet.push_back(point);
  point[0] = 100;
  point[1] = 150;
  point[2] = 0;
  subSet.push_back(point);
  point[0] = 110;
  point[1] = 30;
  point[2] = 0;
  subSet.push_back(point);
  point[0] = 40;
  point[1] = 200;
  point[2] = 0;
  subSet.push_back(point);
  return subSet;
}

// Create image with pixelValue in every pixel except for the pixels in subSet, which get successively the values of distances
inline static mitk::Image::Pointer CreateTestImageWithPointSet(float pixelValue, unsigned int dimX, unsigned int dimY, mitk::PointSet::Pointer subSet)
{
  typedef itk::Image<float,2> ItkImageType2D;
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
  std::vector<float> distances;
  distances.push_back(50);
  distances.push_back(500);
  distances.push_back(2050);
  distances.push_back(300);
  // set the pixel values for the subset
  for(int i=0; i<subSet->GetSize(); i++)
  {
    mitk::Point3D point = subSet->GetPoint(i);
    ItkImageType2D::IndexType index;
    index[0] = point[0];
    index[1] = point[1];
    float distance = distances.at(i);
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
  //initialize intrinsic parameters with some arbitrary values
  mitk::ToFProcessingCommon::ToFPoint2D interPixelDistance;
  interPixelDistance[0] = 0.04564;
  interPixelDistance[1] = 0.0451564;

  mitk::ToFProcessingCommon::ToFScalarType focalLengthX = 295.78960;
  mitk::ToFProcessingCommon::ToFScalarType focalLengthY = 296.348535;

  mitk::ToFProcessingCommon::ToFScalarType focalLength = (focalLengthX*interPixelDistance[0]+focalLengthY*interPixelDistance[1])/2.0;
  mitk::ToFProcessingCommon::ToFScalarType k1=-0.36,k2=-0.14,p1=0.001,p2=-0.00;

  mitk::ToFProcessingCommon::ToFPoint2D principalPoint;
  principalPoint[0] = 103.576546;
  principalPoint[1] = 100.1532;

  mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
  cameraIntrinsics->SetFocalLength(focalLengthX,focalLengthY);
  cameraIntrinsics->SetPrincipalPoint(principalPoint[0],principalPoint[1]);
  cameraIntrinsics->SetDistorsionCoeffs(k1,k2,p1,p2);

  // test SetCameraIntrinsics()
  filter->SetCameraIntrinsics(cameraIntrinsics);
  MITK_TEST_CONDITION_REQUIRED((focalLengthX==filter->GetCameraIntrinsics()->GetFocalLengthX()),"Testing SetCameraIntrinsics with focalLength");
  mitk::ToFProcessingCommon::ToFPoint2D pp;
  pp[0] = filter->GetCameraIntrinsics()->GetPrincipalPointX();
  pp[1] = filter->GetCameraIntrinsics()->GetPrincipalPointY();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(principalPoint,pp),"Testing SetCameraIntrinsics with principalPoint()");

  // test SetInterPixelDistance()
  filter->SetInterPixelDistance(interPixelDistance);
  mitk::ToFProcessingCommon::ToFPoint2D ipD = filter->GetInterPixelDistance();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(ipD,interPixelDistance),"Testing Set/GetInterPixelDistance()");

  // test SetReconstructionMode()
  filter->SetReconstructionMode(false);
  MITK_TEST_CONDITION_REQUIRED(filter->GetReconstructionMode() == false,"Testing Set/GetReconstructionMode()");

  // test Set/GetInput()
  filter->SetInput(image);
  MITK_TEST_CONDITION_REQUIRED((image==filter->GetInput()),"Testing Set/GetInput()");


  // test filter without subset (without using the interpixeldistance)
  MITK_INFO<<"Test filter without subset without using the interpixeldistance";
  filter->SetReconstructionMode(true);
  mitk::PointSet::Pointer expectedResult = mitk::PointSet::New();
  unsigned int counter = 0;
  mitk::ImagePixelReadAccessor<float,2> imageAcces(image, image->GetSliceData(0));
  for (unsigned int j=0; j<dimY; j++)
  {
    for (unsigned int i=0; i<dimX; i++)
    {
      itk::Index<2> index;
      index[0] = i;
      index[1] = j;
      float distance = imageAcces.GetPixelByIndex(index);
      mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLengthX,focalLengthY,principalPoint[0],principalPoint[1]);
      expectedResult->InsertPoint(counter,coordinate);
      counter++;
    }
  }
  filter->Update();
  mitk::PointSet::Pointer result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(expectedResult,result),"Testing filter without subset");

  // compare filter result with ToFDistanceImageToSurfaceFilter
  MITK_INFO<<"Compare filter result with ToFDistanceImageToSurfaceFilter";
  mitk::ToFDistanceImageToSurfaceFilter::Pointer surfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();
  surfaceFilter->SetInput(image);
  surfaceFilter->SetInterPixelDistance(interPixelDistance);
  surfaceFilter->SetCameraIntrinsics(cameraIntrinsics);
  surfaceFilter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::WithOutInterPixelDistance);
  MITK_TEST_CONDITION_REQUIRED(filter->GetReconstructionMode() == mitk::ToFDistanceImageToSurfaceFilter::WithOutInterPixelDistance,"Testing Set/GetReconstructionMode()");
  mitk::Surface::Pointer surface = surfaceFilter->GetOutput();
  surface->Update();
  // create point set from surface
  mitk::PointSet::Pointer pointSet = mitk::ToFTestingCommon::VtkPolyDataToMitkPointSet(surface->GetVtkPolyData());
  //compare pointset against ground truth
  MITK_TEST_CONDITION_REQUIRED((pointSet->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(pointSet,result),"Compare with surface points");

  // test filter without subset (with using the interpixeldistance)
  MITK_INFO<<"Test filter without subset with using the interpixeldistance";
  filter->Modified();
  filter->SetReconstructionMode(false);
  expectedResult = mitk::PointSet::New();
  counter = 0;
  for (unsigned int j=0; j<dimY; j++)
  {
    for (unsigned int i=0; i<dimX; i++)
    {
      itk::Index<2> index;
      index[0] = i;
      index[1] = j;
      float distance = imageAcces.GetPixelByIndex(index);
      mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(i,j,distance,focalLength,interPixelDistance,principalPoint);
      expectedResult->InsertPoint(counter,coordinate);
      counter++;
    }
  }
  filter->Update();
  result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(expectedResult,result),"Testing filter without subset");

  // compare filter result with ToFDistanceImageToSurfaceFilter
  MITK_INFO<<"Compare filter result with ToFDistanceImageToSurfaceFilter";
  surfaceFilter = mitk::ToFDistanceImageToSurfaceFilter::New();
  surfaceFilter->SetInput(image);
  surfaceFilter->SetInterPixelDistance(interPixelDistance);
  surfaceFilter->SetCameraIntrinsics(cameraIntrinsics);
  surfaceFilter->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance);
  MITK_TEST_CONDITION_REQUIRED(surfaceFilter->GetReconstructionMode() == mitk::ToFDistanceImageToSurfaceFilter::WithInterPixelDistance,"Testing Set/GetReconstructionMode()");
  surface = surfaceFilter->GetOutput();
  surface->Update();
  // create point set from surface
  pointSet = mitk::ToFTestingCommon::VtkPolyDataToMitkPointSet(surface->GetVtkPolyData());
  //compare against ground truth
  MITK_TEST_CONDITION_REQUIRED((pointSet->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(pointSet,result),"Compare with surface points");


  // test filter with subset (without using the interpixeldistance)
  MITK_INFO<<"Test filter with subset without using the interpixeldistance";
  filter = mitk::ToFDistanceImageToPointSetFilter::New();
  filter->SetInput(image);
  filter->SetInterPixelDistance(interPixelDistance);
  filter->SetCameraIntrinsics(cameraIntrinsics);
  filter->SetReconstructionMode(true);
  expectedResult = mitk::PointSet::New();
  counter = 0;
  for(int i=0; i<subSet->GetSize(); i++)
  {
    mitk::Point3D point = subSet->GetPoint(i);
    itk::Index<2> index;
    index[0] = point[0];
    index[1] = point[1];
    float distance = imageAcces.GetPixelByIndex(index);
    mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(point[0],point[1],
                                                                                      distance,focalLengthX,focalLengthY,principalPoint[0],principalPoint[1]);
    expectedResult->InsertPoint(counter,coordinate);
    counter++;
  }
  filter->SetSubset(subSet);
  filter->Modified();
  filter->Update();
  result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(expectedResult,result),"Testing filter with subset");


  // test filter with subset (with using the interpixeldistance)
  MITK_INFO<<"Test filter with subset with using the interpixeldistance";
  filter = mitk::ToFDistanceImageToPointSetFilter::New();
  filter->SetInput(image);
  filter->SetInterPixelDistance(interPixelDistance);
  filter->SetCameraIntrinsics(cameraIntrinsics);
  filter->SetReconstructionMode(false);
  expectedResult = mitk::PointSet::New();
  counter = 0;
  for(int i=0; i<subSet->GetSize(); i++)
  {
    mitk::Point3D point = subSet->GetPoint(i);
    itk::Index<2> index;
    index[0] = point[0];
    index[1] = point[1];
    float distance = imageAcces.GetPixelByIndex(index);
    mitk::Point3D coordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(point[0],point[1],
                                                                                      distance,focalLength,interPixelDistance,principalPoint);
    expectedResult->InsertPoint(counter,coordinate);
    counter++;
  }
  filter->SetSubset(subSet);
  filter->Modified();
  filter->Update();
  result = filter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED((expectedResult->GetSize()==result->GetSize()),"Test if point set size is equal");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFTestingCommon::PointSetsEqual(expectedResult,result),"Testing filter with subset");

  // Test case to reproduce and check fix of bug 13933.
  std::vector<itk::Index<3> > vecSubset = CreateVectorPointSet();
  filter = mitk::ToFDistanceImageToPointSetFilter::New();
  try
  {
    filter->SetSubset(vecSubset);
  }
  catch (...)
  {
    MITK_TEST_CONDITION_REQUIRED(false, "Caught an exception while setting point subset!");
  }

  MITK_TEST_END();
}
