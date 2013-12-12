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
