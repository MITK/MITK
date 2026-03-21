/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __itkMaskedNaryStatisticsImageFilter_hxx
#define __itkMaskedNaryStatisticsImageFilter_hxx

#include "itkMaskedNaryStatisticsImageFilter.h"
#include "itkProgressReporter.h"
#include "itkMaskedStatisticsImageFilter.h"

namespace itk
{
  /**
  * Constructor
  */
  template< class TInputImage, class TMaskImage >
  MaskedNaryStatisticsImageFilter< TInputImage, TMaskImage >
    ::MaskedNaryStatisticsImageFilter()
  {
    // This number will be incremented each time an image
    // is added over the two minimum required
    this->SetNumberOfRequiredInputs(1);
  }

  template< class TInputImage, class TMaskImage >
  void
    MaskedNaryStatisticsImageFilter< TInputImage, TMaskImage >
    ::GenerateData()
  {
    const unsigned int numberOfInputImages =
      static_cast< unsigned int >( this->GetNumberOfIndexedInputs() );

    ProgressReporter progress( this, 0, numberOfInputImages );

    m_Maximum.clear();
    m_Minimum.clear();
    m_Sum.clear();
    m_Mean.clear();
    m_Sigma.clear();
    m_Variance.clear();

    // go through the inputs and add iterators for non-null inputs
    for ( unsigned int i = 0; i < numberOfInputImages; ++i )
    {
      InputImagePointer inputPtr =
        dynamic_cast< TInputImage * >( ProcessObject::GetInput(i) );

      if ( !inputPtr )
      {
        itkExceptionMacro(<< "Cannot compute statistics. At least one of the input images has a wrong type. Wrong image index: "<< i);
      }

      typedef itk::MaskedStatisticsImageFilter<TInputImage, TMaskImage> FilterType;
      typename FilterType::Pointer statFilter = FilterType::New();

      statFilter->SetInput(inputPtr);
      statFilter->SetMask(m_Mask);
      statFilter->Update();

      m_Maximum.push_back(statFilter->GetMaximum());
      m_Minimum.push_back(statFilter->GetMinimum());
      m_Sum.push_back(statFilter->GetSum());
      m_Mean.push_back(statFilter->GetMean());
      m_Sigma.push_back(statFilter->GetSigma());
      m_Variance.push_back(statFilter->GetVariance());

      progress.CompletedPixel();
    }
  }

} // end namespace itk

#endif
