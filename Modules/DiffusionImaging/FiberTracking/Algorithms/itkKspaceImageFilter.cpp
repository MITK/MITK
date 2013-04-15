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

#ifndef __itkKspaceImageFilter_txx
#define __itkKspaceImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkKspaceImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< class TPixelType >
KspaceImageFilter< TPixelType >
::KspaceImageFilter()
    : m_tLine(1)
    , m_kOffset(0)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    m_SpectrumImage = InputImageType::New();
    m_SpectrumImage->SetSpacing( inputImage->GetSpacing() );
    m_SpectrumImage->SetOrigin( inputImage->GetOrigin() );
    m_SpectrumImage->SetDirection( inputImage->GetDirection() );
    m_SpectrumImage->SetLargestPossibleRegion( inputImage->GetLargestPossibleRegion() );
    m_SpectrumImage->SetBufferedRegion( inputImage->GetLargestPossibleRegion() );
    m_SpectrumImage->SetRequestedRegion( inputImage->GetLargestPossibleRegion() );
    m_SpectrumImage->Allocate();
    m_SpectrumImage->FillBuffer(0);

    if (m_FrequencyMap.IsNull())
    {
        m_FrequencyMap = InputImageType::New();
        m_FrequencyMap->SetSpacing( inputImage->GetSpacing() );
        m_FrequencyMap->SetOrigin( inputImage->GetOrigin() );
        m_FrequencyMap->SetDirection( inputImage->GetDirection() );
        m_FrequencyMap->SetLargestPossibleRegion( inputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->SetBufferedRegion( inputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->SetRequestedRegion( inputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->Allocate();
        m_FrequencyMap->FillBuffer(0);
    }

    m_tLine /= 1000;
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, int threadId)
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    double szx = outputImage->GetLargestPossibleRegion().GetSize(0);
    double szy = outputImage->GetLargestPossibleRegion().GetSize(1);
    double numPix = szx*szy;
    double dt = m_tLine/szx;

    double fromMaxEcho = - m_tLine*szy/2;

    while( !oit.IsAtEnd() )
    {
        double kx = oit.GetIndex()[0];
        double ky = oit.GetIndex()[1];

        int temp_k = kx;
        if (oit.GetIndex()[1]%2 == 1)
        {
            temp_k = szx-kx-1;  // reverse readout direction
            kx -= m_kOffset;    // add gradient delay induced offset
        }
        else
            kx += m_kOffset;    // add gradient delay induced offset

        double t = fromMaxEcho + (ky*szx+temp_k)*dt;

        vcl_complex<double> s(0,0);
        InputIteratorType it(inputImage, inputImage->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            double x = it.GetIndex()[0];
            double y = it.GetIndex()[1];

            vcl_complex<double> f(it.Get(), 0);
            s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/szx + ky*y/szy) + m_FrequencyMap->GetPixel(it.GetIndex())*10*t ) );

            ++it;
        }
        s /= numPix;
        double magn = sqrt(s.real()*s.real()+s.imag()*s.imag());
        m_SpectrumImage->SetPixel(oit.GetIndex(), magn);
        oit.Set(s);

        ++oit;
    }
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::AfterThreadedGenerateData()
{
    ImageRegion<2> region = m_SpectrumImage->GetLargestPossibleRegion();
    typename InputImageType::Pointer rearrangedSlice = InputImageType::New();
    rearrangedSlice->SetLargestPossibleRegion( region );
    rearrangedSlice->SetBufferedRegion( region );
    rearrangedSlice->SetRequestedRegion( region );
    rearrangedSlice->Allocate();

    int xHalf = region.GetSize(0)/2;
    int yHalf = region.GetSize(1)/2;

    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            typename InputImageType::IndexType idx;
            idx[0]=x; idx[1]=y;
            TPixelType pix = m_SpectrumImage->GetPixel(idx);

            if( idx[0] <  xHalf )
                idx[0] = idx[0] + xHalf;
            else
                idx[0] = idx[0] - xHalf;

            if( idx[1] <  yHalf )
                idx[1] = idx[1] + yHalf;
            else
                idx[1] = idx[1] - yHalf;

            rearrangedSlice->SetPixel(idx, pix);
        }

    m_SpectrumImage = rearrangedSlice;
}

}
#endif
