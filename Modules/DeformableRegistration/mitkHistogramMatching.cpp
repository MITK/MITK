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

#include "mitkHistogramMatching.h"

#include <mitkImageCast.h>

namespace mitk {

  HistogramMatching::HistogramMatching()
  {
    m_ThresholdAtMeanIntensity = true;
    m_NumberOfHistogramLevels = 1024;
    m_NumberOfMatchPoints = 7;
  }

  HistogramMatching::~HistogramMatching()
  {
  }

  void HistogramMatching::SetNumberOfHistogramLevels(int histogrammLevels)
  {
    m_NumberOfHistogramLevels = histogrammLevels;
  }

  void HistogramMatching::SetNumberOfMatchPoints(int matchPoints)
  {
    m_NumberOfMatchPoints = matchPoints;
  }

  void HistogramMatching::SetThresholdAtMeanIntensity(bool on)
  {
    m_ThresholdAtMeanIntensity = on;
  }

  template < typename TPixel, unsigned int VImageDimension >
  void HistogramMatching::GenerateData2( const itk::Image<TPixel, VImageDimension>* itkImage1)
  {
    typedef typename itk::Image< TPixel, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixel, VImageDimension >  MovingImageType;

    typedef float InternalPixelType;
    typedef typename itk::Image< InternalPixelType, VImageDimension > InternalImageType;
    typedef typename itk::CastImageFilter< FixedImageType,
                                InternalImageType > FixedImageCasterType;
    typedef typename itk::CastImageFilter< MovingImageType,
                                InternalImageType > MovingImageCasterType;
    typedef typename itk::HistogramMatchingImageFilter<
                                    InternalImageType,
                                    InternalImageType >   MatchingFilterType;

    typename FixedImageType::Pointer fixedImage = FixedImageType::New();
    mitk::CastToItkImage(m_ReferenceImage, fixedImage);
    typename MovingImageType::ConstPointer movingImage = itkImage1;
    if (fixedImage.IsNotNull() && movingImage.IsNotNull())
    {
      typename FixedImageCasterType::Pointer fixedImageCaster = FixedImageCasterType::New();
      fixedImageCaster->SetInput(fixedImage);
      typename MovingImageCasterType::Pointer movingImageCaster = MovingImageCasterType::New();
      movingImageCaster->SetInput(movingImage);
      typename MatchingFilterType::Pointer matcher = MatchingFilterType::New();
      matcher->SetInput( movingImageCaster->GetOutput() );
      matcher->SetReferenceImage( fixedImageCaster->GetOutput() );
      matcher->SetNumberOfHistogramLevels( m_NumberOfHistogramLevels );
      matcher->SetNumberOfMatchPoints( m_NumberOfMatchPoints );
      matcher->SetThresholdAtMeanIntensity(m_ThresholdAtMeanIntensity);
      matcher->Update();
      Image::Pointer outputImage = this->GetOutput();
      mitk::CastToMitkImage( matcher->GetOutput(), outputImage );
    }
  }
} // end namespace
