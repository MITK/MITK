/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAbstractOpenCVImageFilter.h"

int mitk::AbstractOpenCVImageFilter::INVALID_IMAGE_ID = -1;

mitk::AbstractOpenCVImageFilter::AbstractOpenCVImageFilter()
{
}

bool mitk::AbstractOpenCVImageFilter::FilterImage( cv::Mat& image, int id )
{
  if (id < INVALID_IMAGE_ID) { id = INVALID_IMAGE_ID; }

  m_CurrentImageId = id;
  return this->OnFilterImage(image);
}

int mitk::AbstractOpenCVImageFilter::GetCurrentImageId()
{
  return m_CurrentImageId;
}
