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

/**Documentation
 *  test for the class "itkSkeletonizationFilter".
 */
int mitkTbssSkeletonizationTest(int argc , char* argv[])
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


  // Check dimensions
  FloatImageType::SizeType size = skeleton->GetLargestPossibleRegion().GetSize();
  FloatImageType::SizeType controlSize = controlSkeleton->GetLargestPossibleRegion().GetSize();


  MITK_TEST_CONDITION(size == controlSize, "Size of skeleton and control skeleton are the same");


  // Loop trough both images and check if all values are the same
  bool same = true;
  for(int x=0; x<size[0]; x++)
  {
    for(int y=0; y<size[1]; y++)
    {
      for(int z=0; z<size[2]; z++)
      {
        itk::Index<3> ix = {x,y,z};
        if(skeleton->GetPixel(ix) != controlSkeleton->GetPixel(ix))
        {
          same = false;
        }
      }
    }
  }

  MITK_TEST_CONDITION(same, "Check correctness of the skeleton");


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



  MITK_TEST_CONDITION(pSize == pControlSize, "Size of projection image and control projection image are the same");


  // Check all pixel values of the projection
  same = true;

  for(int x=0; x<pSize[0]; x++)
  {
    for(int y=0; y<pSize[1]; y++)
    {
      for(int z=0; z<pSize[2]; z++)
      {

        for(int t=0; t<pSize[3]; t++)
        {
          itk::Index<4> ix = {x,y,z,t};
          if(projected->GetPixel(ix) != controlProjection->GetPixel(ix))
          {
            same = false;
          }
        }
      }
    }
  }


  MITK_TEST_CONDITION(same, "Check correctness of the projections");


  MITK_TEST_END();
}
