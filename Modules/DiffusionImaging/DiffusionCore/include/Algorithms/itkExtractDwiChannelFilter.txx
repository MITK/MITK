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

#ifndef __itkExtractDwiChannelFilter_txx
#define __itkExtractDwiChannelFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"

namespace itk {


template< class TInPixelType >
ExtractDwiChannelFilter< TInPixelType>::ExtractDwiChannelFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType >
void ExtractDwiChannelFilter< TInPixelType>::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    if ( inputImagePointer->GetVectorLength()<=m_ChannelIndex )
        itkExceptionMacro("Index out of bounds!");
}

template< class TInPixelType >
void ExtractDwiChannelFilter< TInPixelType>::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    InputIteratorType git( inputImagePointer, outputRegionForThread );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
        oit.Set( git.Get()[m_ChannelIndex] );
        ++oit;
        ++git;
    }

    std::cout << "One Thread finished calculation" << std::endl;
}

}

#endif // __itkExtractDwiChannelFilter_txx
