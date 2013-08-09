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

#include "mitkCropOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include <highgui.h>
#include <cv.h>

static void CropTestLoadedImage(std::string mitkImagePath)
{
  cv::Mat image = cvLoadImage(mitkImagePath.c_str());
  cv::Mat compareImg = image.clone();

  mitk::CropOpenCVImageFilter::Pointer cropFilter = mitk::CropOpenCVImageFilter::New();

  // try to crop without setting a region of interest
  MITK_TEST_CONDITION_REQUIRED( ! cropFilter->FilterImage(image), "Filter function must return false if no region of interest is set.");
  MITK_TEST_CONDITION_REQUIRED(cv::countNonZero(image != compareImg), "Image should not be changed yet.");

  // set region of interst now and then try to crop again
  cv::Rect roi = cv::Rect(0,0, image.cols, image.rows);
  cropFilter->SetCropRegion(roi);
  MITK_TEST_CONDITION_REQUIRED(cropFilter->FilterImage(image), "Filter function should return successfully.");
  MITK_TEST_CONDITION_REQUIRED(cv::countNonZero(image != compareImg), "Image should not be changed if cropping with a roi of the whole image.");

  // test if filter corrects negative roi position
  cv::Rect roiWrong = cv::Rect(-1,-1, 2, 2);
  roi = cv::Rect(0,0,2,2);
  cropFilter->SetCropRegion(roiWrong);
  MITK_TEST_CONDITION_REQUIRED(cropFilter->FilterImage(image), "Filter function should return successfully.");
  MITK_TEST_CONDITION_REQUIRED(cv::countNonZero(image != compareImg(roi)), "Image should be equal to directly cropped image whith correct roi.");

  // test whith "normal" roi
  roi = cv::Rect( 5,5,10,10 );
  cropFilter->SetCropRegion(roi);
  MITK_TEST_CONDITION_REQUIRED(cropFilter->FilterImage(image), "Filter function should return successfully.");
  MITK_TEST_CONDITION_REQUIRED(cv::countNonZero(image != compareImg(roi)), "Image should be equal to directly cropped image.");

  // test with not correctable roi
  roiWrong = cv::Rect( 5,5,-1,-1 );
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, cropFilter->SetCropRegion(roiWrong));
}

/**Documentation
 *  test for the class "CropOpenCVImageFilter".
 */
int mitkCropOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("CropOpenCVImageFilter")

  CropTestLoadedImage(argv[1]);
  // always end with this!
  MITK_TEST_END();

}
