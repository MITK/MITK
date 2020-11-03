/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGrabCutOpenCVImageFilter.h"
#include <mitkTestingMacros.h>

#include "itkIndex.h"
#include <itksys/SystemTools.hxx>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "mitkOpenCVToMitkImageFilter.h"

static void GrabCutTestLoadedImage(std::string imagePath, std::string maskPath, std::string resultMaskPath)
{
  // load test images
  cv::Mat image = cv::imread(imagePath.c_str());
  cv::Mat maskImage = cv::imread(maskPath.c_str());
  cv::Mat resultMaskImage = cv::imread(resultMaskPath.c_str());

  // make sure that the loaded mask is a gray scale image
  cv::Mat maskImageGray;
  cv::cvtColor(maskImage, maskImageGray, CV_RGB2GRAY, 1);

  // make sure that the loaded reference image is a gray scale image
  cv::Mat resultMaskImageGray;
  cv::cvtColor(resultMaskImage, resultMaskImageGray, CV_RGB2GRAY, 1);

  // extract foreground points from loaded mask image
  cv::Mat foregroundMask, foregroundPoints;
  cv::compare(maskImageGray, 250, foregroundMask, cv::CMP_GE);
  cv::findNonZero(foregroundMask, foregroundPoints);

  // push extracted forground points into a vector of itk indices
  std::vector<itk::Index<2> > foregroundPointsVector;
  for ( size_t n = 0; n < foregroundPoints.total(); ++n)
  {
    itk::Index<2> index;
    index.SetElement(0, foregroundPoints.at<cv::Point>(n).x);
    index.SetElement(1, foregroundPoints.at<cv::Point>(n).y);
    foregroundPointsVector.push_back(index);
  }

  mitk::GrabCutOpenCVImageFilter::Pointer grabCutFilter = mitk::GrabCutOpenCVImageFilter::New();

  int currentImageId = 0;

  // test filtering with image set but no model points set
  {
    MITK_TEST_CONDITION(grabCutFilter->FilterImage(image), "Filtering should return true for sucess.")
    cv::Mat resultMask = grabCutFilter->GetResultMask();
    MITK_TEST_CONDITION(resultMask.empty(), "Result mask must be empty when no foreground points are set.")
  }

  // test filtering with very little model points set
  {
    std::vector<itk::Index<2> > littleForegroundPointsSet(foregroundPointsVector.begin(), foregroundPointsVector.begin()+3);
    grabCutFilter->SetModelPoints(littleForegroundPointsSet);
    grabCutFilter->FilterImage(image, ++currentImageId);

    cv::Mat resultMask;
    // wait up to ten seconds for the segmentation to finish
    for (unsigned int n = 0; n < 100; ++n)
    {
      if ( grabCutFilter->GetResultImageId() == currentImageId )
      {
        resultMask = grabCutFilter->GetResultMask();
        break;
      }
      itksys::SystemTools::Delay(100);
    }

    MITK_TEST_CONDITION(!resultMask.empty(),
                        "Result mask must not be empty when little ("
                        << littleForegroundPointsSet.size() <<") foreground points are set.");
  }

  // test filtering with image and model points set
  {
    grabCutFilter->SetModelPoints(foregroundPointsVector);
    MITK_TEST_CONDITION(grabCutFilter->FilterImage(image, ++currentImageId), "Filtering should return true for sucess.")

    cv::Mat resultMask;
    // wait up to ten seconds for the segmentation to finish
    for (unsigned int n = 0; n < 100; ++n)
    {
      if ( grabCutFilter->GetResultImageId() == currentImageId )
      {
        resultMask = grabCutFilter->GetResultMask();
        break;
      }
      itksys::SystemTools::Delay(100);
    }

    MITK_TEST_CONDITION( ! resultMask.empty() && cv::countNonZero(resultMask != resultMaskImageGray) == 0,
                         "Filtered image should match reference image.")

    // adding new image should still work
    MITK_TEST_CONDITION(grabCutFilter->FilterImage(image), "Adding new image should still work.")
  }


  // test filtering with using only region around model points
  // (but with really big additional width so that whole image should be used again)
  {
    grabCutFilter->SetUseOnlyRegionAroundModelPoints(image.cols);
    grabCutFilter->FilterImage(image, ++currentImageId);

    cv::Mat resultMask;
    // wait up to ten seconds for the segmentation to finish
    for (unsigned int n = 0; n < 100; ++n)
    {
      if (grabCutFilter->GetResultImageId() == currentImageId)
      {
        resultMask = grabCutFilter->GetResultMask();
        break;
      }

      itksys::SystemTools::Delay(100);
    }

    MITK_TEST_CONDITION( ! resultMask.empty() && cv::countNonZero(resultMask != resultMaskImageGray) == 0,
                         "Filtered image with really big region used should match reference image again.")
  }


  // test filtering with using only region around model points
  {
    grabCutFilter->SetUseOnlyRegionAroundModelPoints(0);
    grabCutFilter->FilterImage(image, ++currentImageId);

    cv::Mat resultMask;
    // wait up to ten seconds for the segmentation to finish
    for (unsigned int n = 0; n < 100; ++n)
    {
      if (grabCutFilter->GetResultImageId() == currentImageId)
      {
        resultMask = grabCutFilter->GetResultMask();
        break;
      }

      itksys::SystemTools::Delay(100);
    }

    cv::Mat nonPropablyBackgroundMask, modelPoints;
    cv::compare(maskImageGray, 250, nonPropablyBackgroundMask, cv::CMP_GE);
    cv::findNonZero(nonPropablyBackgroundMask, modelPoints);
    cv::Rect boundingRect = cv::boundingRect(modelPoints);

    cv::Mat compareMask(resultMask.rows, resultMask.cols, resultMask.type(), 0.0);
    resultMaskImageGray(boundingRect).copyTo(compareMask(boundingRect));

    MITK_TEST_CONDITION( ! resultMask.empty() && cv::countNonZero(resultMask != compareMask) == 0,
                         "Filtered image with region just around the model points used should match reference image again.")
  }
}

int mitkGrabCutOpenCVImageFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("GrabCutOpenCVImageFilter")

  MITK_TEST_CONDITION_REQUIRED(argc == 4, "Test needs four command line parameters.")

  GrabCutTestLoadedImage(argv[1], argv[2], argv[3]);

  MITK_TEST_END() // always end with this!
}
