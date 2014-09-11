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
    : m_FrequencyMapSlice(NULL)
    , m_Z(0)
    , m_UseConstantRandSeed(false)
    , m_SpikesPerSlice(0)
    , m_IsBaseline(true)
{
    m_DiffusionGradientDirection.Fill(0.0);

    m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{
    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        m_RandGen->SetSeed(0);
    else
        m_RandGen->SetSeed();

    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    itk::ImageRegion<2> region; region.SetSize(0, m_OutSize[0]);  region.SetSize(1, m_OutSize[1]);
    outputImage->SetLargestPossibleRegion( region );
    outputImage->SetBufferedRegion( region );
    outputImage->SetRequestedRegion( region );
    outputImage->Allocate();

    double gamma = 42576000;    // Gyromagnetic ratio in Hz/T
    if ( m_Parameters.m_SignalGen.m_EddyStrength>0 && m_DiffusionGradientDirection.GetNorm()>0.001)
    {
        m_DiffusionGradientDirection.Normalize();
        m_DiffusionGradientDirection = m_DiffusionGradientDirection *  m_Parameters.m_SignalGen.m_EddyStrength/1000 *  gamma;
        m_IsBaseline = false;
    }

    this->SetNthOutput(0, outputImage);
    m_Spike = vcl_complex<double>(0,0);

    m_Transform =  m_Parameters.m_SignalGen.m_ImageDirection;
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            m_Transform[i][j] *=  m_Parameters.m_SignalGen.m_ImageSpacing[j];
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType)
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

    double kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);  // k-space size in x-direction
    double kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);  // k-space size in y-direction
    double xMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(0); // scanner coverage in x-direction
    double yMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(1); // scanner coverage in y-direction

    double numPix = kxMax*kyMax;
    double dt =  m_Parameters.m_SignalGen.m_tLine/kxMax;
    double fromMaxEcho = -  m_Parameters.m_SignalGen.m_tLine*kyMax/2;

    double upsampling = xMax/kxMax;     //  discrepany between k-space resolution and image resolution
    double yMaxFov = kyMax*upsampling;  //  actual FOV in y-direction (in x-direction xMax==FOV)

    int xRingingOffset = xMax-kxMax;
    int yRingingOffset = yMaxFov-kyMax;

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
        if ( m_Parameters.m_SignalGen.m_EddyStrength>0)
            eddyDecay = exp(-( m_Parameters.m_SignalGen.m_tEcho/2 + t)/ m_Parameters.m_SignalGen.m_Tau) * t/1000;

        // calcualte signal relaxation factors
        std::vector< double > relaxFactor;
        if ( m_Parameters.m_SignalGen.m_DoSimulateRelaxation)
            for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
                relaxFactor.push_back(exp(-( m_Parameters.m_SignalGen.m_tEcho+t)/m_T2.at(i) -fabs(t)/ m_Parameters.m_SignalGen.m_tInhom));

        double kx = kIdx[0];
        double ky = kIdx[1];
        if (oit.GetIndex()[1]%2 == 1)               // reverse readout direction and add ghosting
        {
            kIdx[0] = kxMax-kIdx[0]-1;                // reverse readout direction
            kx = (double)kIdx[0]- m_Parameters.m_SignalGen.m_KspaceLineOffset;    // add gradient delay induced offset
        }
        else
            kx +=  m_Parameters.m_SignalGen.m_KspaceLineOffset;    // add gradient delay induced offset

        // add gibbs ringing offset (cropps k-space)
        if (kx>=kxMax/2)
            kx += xRingingOffset;
        if (ky>=kyMax/2)
            ky += yRingingOffset;

        vcl_complex<double> s(0,0);
        InputIteratorType it(m_CompartmentImages.at(0), m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
            double x = it.GetIndex()[0]-xMax/2;
            double y = it.GetIndex()[1]-yMax/2;

            vcl_complex<double> f(0, 0);

            // sum compartment signals and simulate relaxation
            for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
                if ( m_Parameters.m_SignalGen.m_DoSimulateRelaxation)
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) * relaxFactor.at(i) *  m_Parameters.m_SignalGen.m_SignalScale, 0);
                else
                    f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) *  m_Parameters.m_SignalGen.m_SignalScale );

            // simulate eddy currents and other distortions
            double omega_t = 0;
            if (  m_Parameters.m_SignalGen.m_EddyStrength>0 && !m_IsBaseline)
            {
                itk::Vector< double, 3 > pos; pos[0] = x; pos[1] = y; pos[2] = m_Z;
                pos = m_Transform*pos/1000;   // vector from image center to current position (in meter)
                omega_t += (m_DiffusionGradientDirection[0]*pos[0]+m_DiffusionGradientDirection[1]*pos[1]+m_DiffusionGradientDirection[2]*pos[2])*eddyDecay;
            }
            if (m_FrequencyMapSlice.IsNotNull()) // simulate distortions
                omega_t += m_FrequencyMapSlice->GetPixel(it.GetIndex())*t/1000;

            if (y<-yMaxFov/2)
                y += yMaxFov;
            else if (y>=yMaxFov/2)
                y -= yMaxFov;

            // actual DFT term
            s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/xMax + ky*y/yMaxFov + omega_t )) );

            ++it;
        }
        s /= numPix;

        if (m_SpikesPerSlice>0 && sqrt(s.imag()*s.imag()+s.real()*s.real()) > sqrt(m_Spike.imag()*m_Spike.imag()+m_Spike.real()*m_Spike.real()) )
            m_Spike = s;

        outputImage->SetPixel(kIdx, s);
        ++oit;
    }
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::AfterThreadedGenerateData()
{
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    double kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);  // k-space size in x-direction
    double kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);  // k-space size in y-direction

    m_Spike *=  m_Parameters.m_SignalGen.m_SpikeAmplitude;
    itk::Index< 2 > spikeIdx;
    for (unsigned int i=0; i<m_SpikesPerSlice; i++)
    {
        spikeIdx[0] = m_RandGen->GetIntegerVariate()%(int)kxMax;
        spikeIdx[1] = m_RandGen->GetIntegerVariate()%(int)kyMax;
        outputImage->SetPixel(spikeIdx, m_Spike);
    }
}

}
#endif
