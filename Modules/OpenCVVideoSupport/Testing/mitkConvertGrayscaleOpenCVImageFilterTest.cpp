/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkConvertGrayscaleOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

static void ConvertTestLoadedImage(std::string mitkImagePath, std::string mitkGrayscaleImagePath)
{
  cv::Mat image = cv::imread(mitkImagePath.c_str());
  cv::Mat compareImg = cv::imread(mitkGrayscaleImagePath.c_str());

  // directly convert the image for comparison
  cv::Mat comparisonImg;
  cv::cvtColor(compareImg, comparisonImg, CV_RGB2GRAY, 1);

  mitk::ConvertGrayscaleOpenCVImageFilter::Pointer grayscaleFilter
      = mitk::ConvertGrayscaleOpenCVImageFilter::New();

  MITK_TEST_CONDITION(grayscaleFilter->FilterImage(image), "Filtering should return true for success.");

  MITK_TEST_CONDITION(image.channels() == 1, "Image must not have more than one channel after grayscale conversion.");

  MITK_TEST_CONDITION(cv::countNonZero(image != comparisonImg) == 0, "All pixel values must be the same between the two converted images.");

  MITK_TEST_CONDITION_REQUIRED(grayscaleFilter->FilterImage(image), "Image conversion should be no problem if image is a grayscale image already.")
}

/**Documentation
 *  test for the class "ConvertGrayscaleOpenCVImageFilter".
 */
int mitkConvertGrayscaleOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ConvertGrayscaleOpenCVImageFilter")

  MITK_TEST_CONDITION_REQUIRED(argc > 2, "At least three parameters needed for this test.")

  ConvertTestLoadedImage(argv[1], argv[2]);

  MITK_TEST_END(); // always end with this!

}
