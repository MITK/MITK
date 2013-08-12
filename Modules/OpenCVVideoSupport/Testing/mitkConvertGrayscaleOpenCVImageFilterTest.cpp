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

static void ConvertTestLoadedImage(std::string mitkImagePath)
{
  cv::Mat image = cvLoadImage(mitkImagePath.c_str());

  // directly convert the image for comparison
  cv::Mat comparisonImg;
  cv::cvtColor(image, comparisonImg, CV_RGB2GRAY, 1);

  mitk::ConvertGrayscaleOpenCVImageFilter::Pointer grayscaleFilter
      = mitk::ConvertGrayscaleOpenCVImageFilter::New();

  MITK_TEST_CONDITION_REQUIRED(grayscaleFilter->FilterImage(image), "Filtering should return true for success.");

  MITK_TEST_CONDITION_REQUIRED(image.channels() == 1, "Image must not have more than one channel after grayscale conversion.");

  MITK_TEST_CONDITION_REQUIRED(cv::countNonZero(image != comparisonImg) == 0, "All pixel values must be the same between the two converted images.");
}

/**Documentation
 *  test for the class "ConvertGrayscaleOpenCVImageFilter".
 */
int mitkConvertGrayscaleOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ConvertGrayscaleOpenCVImageFilter")

  ConvertTestLoadedImage(argv[1]);
  // always end with this!
  MITK_TEST_END();

}
