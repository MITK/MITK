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

/**Documentation
 *  test for the class "itkSkeletonizationFilter".
 */
int mitkTbssSkeletonizationTest(int argc , char* argv[])
{

  MITK_TEST_BEGIN("TbssSkeletonizationTest");


  MITK_INFO << "argv[0]: " << argv[0];
  MITK_INFO << "argv[1]: " << argv[1];
  MITK_INFO << "argv[2]: " << argv[2];


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

  MITK_TEST_END();
}
