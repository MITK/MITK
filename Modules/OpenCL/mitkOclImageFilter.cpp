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

#include "mitkOclImageFilter.h"
#include "mitkOclFilter.h"

#include "mitkOclImage.h"

mitk::OclImageFilter::OclImageFilter()
{
  // set the filter type to default value = SHORT
  this->m_currentType = FILTER_SHORT;
}

mitk::OclImageFilter::~OclImageFilter()
{

}

void mitk::OclImageFilter::SetInput(mitk::OclImage::Pointer _im)
{
  m_input = _im;
}

void mitk::OclImageFilter::SetInput(mitk::Image::Pointer _im)
{
  m_input = mitk::OclImage::New();
  m_input->InitializeByMitkImage(_im);

  // update current size
  MITK_INFO << "Current Type was: " << this->m_currentType;
  this->m_currentType = m_input->GetBytesPerPixel() - 1;

  MITK_INFO << "Current Type is: " << this->m_currentType;

}

