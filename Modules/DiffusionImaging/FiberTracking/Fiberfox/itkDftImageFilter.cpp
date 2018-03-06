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

#ifndef __itkDftImageFilter_txx
#define __itkDftImageFilter_txx

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "itkDftImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

namespace itk {

template< class TPixelType >
DftImageFilter< TPixelType >
::DftImageFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void DftImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{

}

template< class TPixelType >
void DftImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType)
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    int szx = outputImage->GetLargestPossibleRegion().GetSize(0);
    int szy = outputImage->GetLargestPossibleRegion().GetSize(1);

    while( !oit.IsAtEnd() )
    {
        float kx = oit.GetIndex()[0];
        float ky = oit.GetIndex()[1];

        if ((int)szx%2==1)
            kx -= (szx-1)/2;
        else
            kx -= szx/2;
        if ((int)szy%2==1)
            ky -= (szy-1)/2;
        else
            ky -= szy/2;

        vcl_complex<TPixelType> s(0,0);
        InputIteratorType it(inputImage, inputImage->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            float x = it.GetIndex()[0];
            float y = it.GetIndex()[1];

            if ((int)szx%2==1)
                x -= (szx-1)/2;
            else
                x -= szx/2;
            if ((int)szy%2==1)
                y -= (szy-1)/2;
            else
                y -= szy/2;

            s += it.Get() * exp( std::complex<TPixelType>(0, -2 * itk::Math::pi * (kx*x/szx + ky*y/szy) ) );

            ++it;
        }

        oit.Set(s);
        ++oit;
    }
}

}
#endif
