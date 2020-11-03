/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBasicCombinationOpenCVImageFilter.h"
#include "mitkConvertGrayscaleOpenCVImageFilter.h"
#include "mitkCropOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

static bool ImagesAreEqualInGray(const cv::Mat& img1, const cv::Mat& img2)
{
  cv::Mat grayImg1;
  cv::Mat grayImg2;

  cv::cvtColor(img1, grayImg1, CV_RGB2GRAY, 1);
  cv::cvtColor(img2, grayImg2, CV_RGB2GRAY, 1);

  return cv::countNonZero(grayImg1 != grayImg2) == 0;
}


static void ConvertTestLoadedImage(std::string mitkImagePath)
{
  cv::Mat image = cv::imread(mitkImagePath.c_str());
  cv::Mat compareImg = image.clone();

  mitk::BasicCombinationOpenCVImageFilter::Pointer combinationFilter
      = mitk::BasicCombinationOpenCVImageFilter::New();

  MITK_TEST_CONDITION(combinationFilter->FilterImage(image), "Filtering with empty filter list is ok.");
  MITK_TEST_CONDITION(ImagesAreEqualInGray(image, compareImg), "Image must not be changed after filtering with empty filter list.");

  mitk::ConvertGrayscaleOpenCVImageFilter::Pointer grayscaleFilter
      = mitk::ConvertGrayscaleOpenCVImageFilter::New();

  combinationFilter->PushFilter(grayscaleFilter.GetPointer());
  MITK_TEST_CONDITION(combinationFilter->FilterImage(image), "Filtering with grayscale filter should be ok.");
  MITK_TEST_CONDITION(image.channels() == 1, "Image must not have more than one channel after grayscale conversion.");

  image.release();
  image = compareImg.clone();

  mitk::CropOpenCVImageFilter::Pointer cropFilter = mitk::CropOpenCVImageFilter::New();
  combinationFilter->PushFilter(cropFilter.GetPointer());
  MITK_TEST_CONDITION( ! combinationFilter->FilterImage(image), "Filter function must return false if an filter returns false.");

  MITK_TEST_CONDITION(combinationFilter->PopFilter() == cropFilter, "Last added filter is equal to returned filter.");

  image.release();
  image = compareImg.clone();

  MITK_TEST_CONDITION(combinationFilter->FilterImage(image), "Filter function should return true again after removing incorrect filter.");

  MITK_TEST_CONDITION(combinationFilter->RemoveFilter(grayscaleFilter.GetPointer()), "Filter must be found.");

  image.release();
  image = compareImg.clone();

  MITK_TEST_CONDITION(combinationFilter->FilterImage(image), "Filter function should still return true.");

  MITK_TEST_CONDITION(ImagesAreEqualInGray(image, compareImg), "Image must not be changed after all filters were removed.");
}

/**Documentation
 *  test for the class "ConvertGrayscaleOpenCVImageFilter".
 */
int mitkBasicCombinationOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("BasicCombinationOpenCVImageFilter")

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Two parameters are needed for this test.")

  ConvertTestLoadedImage(argv[1]);

  MITK_TEST_END(); // always end with this!

}
