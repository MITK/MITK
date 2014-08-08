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
#include "itkOtsuMultipleThresholdsImageFilter.h"

struct paramContainer
{
  paramContainer( unsigned int numThresholds, mitk::Image::Pointer image )
    : m_NumberOfThresholds(numThresholds), m_Image(image)
  {
  }

  unsigned int m_NumberOfThresholds;
  mitk::Image::Pointer m_Image;
};

template<typename TPixel, unsigned int VImageDimension>
void
AccessItkOtsuFilter(const itk::Image<TPixel, VImageDimension>* itkImage, paramContainer params)
{
  typedef itk::Image<TPixel, VImageDimension> itkInputImageType;
  typedef itk::Image< mitk::OtsuSegmentationFilter::OutputPixelType, 3 > itkOutputImageType;
  typedef itk::OtsuMultipleThresholdsImageFilter< itkInputImageType, itkOutputImageType > OtsuFilterType;

  typename OtsuFilterType::Pointer filter = OtsuFilterType::New();
  filter->SetNumberOfThresholds(params.m_NumberOfThresholds);
  filter->SetInput( itkImage );

  try
  {
    filter->Update();
  }
  catch( ... )
  {
    mitkThrow() << "itkOtsuFilter error.";
  }

  mitk::CastToMitkImage<itkOutputImageType>(filter->GetOutput(), params.m_Image);
  return;
}

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
  mitk::Image::ConstPointer mitkImage = GetInput();
  AccessByItk_n( mitkImage, AccessItkOtsuFilter, (paramContainer( m_NumberOfThresholds, this->GetOutput()) ) );
}
}
