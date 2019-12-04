/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOclImageFilter.h"
#include "mitkOclFilter.h"
#include "mitkOclImage.h"

mitk::OclImageFilter::OclImageFilter()
{
  // set the filter type to default value = SHORT
  this->m_CurrentType = FILTER_SHORT;
}

mitk::OclImageFilter::~OclImageFilter()
{
}

void mitk::OclImageFilter::SetInput(mitk::OclImage::Pointer image)
{
  m_Input = image;
}

void mitk::OclImageFilter::SetInput(mitk::Image::Pointer image)
{
  m_Input = mitk::OclImage::New();
  m_Input->InitializeByMitkImage(image);

  // update current size
  MITK_DEBUG << "Current Type was: " << this->m_CurrentType;
  this->m_CurrentType = m_Input->GetBytesPerPixel() - 1;
  MITK_DEBUG << "Current Type is: " << this->m_CurrentType;
}

