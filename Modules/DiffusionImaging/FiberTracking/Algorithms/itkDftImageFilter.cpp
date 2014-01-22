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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkDftImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< class TPixelType >
DftImageFilter< TPixelType >
::DftImageFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void DftImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId)
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    int szx = outputImage->GetLargestPossibleRegion().GetSize(0);
    int szy = outputImage->GetLargestPossibleRegion().GetSize(1);

    while( !oit.IsAtEnd() )
    {
        int kx = oit.GetIndex()[0];
        int ky = oit.GetIndex()[1];

        if( kx <  szx/2 )
            kx = kx + szx/2;
        else
            kx = kx - szx/2;

        if( ky <  szy/2 )
            ky = ky + szy/2;
        else
            ky = ky - szy/2;

        vcl_complex<double> s(0,0);
        InputIteratorType it(inputImage, inputImage->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            int x = it.GetIndex()[0];
            int y = it.GetIndex()[1];

            vcl_complex<double> f(it.Get().real(), it.Get().imag());
            s += f * exp( std::complex<double>(0, -2 * M_PI * (kx*(double)x/szx + ky*(double)y/szy) ) );

            ++it;
        }
        double magn = sqrt(s.real()*s.real()+s.imag()*s.imag());
        oit.Set(magn);

        ++oit;
    }
}

}
#endif
