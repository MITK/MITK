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
#ifndef __itkSplitDWImageFilter_txx
#define __itkSplitDWImageFilter_txx

#include "itkSplitDWImageFilter.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

template< class TInputImagePixelType,
          class TOutputImagePixelType>
itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::SplitDWImageFilter()
  : m_IndexList(0),
    m_ExtractAllImages(true)
{
  this->SetNumberOfRequiredInputs(1);
}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::CopyInformation( const DataObject* /*data*/)
{

}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter<
TInputImagePixelType, TOutputImagePixelType >::GenerateOutputInformation()
{

}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::SetExtractAllAboveThreshold(double b_threshold, BValueMapType map)
{
  m_ExtractAllImages = false;
  m_IndexList.clear();

  // create the index list following the given threshold
  // iterate over the b-value map
  BValueMapType::const_iterator bvalueIt = map.begin();
  while( bvalueIt != map.end() )
  {
    // check threshold
    if (bvalueIt->first > b_threshold)
    {
       // the map contains an index container, this needs to be inserted into the
       // index list
       IndexListType::const_iterator listIt = bvalueIt->second.begin();
       while( listIt != bvalueIt->second.end() )
       {
         m_IndexList.push_back( *listIt );
         ++listIt;
       }
    }

    ++bvalueIt;
  }
}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter< TInputImagePixelType, TOutputImagePixelType >
::SetExtractSingleShell(double b_value, BValueMapType map, double tol)
{
  m_ExtractAllImages = false;
  m_IndexList.clear();

  // create index list
  BValueMapType::const_iterator bvalueIt = map.begin();
  while( bvalueIt != map.end() )
  {
    IndexListType::const_iterator listIt = bvalueIt->second.begin();
    if( std::fabs( bvalueIt->first - b_value) < tol)
    {
      m_IndexList.insert( m_IndexList.begin(), bvalueIt->second.begin(), bvalueIt->second.end() );
      ++listIt;
    }

    ++bvalueIt;
  }
}

template< class TInputImagePixelType,
          class TOutputImagePixelType>
void itk::SplitDWImageFilter<
TInputImagePixelType, TOutputImagePixelType >::GenerateData()
{

  if( m_IndexList.empty() && !m_ExtractAllImages )
  {
    itkExceptionMacro(<<"The index list is empty and the option to extract all images is disabled. ");
  }

  // construct the index list (for each component)
  if( m_ExtractAllImages )
  {
    m_IndexList.clear();

    for( unsigned int i=0; i< this->GetInput()->GetNumberOfComponentsPerPixel(); i++)
      m_IndexList.push_back(i);
  }

  // declare the output image
  // this will have the b0 images stored as timesteps
  typename OutputImageType::Pointer outputImage = this->GetOutput();
  //OutputImageType::New();

  // allocate image with
  //  - dimension: [DimX, DimY, DimZ, size(IndexList) ]
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
  outputRegion.SetSize(3, m_IndexList.size());
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
  itk::ImageRegionConstIterator<InputImageType> inputIt( this->GetInput(),
                                                         this->GetInput()->GetLargestPossibleRegion() );


  // we want to iterate separately over each 3D volume of the output image
  // so reset the regions last dimension
  outputRegion.SetSize(3,1);
  unsigned int currentTimeStep = 0;

  // for each index in the iterator list, extract the image and insert it as a new time step
  for(IndexListType::const_iterator indexIt = m_IndexList.begin();
      indexIt != m_IndexList.end();
      indexIt++)
  {
    // set the time step
    outputRegion.SetIndex(3, currentTimeStep);
    itk::ImageRegionIterator< OutputImageType> outputIt( outputImage.GetPointer(), outputRegion );

    // iterate over the current b0 image and store it to corresponding place
    outputIt.GoToBegin();
    inputIt.GoToBegin();
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


#endif // __itkSplitDWImageFilter_txx
