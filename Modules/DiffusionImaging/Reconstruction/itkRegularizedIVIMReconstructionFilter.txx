/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Language:  C++
Date:      $Date: 2006-01-11 19:43:31 $
Version:   $Revision: x $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkRegularizedIVIMReconstructionFilter_txx
#define _itkRegularizedIVIMReconstructionFilter_txx

#include "itkConstShapedNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include <vector>
#include <algorithm>

namespace itk
{

  template <class TInputPixel, class TOutputPixel, class TRefPixelType>
  RegularizedIVIMReconstructionFilter<TInputPixel, TOutputPixel, TRefPixelType>
    ::RegularizedIVIMReconstructionFilter()
  {
    m_Lambda = 1.0;
  }


  template <class TInputPixel, class TOutputPixel, class TRefPixelType>
  void
  RegularizedIVIMReconstructionFilter<TInputPixel, TOutputPixel, TRefPixelType>
    ::GenerateData()
  {
    // first we cast the input image to match output type
    typename CastType::Pointer infilter = CastType::New();
    infilter->SetInput(this->GetInput());
    infilter->Update();
    typename OutputImageType::Pointer image = infilter->GetOutput();

    typename SingleIterationFilterType::Pointer filter;
    for(int i=0; i<m_NumberIterations; i++)
    {
      filter = SingleIterationFilterType::New();
      filter->SetInput( image.GetPointer() );
      filter->SetOriginalImage( m_ReferenceImage );
      filter->SetBValues(m_BValues);
      filter->SetLambda(m_Lambda);
      filter->SetNumberOfThreads(this->GetNumberOfThreads());
      filter->UpdateLargestPossibleRegion();
      image = filter->GetOutput();
      std::cout << "Iteration " << i+1 << "/" << 
        m_NumberIterations << std::endl;
    }
    
    typename OutputImageType::Pointer output = this->GetOutput();
    output->SetOrigin( image->GetOrigin() );     // Set the image origin
    output->SetDirection( image->GetDirection() );  // Set the image direction
    output->SetSpacing(image->GetSpacing());
    output->SetLargestPossibleRegion( image->GetLargestPossibleRegion() );
    output->SetBufferedRegion( image->GetLargestPossibleRegion() );
    output->Allocate();

    itk::ImageRegionIterator<OutputImageType> oit(
      output, output->GetLargestPossibleRegion());
    oit.GoToBegin();

    itk::ImageRegionConstIterator<OutputImageType> iit(
      image, image->GetLargestPossibleRegion());
    iit.GoToBegin();

    while(!oit.IsAtEnd())
    {
      oit.Set(iit.Get());
      ++iit;
      ++oit;
    }
  }


  /**
  * Standard "PrintSelf" method
  */
  template <class TInputPixel, class TOutputPixel, class TRefPixelType>
  void
  RegularizedIVIMReconstructionFilter<TInputPixel, TOutputPixel, TRefPixelType>
    ::PrintSelf(
    std::ostream& os, 
    Indent indent) const
  {
    Superclass::PrintSelf( os, indent );
  }

} // end namespace itk

#endif
