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

#include "mitkConvertGrayscaleOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include <highgui.h>
#include <cv.h>

static void ConvertTestLoadedImage(std::string mitkImagePath, std::string mitkGrayscaleImagePath)
{
  cv::Mat image = cvLoadImage(mitkImagePath.c_str());
  cv::Mat compareImg = cvLoadImage(mitkGrayscaleImagePath.c_str());

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
