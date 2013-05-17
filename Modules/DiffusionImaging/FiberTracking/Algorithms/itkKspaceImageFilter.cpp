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
    , m_FrequencyMap(NULL)
    , m_SimulateRelaxation(true)
{

}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetSpacing( m_CompartmentImages.at(0)->GetSpacing() );
    outputImage->SetOrigin( m_CompartmentImages.at(0)->GetOrigin() );
    outputImage->SetDirection( m_CompartmentImages.at(0)->GetDirection() );
    outputImage->SetLargestPossibleRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
    outputImage->SetBufferedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
    outputImage->SetRequestedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
    outputImage->Allocate();

    m_SimulateDistortions = true;
    if (m_FrequencyMap.IsNull())
    {
        m_SimulateDistortions = false;
        m_FrequencyMap = InputImageType::New();
        m_FrequencyMap->SetSpacing( outputImage->GetSpacing() );
        m_FrequencyMap->SetOrigin( outputImage->GetOrigin() );
        m_FrequencyMap->SetDirection( outputImage->GetDirection() );
        m_FrequencyMap->SetLargestPossibleRegion( outputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->SetBufferedRegion( outputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->SetRequestedRegion( outputImage->GetLargestPossibleRegion() );
        m_FrequencyMap->Allocate();
        m_FrequencyMap->FillBuffer(0);
    }

    this->SetNthOutput(0, outputImage);
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId)
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

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

        double t = fromMaxEcho + (ky*szx+temp_k)*dt;    // dephasing time

        vcl_complex<double> s(0,0);

        InputIteratorType it(m_CompartmentImages.at(0), m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            double x = it.GetIndex()[0];
            double y = it.GetIndex()[1];

            vcl_complex<double> f(0, 0);

            // sum compartment signals and simulate relaxation
            for (int i=0; i<m_CompartmentImages.size(); i++)
                if (m_SimulateRelaxation)
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) * exp(-(m_TE+t)/m_T2.at(i) -fabs(t)/m_Tinhom), 0);
                else
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) );

            // simulate distortions and ghosting
            if (m_SimulateDistortions)
                s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/szx + ky*y/szy) + m_FrequencyMap->GetPixel(it.GetIndex())*10*t ) );
            else
                s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/szx + ky*y/szy)) );

            ++it;
        }
        s /= numPix;
        oit.Set(s);
        ++oit;
    }
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::AfterThreadedGenerateData()
{

}

}
#endif
