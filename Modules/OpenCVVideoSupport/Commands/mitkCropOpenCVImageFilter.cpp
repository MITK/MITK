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

#include "cv.h"

namespace mitk {

bool CropOpenCVImageFilter::filterImage( cv::Mat image )
{
  if (m_CropRegion.width <= 0)
  {
    MITK_ERROR("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Cropping cannot be done without setting a non-empty crop region first.";
    return false;
  }

  cv::Mat buffer = image(m_CropRegion);
  image.release();
  image = buffer;

  return true;
}

void CropOpenCVImageFilter::SetCropRegion( cv::Rect cropRegion )
{
  m_CropRegion = cropRegion;
}

cv::Rect CropOpenCVImageFilter::GetCropRegion( )
{
  return m_CropRegion;
}

} // namespace mitk
