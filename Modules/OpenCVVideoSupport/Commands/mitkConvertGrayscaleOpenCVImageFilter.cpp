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

#include "cv.h"

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
