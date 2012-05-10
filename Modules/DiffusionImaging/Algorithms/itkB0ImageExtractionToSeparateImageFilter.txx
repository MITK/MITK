/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
#define __itkB0ImageExtractionToSeparateImageFilter_txx

#include "itkB0ImageExtractionToSeparateImageFilter.h"

template< class TInputImagePixelType,
          class TOutputImagePixelType>
  itk::B0ImageExtractionToSeparateImageFilter< TInputImagePixelType, TOutputImagePixelType >
  ::B0ImageExtractionToSeparateImageFilter()
    : B0ImageExtractionImageFilter< TInputImagePixelType, TOutputImagePixelType >()
  {

  }

template< class TInputImagePixelType,
          class TOutputImagePixelType>
  void itk::B0ImageExtractionToSeparateImageFilter<
  TInputImagePixelType, TOutputImagePixelType >::GenerateData()
  {

    GradContainerIteratorType begin = m_Directions->Begin();
    GradContainerIteratorType end = m_Directions->End();

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

    // declare the output image
    typedef itk::Image<float, 4> TempImageType;
    TempImageType::Pointer tmp = TempImageType::New();

    // get the input region
    typename Superclass::InputImageType::RegionType inputRegion =
        this->GetInput()->GetLargestPossibleRegion();

    // allocate image with
    //  - dimension: [DimX, DimY, DimZ, NumOfb0 ]
    //  - spacing: old one, 1.0 time

    TempImageType::SpacingType spacing;
    spacing.Fill(1);

    TempImageType::PointType origin;
    origin.Fill(0);

    for (unsigned int i=0; i< 3; i++)
    {
      spacing[i] = (this->GetInput()->GetSpacing())[i];
      origin[i] = (this->GetInput()->GetOrigin())[i];
    }

    // extract b0 and insert them as a new time step
    //Sum all images that have zero diffusion weighting (indices stored in vector index)
    for(std::vector<int>::iterator indexIt = indices.begin();
      indexIt != indices.end();
      indexIt++)
    {

    }


  }

#endif // ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
