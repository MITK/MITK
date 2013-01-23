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
#ifndef __itkB0ImageExtractionToSeparateImageFilter_txx
#define __itkB0ImageExtractionToSeparateImageFilter_txx

#include "itkB0ImageExtractionToSeparateImageFilter.h"

template< class TInputImagePixelType,
          class TOutputImagePixelType>
itk::B0ImageExtractionToSeparateImageFilter< TInputImagePixelType, TOutputImagePixelType >
::B0ImageExtractionToSeparateImageFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::B0ImageExtractionToSeparateImageFilter<
TInputImagePixelType, TOutputImagePixelType >::CopyInformation( const DataObject *data)
{

}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::B0ImageExtractionToSeparateImageFilter<
TInputImagePixelType, TOutputImagePixelType >::GenerateOutputInformation()
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
  typename OutputImageType::Pointer outputImage = this->GetOutput();
  //OutputImageType::New();

  // get the input region
  typename Superclass::InputImageType::RegionType inputRegion =
      this->GetInput()->GetLargestPossibleRegion();

  // allocate image with
  //  - dimension: [DimX, DimY, DimZ, NumOfb0 ]
  //  - spacing: old one, 1.0 time

  typename OutputImageType::SpacingType spacing;
  spacing.Fill(1);

  typename OutputImageType::PointType origin;
  origin.Fill(0);

  OutputImageRegionType outputRegion;

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

  // output image direction (4x4)
  typename OutputImageType::DirectionType outputDirection;
  //initialize to identity
  outputDirection.SetIdentity();
  // get the input image direction ( 3x3 matrix )
  typename InputImageType::DirectionType inputDirection = this->GetInput()->GetDirection();

  for( unsigned int i=0; i< 3; i++)
  {
    outputDirection(0,i) = inputDirection(0,i);
    outputDirection(1,i) = inputDirection(1,i);
    outputDirection(2,i) = inputDirection(2,i);
  }

  outputImage->SetSpacing( spacing );
  outputImage->SetOrigin( origin );
  outputImage->SetDirection( outputDirection );
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
    inputIt = inputIt.Begin();
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
