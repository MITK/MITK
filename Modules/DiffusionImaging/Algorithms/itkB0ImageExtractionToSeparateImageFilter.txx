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

    // Find the indices of the b0 images in the diffusion image
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
    // this will have the b0 images stored as timesteps
    typedef itk::Image<float, 4> OutputImageType;
    OutputImageType::Pointer outputImage = OutputImageType::New();

    // get the input region
    typename Superclass::InputImageType::RegionType inputRegion =
        this->GetInput()->GetLargestPossibleRegion();

    // allocate image with
    //  - dimension: [DimX, DimY, DimZ, NumOfb0 ]
    //  - spacing: old one, 1.0 time

    OutputImageType::SpacingType spacing;
    spacing.Fill(1);

    OutputImageType::PointType origin;
    origin.Fill(0);

    OutputImageType::RegionType outputRegion;

    // the spacing and origin corresponds to the respective values in the input image (3D)
    // the same for the region
    for (unsigned int i=0; i< 3; i++)
    {
      spacing[i] = (this->GetInput()->GetSpacing())[i];
      origin[i] = (this->GetInput()->GetOrigin())[i];
      outputRegion.SetSize(i, this->GetInput()->GetLargestPossibleRegion().GetSize()[i]);
      outputRegion.SetIndex(i, this->GetInput()->GetLargestPossibleRegion().GetIndex()[i]);
    }

    // number of timesteps = number of b0 images
    outputRegion.SetSize(3, indices.size());
    outputRegion.SetIndex( 3, 0 );

    outputImage->SetSpacing( spacing );
    outputImage->SetOrigin( origin );
    // FIXME: direction matrix
    outputImage->SetRegions( outputRegion );
    outputImage->Allocate();

    // input iterator
    itk::ImageRegionConstIterator<InputImageType> inputIt( this->GetInput(), this->GetInput()->GetLargestPossibleRegion() );

    // we want to iterate separately over each 3D volume of the output image
    // so reset the regions last dimension
    outputRegion.SetSize(3,1);
    unsigned int currentTimeStep = 0;


    // extract b0 and insert them as a new time step
    for(std::vector<int>::iterator indexIt = indices.begin();
      indexIt != indices.end();
      indexIt++)
    {
      // set the time step
      outputRegion.SetIndex(3, currentTimeStep);
      itk::ImageRegionIterator< OutputImageType> outputIt( outputImage.GetPointer(), outputRegion );

      // iterate over the current b0 image and store it to corresponding place
      outputIt = outputIt.Begin();
      while( !outputIt.IsAtEnd() && !inputIt.IsAtEnd() )
      {
        // the input vector
        typename InputImageType::PixelType vec = inputIt.Get();

        outputIt.Set( vec[*indexIt]);
        ++outputIt;
        ++inputIt;
      }

      // increase time step
      currentTimeStep++;
    }


  }

#endif // ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
