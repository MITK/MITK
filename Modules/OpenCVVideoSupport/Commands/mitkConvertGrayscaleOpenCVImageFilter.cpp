/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkConvertGrayscaleOpenCVImageFilter.h"

#include <opencv2/imgproc.hpp>

namespace mitk {

bool ConvertGrayscaleOpenCVImageFilter::OnFilterImage( cv::Mat& image )
{
  // there is nothing to do if the image is grayscale already
  if (image.channels() == 1) { return true; }

  cv::Mat buffer;

  cv::cvtColor(image, buffer, CV_RGB2GRAY, 1);

  // content of buffer should now be the content of image
  buffer.copyTo(image);

  return true;
}
} // namespace mitk
