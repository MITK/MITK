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
#include <itkImageFileReader.h>
#include <itkSkeletonizationFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkDistanceMapFilter.h>
#include <itkProjectionFilter.h>
#include <itkTestingComparisonImageFilter.h>

/**Documentation
 *  test for the class "itkSkeletonizationFilter".
 */
int mitkTbssSkeletonizationTest(int, char* argv[])
{

  MITK_TEST_BEGIN("TbssSkeletonizationTest");


  // Load images
  typedef itk::Image<float, 3> FloatImageType;
  typedef itk::ImageFileReader<FloatImageType> ImageReaderType;

  ImageReaderType::Pointer reader = ImageReaderType::New();
  reader->SetFileName(argv[1]);

  // Initialize skeletonization filter and feed it with the mean_FA.nii.gz

  typedef itk::SkeletonizationFilter<FloatImageType, FloatImageType> SkeletonisationFilterType;
  SkeletonisationFilterType::Pointer skeletonizer = SkeletonisationFilterType::New();
  skeletonizer->SetInput(reader->GetOutput());
  skeletonizer->Update();


  // Create a new image so the skeletonizaton won't be performed every time the skeleton is needed.
  FloatImageType::Pointer skeleton = skeletonizer->GetOutput();

  // Check whether the skeleton is correct
  reader->SetFileName(argv[2]);
  reader->Update();

  FloatImageType::Pointer controlSkeleton = reader->GetOutput();


  // Convert itk images to mitk images and use the mitk::Equal method to compare the result with the reference skeleton.

  mitk::Image::Pointer mitkSkeleton = mitk::Image::New();
  mitkSkeleton->InitializeByItk(skeleton.GetPointer());

  mitk::Image::Pointer mitkRefSkeleton = mitk::Image::New();
  mitkRefSkeleton->InitializeByItk(controlSkeleton.GetPointer());


  MITK_TEST_CONDITION(mitk::Equal(*mitkSkeleton, *mitkRefSkeleton, 0.001, true), "Check correctness of the skeleton");


  // Test the projection filter

  typedef itk::CovariantVector<int,3> VectorType;
  typedef itk::Image<VectorType, 3> DirectionImageType;

  // Retrieve direction image needed later by the projection filter
  DirectionImageType::Pointer directionImg = skeletonizer->GetVectorImage();

  // Define a distance map filter that creates a distance map to limit the projection search
  typedef itk::DistanceMapFilter<FloatImageType, FloatImageType> DistanceMapFilterType;

  DistanceMapFilterType::Pointer distanceMapFilter = DistanceMapFilterType::New();
  distanceMapFilter->SetInput(controlSkeleton); //use controlSkeleton to prevent updating the skeletonfilter, which is time consuming
  distanceMapFilter->Update();

  FloatImageType::Pointer distanceMap = distanceMapFilter->GetOutput();




  // Threshold the skeleton on FA=0.2 to create a binary skeleton mask
  typedef itk::Image<char, 3> CharImageType;
  typedef itk::BinaryThresholdImageFilter<FloatImageType, CharImageType> ThresholdFilterType;
  ThresholdFilterType::Pointer thresholder = ThresholdFilterType::New();
  thresholder->SetInput(controlSkeleton); //use controlSkeleton to prevent updating the skeletonfilter, which is time consuming
  thresholder->SetLowerThreshold(0.2);
  thresholder->SetUpperThreshold(std::numeric_limits<float>::max());
  thresholder->SetOutsideValue(0);
  thresholder->SetInsideValue(1);
  thresholder->Update();


  CharImageType::Pointer thresholdedImg = thresholder->GetOutput();


  // Load the cingulum mask that defines the region where a tubular structure must be searched for
  typedef itk::ImageFileReader< CharImageType > CharReaderType;
  CharReaderType::Pointer charReader = CharReaderType::New();
  charReader->SetFileName(argv[3]);
  charReader->Update();
  CharImageType::Pointer cingulum = charReader->GetOutput();



  // Define projection filter
  typedef itk::ProjectionFilter ProjectionFilterType;

  // Read the 4d test image containing the registered FA data of one subject
  typedef itk::Image<float, 4> Float4DImageType;
  typedef itk::ImageFileReader<Float4DImageType> ImageReader4DType;



  ImageReader4DType::Pointer reader4d = ImageReader4DType::New();
  reader4d->SetFileName(argv[4]);
  reader4d->Update();



  ProjectionFilterType::Pointer projectionFilter = ProjectionFilterType::New();
  projectionFilter->SetDistanceMap(distanceMap);
  projectionFilter->SetDirections(directionImg);
  projectionFilter->SetAllFA(reader4d->GetOutput());
  projectionFilter->SetTube(cingulum);
  projectionFilter->SetSkeleton(thresholdedImg);
  projectionFilter->Project();



  Float4DImageType::Pointer projected = projectionFilter->GetProjections();



  // Open control projection image
  reader4d->SetFileName(argv[5]);
  reader4d->Update();
  Float4DImageType::Pointer controlProjection = reader4d->GetOutput();

  // control dimensions
  Float4DImageType::SizeType pSize = projected->GetLargestPossibleRegion().GetSize();
  Float4DImageType::SizeType pControlSize = controlProjection->GetLargestPossibleRegion().GetSize();

  typedef itk::Testing::ComparisonImageFilter<Float4DImageType, Float4DImageType> ComparisonFilterType;
  ComparisonFilterType::Pointer comparisonFilter = ComparisonFilterType::New();
  comparisonFilter->SetTestInput(projected);
  comparisonFilter->SetValidInput(controlProjection);
  comparisonFilter->Update();
  float diff = comparisonFilter->GetTotalDifference();


  MITK_TEST_CONDITION(pSize == pControlSize, "Size of projection image and control projection image are the same");

  MITK_TEST_CONDITION(diff < 0.001, "Check correctness of the projections");


  MITK_TEST_END();
}
