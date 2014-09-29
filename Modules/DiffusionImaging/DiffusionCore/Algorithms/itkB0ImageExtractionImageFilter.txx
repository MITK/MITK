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
#ifndef __itkB0ImageExtractionImageFilter_txx
#define __itkB0ImageExtractionImageFilter_txx

#include "itkB0ImageExtractionImageFilter.h"

#include "itkImageRegionIterator.h"

namespace itk {

  template< class TInputImagePixelType,
  class TOutputImagePixelType >
    B0ImageExtractionImageFilter< TInputImagePixelType,
    TOutputImagePixelType >
    ::B0ImageExtractionImageFilter()
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
  }

  template< class TInputImagePixelType,
  class TOutputImagePixelType >
    void B0ImageExtractionImageFilter< TInputImagePixelType,
    TOutputImagePixelType >
    ::GenerateData()
  {

    typename GradientDirectionContainerType::Iterator begin = m_Directions->Begin();
    typename GradientDirectionContainerType::Iterator end = m_Directions->End();

    // Find the index of the b0 image
    std::vector<int> indices;
    int index = 0;
    while(begin!=end)
    {
      GradientDirectionType grad = begin->Value();

      if(grad[0] == 0 && grad[1] == 0 && grad[2] == 0)
      {
        indices.push_back(index);
      }
      ++index;
      ++begin;
    }

    typedef itk::Image<float,3> TempImageType;
    TempImageType::Pointer tmp = TempImageType::New();
    typename TempImageType::RegionType region = this->GetInput()->GetLargestPossibleRegion();

    tmp->SetSpacing(this->GetInput()->GetSpacing());
    tmp->SetOrigin(this->GetInput()->GetOrigin());
    tmp->SetDirection(this->GetInput()->GetDirection());
    tmp->SetRegions(region);
    tmp->Allocate();

    itk::ImageRegionIterator<TempImageType> it(tmp.GetPointer(), tmp->GetLargestPossibleRegion() );
    itk::ImageRegionConstIterator<InputImageType> vectorIt(this->GetInput(), this->GetInput()->GetLargestPossibleRegion() );

    it.GoToBegin();
    while(!it.IsAtEnd())
    {
      it.Set(0);
      ++it;
    }

    //Sum all images that have zero diffusion weighting (indices stored in vector index)
    for(std::vector<int>::iterator indexIt = indices.begin();
      indexIt != indices.end();
      indexIt++)
    {
      it.GoToBegin();
      vectorIt.GoToBegin();

      while(!it.IsAtEnd() && !vectorIt.IsAtEnd())
      {
        typename InputImageType::PixelType vec = vectorIt.Get();
        it.Set((1.0 * it.Get()) + (1.0 * vec[*indexIt]) / (1.0 * indices.size()));
        ++it;
        ++vectorIt;
      }
    }

    typename OutputImageType::Pointer b0Image =
      static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());
    typename OutputImageType::RegionType outregion = this->GetInput()->GetLargestPossibleRegion();
    b0Image->SetSpacing(this->GetInput()->GetSpacing());
    b0Image->SetOrigin(this->GetInput()->GetOrigin());
    b0Image->SetDirection(this->GetInput()->GetDirection());
    b0Image->SetRegions(outregion);
    b0Image->Allocate();
    itk::ImageRegionIterator<TempImageType> itIn(tmp, tmp->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<OutputImageType> itOut(b0Image, b0Image->GetLargestPossibleRegion() );
    itIn.GoToBegin();
    itOut.GoToBegin();
    while(!itIn.IsAtEnd())
    {
      itOut.Set(itIn.Get());
      ++itIn;
      ++itOut;
    }
  }

}

#endif // __itkB0ImageExtractionImageFilter_txx
