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

#include "mitkOtsuSegmentationFilter.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"

namespace mitk {

OtsuSegmentationFilter::OtsuSegmentationFilter()
: m_NumberOfThresholds(2)
{
}

OtsuSegmentationFilter::~OtsuSegmentationFilter()
{
}

void OtsuSegmentationFilter::GenerateData()
{
  OtsuFilterType::Pointer filter = OtsuFilterType::New();
  filter->SetNumberOfThresholds(m_NumberOfThresholds);

  mitk::Image::ConstPointer mitkImage = GetInput();
  itkInputImageType::Pointer itkImage;
  mitk::CastToItkImage(mitkImage, itkImage);

  filter->SetInput( itkImage );
  try
  {
    filter->Update();
  }
  catch( std::exception& err )
  {
    MITK_ERROR << err.what();
  }

  mitk::Image::Pointer resultImage = this->GetOutput();
  mitk::CastToMitkImage<itkOutputImageType>(filter->GetOutput(), resultImage);
}
}
