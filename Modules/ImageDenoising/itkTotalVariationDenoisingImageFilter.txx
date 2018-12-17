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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef _itkTotalVariationDenoisingImageFilter_txx
#define _itkTotalVariationDenoisingImageFilter_txx
#include "itkTotalVariationDenoisingImageFilter.h"

#include "itkConstShapedNeighborhoodIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkLocalVariationImageFilter.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"

#include <algorithm>
#include <vector>

namespace itk
{
  template <class TInputImage, class TOutputImage>
  TotalVariationDenoisingImageFilter<TInputImage, TOutputImage>::TotalVariationDenoisingImageFilter()
    : m_Lambda(1.0), m_NumberIterations(0)
  {
  }

  template <class TInputImage, class TOutputImage>
  void TotalVariationDenoisingImageFilter<TInputImage, TOutputImage>::GenerateData()
  {
    // first we cast the input image to match output type
    typename CastType::Pointer infilter = CastType::New();
    infilter->SetInput(this->GetInput());
    infilter->Update();
    typename TOutputImage::Pointer image = infilter->GetOutput();

    // a second copy of the input image is saved as reference
    infilter = CastType::New();
    infilter->SetInput(this->GetInput());
    infilter->Update();
    typename TOutputImage::Pointer origImage = infilter->GetOutput();

    typename SingleIterationFilterType::Pointer filter;
    for (int i = 0; i < m_NumberIterations; i++)
    {
      filter = SingleIterationFilterType::New();
      filter->SetInput(image);
      filter->SetOriginalImage(origImage);
      filter->SetLambda(m_Lambda);
      filter->SetNumberOfThreads(this->GetNumberOfThreads());
      filter->UpdateLargestPossibleRegion();
      image = filter->GetOutput();
      std::cout << "Iteration " << i + 1 << "/" << m_NumberIterations << std::endl;
    }

    typename OutputImageType::Pointer output = this->GetOutput();
    output->SetSpacing(image->GetSpacing());
    typename OutputImageType::RegionType largestPossibleRegion;
    largestPossibleRegion.SetSize(image->GetLargestPossibleRegion().GetSize());
    largestPossibleRegion.SetIndex(image->GetLargestPossibleRegion().GetIndex());
    output->SetLargestPossibleRegion(image->GetLargestPossibleRegion());
    output->SetBufferedRegion(image->GetLargestPossibleRegion());
    output->Allocate();

    itk::ImageRegionIterator<OutputImageType> oit(output, output->GetLargestPossibleRegion());
    oit.GoToBegin();

    itk::ImageRegionConstIterator<OutputImageType> iit(image, image->GetLargestPossibleRegion());
    iit.GoToBegin();

    while (!oit.IsAtEnd())
    {
      oit.Set(iit.Get());
      ++iit;
      ++oit;
    }
  }

  /**
  * Standard "PrintSelf" method
  */
  template <class TInputImage, class TOutput>
  void TotalVariationDenoisingImageFilter<TInputImage, TOutput>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

} // end namespace itk

#endif
