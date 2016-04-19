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

#ifndef __itkVectorImageToImageFilter_txx
#define __itkVectorImageToImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionIterator.h"


namespace itk {


template< class TPixelType >
VectorImageToImageFilter< TPixelType >
::VectorImageToImageFilter()
  : m_Index(0)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void
VectorImageToImageFilter< TPixelType >
::GenerateData()
{
    typename OutputImageType::Pointer outputImage =
            static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    outputImage->SetSpacing(this->GetInput()->GetSpacing());
    outputImage->SetOrigin(this->GetInput()->GetOrigin());
    outputImage->SetRegions(this->GetInput()->GetLargestPossibleRegion());
    outputImage->Allocate();

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    oit.GoToBegin();

  //   InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = NULL;
    inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    ImageRegionIterator <InputImageType> git( inputImagePointer, inputImagePointer->GetLargestPossibleRegion() );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
      oit.Set(git.Get()[m_Index]);
      ++oit;
      ++git;
    }
}

template< class TPixelType >
void VectorImageToImageFilter< TPixelType >::PrintSelf(std::ostream& os, Indent indent) const
{
    Superclass::PrintSelf(os,indent);
}

}

#endif // __itkVectorImageToImageFilter_txx
