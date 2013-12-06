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

#include "mitkGrabCutOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include "itkIndex.h"

#include <highgui.h>
#include <cv.h>

#include <sys/time.h>

#include "mitkOpenCVToMitkImageFilter.h"

static void GrabCutTestLoadedImage(std::string imagePath, std::string maskPath)
{
  // load test images
  cv::Mat image = cvLoadImage(imagePath.c_str());
  cv::Mat maskImage = cvLoadImage(maskPath.c_str());

  // make sure that loaded mask is a gray scale image
  cv::Mat maskImageGray;
  cv::cvtColor(maskImage, maskImageGray, CV_RGB2GRAY, 1);

  cv::Mat foregroundMask; cv::Mat foregroundPoints;
  cv::compare(maskImageGray, 200, foregroundMask, cv::CMP_GE);
  cv::findNonZero(foregroundMask, foregroundPoints);
  //cv::Rect foregroundRect = cv::boundingRect(foregroundPoints);

  cv::Rect foregroundRect(0,0,image.cols,image.rows);

  // extract foreground points from loaded mask image
  cv::compare(maskImageGray, 250, foregroundMask, cv::CMP_GE);

  cv::findNonZero(foregroundMask(foregroundRect), foregroundPoints);

  std::vector<itk::Index<2> > foregroundPointsVector;
  for ( size_t n = 0; n < foregroundPoints.total(); ++n)
  {
    itk::Index<2> index;
    index = {{foregroundPoints.at<cv::Point>(n).x, foregroundPoints.at<cv::Point>(n).y}};
    foregroundPointsVector.push_back(index);
  }

  mitk::GrabCutOpenCVImageFilter::Pointer grabCutFilter
      = mitk::GrabCutOpenCVImageFilter::New();

  cv::Mat croppedImage = image(foregroundRect);
  MITK_TEST_CONDITION(grabCutFilter->FilterImage(croppedImage), "Filtering should return true for sucess.")

  cv::Mat resultMask = grabCutFilter->GetResultMask();
  MITK_TEST_CONDITION(resultMask.empty(), "Result mask must be empty when no foreground points are set.")

  grabCutFilter->SetModelPoints(foregroundPointsVector);

  timeval t1, t2;
  gettimeofday(&t1, NULL); // start timer

  MITK_TEST_CONDITION(grabCutFilter->FilterImage(croppedImage), "Filtering should return true for sucess.")

  gettimeofday(&t2, NULL); // stop timer

  double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;  // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;      // us to ms

  MITK_INFO << "GrabCut duration: " << elapsedTime << "ms";

  cv::imwrite("/home/wintersa/output.png", grabCutFilter->GetResultMask());
}

int mitkGrabCutOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("GrabCutOpenCVImageFilter")

  MITK_TEST_CONDITION_REQUIRED(argc == 3, "Test needs three command line parameters.")

  GrabCutTestLoadedImage(argv[1], argv[2]);

  MITK_TEST_END() // always end with this!
}
