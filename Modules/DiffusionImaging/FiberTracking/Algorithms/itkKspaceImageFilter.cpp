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
#include <itkImageFileWriter.h>

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
    , m_SimulateEddyCurrents(false)
    , m_Tau(70)
    , m_EddyGradientMagnitude(30)
    , m_IsBaseline(true)
    , m_SignalScale(1)
    , m_Spikes(0)
    , m_SpikeAmplitude(1)
{
    m_DiffusionGradientDirection.Fill(0.0);
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetSpacing( m_CompartmentImages.at(0)->GetSpacing() );
    outputImage->SetOrigin( m_CompartmentImages.at(0)->GetOrigin() );
    outputImage->SetDirection( m_CompartmentImages.at(0)->GetDirection() );
    itk::ImageRegion<2> region; region.SetSize(0, m_OutSize[0]);  region.SetSize(1, m_OutSize[1]);
    outputImage->SetLargestPossibleRegion( region );
    outputImage->SetBufferedRegion( region );
    outputImage->SetRequestedRegion( region );
    outputImage->Allocate();

    m_TEMPIMAGE = InputImageType::New();
    m_TEMPIMAGE->SetSpacing( m_CompartmentImages.at(0)->GetSpacing() );
    m_TEMPIMAGE->SetOrigin( m_CompartmentImages.at(0)->GetOrigin() );
    m_TEMPIMAGE->SetDirection( m_CompartmentImages.at(0)->GetDirection() );
    m_TEMPIMAGE->SetLargestPossibleRegion( region );
    m_TEMPIMAGE->SetBufferedRegion( region );
    m_TEMPIMAGE->SetRequestedRegion( region );
    m_TEMPIMAGE->Allocate();

    m_SimulateDistortions = true;
    if (m_FrequencyMap.IsNull())
    {
        m_SimulateDistortions = false;
        m_FrequencyMap = InputImageType::New();
        m_FrequencyMap->SetSpacing( m_CompartmentImages.at(0)->GetSpacing() );
        m_FrequencyMap->SetOrigin( m_CompartmentImages.at(0)->GetOrigin() );
        m_FrequencyMap->SetDirection( m_CompartmentImages.at(0)->GetDirection() );
        m_FrequencyMap->SetLargestPossibleRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        m_FrequencyMap->SetBufferedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        m_FrequencyMap->SetRequestedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        m_FrequencyMap->Allocate();
        m_FrequencyMap->FillBuffer(0);
    }

    double gamma = 42576000;    // Gyromagnetic ratio in Hz/T
    if (m_DiffusionGradientDirection.GetNorm()>0.001)
    {
        m_EddyGradientMagnitude /= 1000; // eddy gradient magnitude in T/m
        m_DiffusionGradientDirection.Normalize();
        m_DiffusionGradientDirection = m_DiffusionGradientDirection * m_EddyGradientMagnitude *  gamma;
        m_IsBaseline = false;
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

    double kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);
    double kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);
    double numPix = kxMax*kyMax;
    double dt = m_tLine/kxMax;

    double fromMaxEcho = - m_tLine*kyMax/2;

    double in_szx = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(0);
    double in_szy = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(1);
    int xOffset = in_szx-kxMax;
    int yOffset = in_szy-kyMax;

    vcl_complex<double> spike(0,0);
    while( !oit.IsAtEnd() )
    {
        itk::Index< 2 > kIdx;
        kIdx[0] = oit.GetIndex()[0];
        kIdx[1] = oit.GetIndex()[1];

        double t = fromMaxEcho + ((double)kIdx[1]*kxMax+(double)kIdx[0])*dt;    // dephasing time

        // rearrange slice
        if( kIdx[0] <  kxMax/2 )
            kIdx[0] = kIdx[0] + kxMax/2;
        else
            kIdx[0] = kIdx[0] - kxMax/2;

        if( kIdx[1] <  kyMax/2 )
            kIdx[1] = kIdx[1] + kyMax/2;
        else
            kIdx[1] = kIdx[1] - kyMax/2;

        // calculate eddy current decay factors
        double eddyDecay = 0;
        if (m_SimulateEddyCurrents)
            eddyDecay = exp(-(m_TE/2 + t)/m_Tau) * t/1000;

        // calcualte signal relaxation factors
        std::vector< double > relaxFactor;
        if (m_SimulateRelaxation)
            for (int i=0; i<m_CompartmentImages.size(); i++)
                relaxFactor.push_back(exp(-(m_TE+t)/m_T2.at(i) -fabs(t)/m_Tinhom));

        double kx = kIdx[0];
        double ky = kIdx[1];
        if (oit.GetIndex()[1]%2 == 1)               // reverse readout direction and add ghosting
        {
            kIdx[0] = kxMax-kIdx[0]-1;                // reverse readout direction
            kx = (double)kIdx[0]-m_kOffset;    // add gradient delay induced offset
        }
        else
            kx += m_kOffset;    // add gradient delay induced offset

        // add gibbs ringing offset (cropps k-space)
        if (kx>=kxMax/2)
            kx += xOffset;
        if (ky>=kyMax/2)
            ky += yOffset;

        vcl_complex<double> s(0,0);
        InputIteratorType it(m_CompartmentImages.at(0), m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            double x = it.GetIndex()[0]-in_szx/2;
            double y = it.GetIndex()[1]-in_szy/2;

            vcl_complex<double> f(0, 0);

            // sum compartment signals and simulate relaxation
            for (int i=0; i<m_CompartmentImages.size(); i++)
                if (m_SimulateRelaxation)
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) * relaxFactor.at(i) * m_SignalScale, 0);
                else
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) * m_SignalScale );

            // simulate eddy currents and other distortions
            double omega_t = 0;
            if ( m_SimulateEddyCurrents && !m_IsBaseline)
            {
                itk::Vector< double, 3 > pos; pos[0] = x; pos[1] = y; pos[2] = m_Z;
                pos = m_DirectionMatrix*pos/1000;   // vector from image center to current position (in meter)
                omega_t += (m_DiffusionGradientDirection[0]*pos[0]+m_DiffusionGradientDirection[1]*pos[1]+m_DiffusionGradientDirection[2]*pos[2])*eddyDecay;
            }
            if (m_SimulateDistortions)
                omega_t += m_FrequencyMap->GetPixel(it.GetIndex())*t/1000;

            // actual DFT term
            s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/in_szx + ky*y/in_szy + omega_t )) );

            ++it;
        }
        s /= numPix;

        if (m_Spikes>0 && sqrt(s.imag()*s.imag()+s.real()*s.real()) > sqrt(spike.imag()*spike.imag()+spike.real()*spike.real()) )
            spike = s;

        //        m_TEMPIMAGE->SetPixel(kIdx, sqrt(s.real()*s.real()+s.imag()*s.imag()));
        outputImage->SetPixel(kIdx, s);
        ++oit;
    }

    spike *= m_SpikeAmplitude;
    for (int i=0; i<m_Spikes; i++)
    {
        itk::Index< 2 > spikeIdx;
        spikeIdx[0] = rand()%(int)kxMax;
        spikeIdx[1] = rand()%(int)kyMax;
        outputImage->SetPixel(spikeIdx, spike);
    }

    //    typedef itk::ImageFileWriter< InputImageType > WriterType;
    //    typename WriterType::Pointer writer = WriterType::New();
    //    writer->SetFileName("/local/kspace.nrrd");
    //    writer->SetInput(m_TEMPIMAGE);
    //    writer->Update();
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::AfterThreadedGenerateData()
{

}

}
#endif
