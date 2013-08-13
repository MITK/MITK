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

bool CropOpenCVImageFilter::FilterImage( cv::Mat& image )
{
  if (m_CropRegion.width == 0)
  {
    MITK_ERROR("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Cropping cannot be done without setting a non-empty crop region first.";
    return false;
  }

  cv::Rect cropRegion = m_CropRegion;

  // We can try and correct too large boundaries
  if ( cropRegion.x + cropRegion.width > image.size().width)
  {
    cropRegion.width = image.size().width - cropRegion.x;
    MITK_WARN("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Changed too large roi in x direction to fit the image size.";
  }
  if ( cropRegion.y + cropRegion.height > image.size().height)
  {
    cropRegion.height = image.size().height - cropRegion.y;
    MITK_WARN("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Changed too large roi in y direction to fit the image size.";
  }

  cv::Mat buffer = image(cropRegion);
  image.release();
  image = buffer;

  return true;
}

void CropOpenCVImageFilter::SetCropRegion( cv::Rect cropRegion )
{
  // First, let's do some basic checks to make sure rectangle is inside of actual image
  if (cropRegion.x < 0)
  {
    MITK_WARN("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Changed negative x value in roi to 0.";
    cropRegion.x = 0;
  }
  if (cropRegion.y < 0)
  {
    cropRegion.y = 0;
    MITK_WARN("AbstractOpenCVImageFilter")("CropOpenCVImageFilter")
        << "Changed negative y value in roi to 0.";
  }

  // Nothing to save, throw an exception
  if ( cropRegion.height < 0 || cropRegion.width < 0 )
  {
    mitkThrow() << "Invalid boundaries supplied to USImageVideoSource::SetRegionOfInterest()";
  }

  m_CropRegion = cropRegion;
}

void CropOpenCVImageFilter::SetCropRegion( int topLeftX, int topLeftY, int bottomRightX, int bottomRightY )
{
  this->SetCropRegion( cv::Rect(topLeftX, topLeftY, bottomRightX - topLeftX, bottomRightY - topLeftY) );
}

cv::Rect CropOpenCVImageFilter::GetCropRegion( )
{
  return m_CropRegion;
}

} // namespace mitk
