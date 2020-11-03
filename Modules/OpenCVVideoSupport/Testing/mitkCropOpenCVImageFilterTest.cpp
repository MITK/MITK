/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

static void CropTestLoadedImage(std::string mitkImagePath, std::string mitkCroppedImagePath)
{
  cv::Mat image = cv::imread(mitkImagePath.c_str());
  cv::Mat croppedImage = cv::imread(mitkCroppedImagePath.c_str());

  MITK_INFO << mitkImagePath.c_str();
  MITK_INFO << mitkCroppedImagePath.c_str();

  mitk::CropOpenCVImageFilter::Pointer cropFilter = mitk::CropOpenCVImageFilter::New();

  // try to crop without setting a region of interest
  cv::Mat testImage = image.clone();
  MITK_TEST_CONDITION( ! cropFilter->FilterImage(testImage), "Filter function must return false if no region of interest is set.");
  MITK_TEST_CONDITION(ImagesAreEqualInGray(testImage, image), "Image should not be changed yet.");

  // set region of interst now and then try to crop again
  cv::Rect roi = cv::Rect(0,0, testImage.cols, testImage.rows);
  cropFilter->SetCropRegion(roi);
  MITK_TEST_CONDITION(cropFilter->FilterImage(testImage), "Filter function should return successfully.");
  MITK_TEST_CONDITION(ImagesAreEqualInGray(testImage, image), "Image should not be changed if cropping with a roi of the whole image.");

  // test if filter corrects negative roi position
  cv::Rect roiWrong = cv::Rect(-1,-1, 2, 2);
  roi = cv::Rect(0,0,2,2);
  cropFilter->SetCropRegion(roiWrong);
  MITK_TEST_CONDITION(cropFilter->FilterImage(testImage), "Filter function should return successfully.");
  MITK_TEST_CONDITION(ImagesAreEqualInGray(testImage, image(roi)), "Image should be equal to directly cropped image whith correct roi.");

  // test whith "normal" roi
  testImage = image.clone();
  roi = cv::Rect( 150,100,100,100 );
  cropFilter->SetCropRegion(roi);
  MITK_TEST_CONDITION(cropFilter->FilterImage(testImage), "Filter function should return successfully.");
  MITK_TEST_CONDITION(ImagesAreEqualInGray(testImage, croppedImage), "Image should be equal to cropped image (loaded from data directory).");

  // test with not correctable roi
  roiWrong = cv::Rect( 5,5,-1,-1 );
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, cropFilter->SetCropRegion(roiWrong));

  // test with rois where the top left corner is outside the image boundaries
  roiWrong = cv::Rect( testImage.cols,0,1,1 );
  cropFilter->SetCropRegion(roiWrong);
  MITK_TEST_CONDITION(!cropFilter->FilterImage(testImage),
                      "Filter function should return unsuccessfully if top left corner is outside image boundary (cols).");
  roiWrong = cv::Rect( 0,testImage.rows,1,1 );
  cropFilter->SetCropRegion(roiWrong);
  MITK_TEST_CONDITION(!cropFilter->FilterImage(testImage),
                       "Filter function should return unsuccessfully if top left corner is outside image boundary (rows).");
  roiWrong = cv::Rect( testImage.cols,testImage.rows,1,1 );
  cropFilter->SetCropRegion(roiWrong);
  MITK_TEST_CONDITION(!cropFilter->FilterImage(testImage),
                       "Filter function should return unsuccessfully if top left corner is outside image boundary (cols+rows).");
}

/**Documentation
 *  test for the class "CropOpenCVImageFilter".
 */
int mitkCropOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("CropOpenCVImageFilter")

  MITK_TEST_CONDITION_REQUIRED(argc > 2, "At least three parameters needed for this test.");

  CropTestLoadedImage(argv[1], argv[2]);

  MITK_TEST_END(); // always end with this!

}
