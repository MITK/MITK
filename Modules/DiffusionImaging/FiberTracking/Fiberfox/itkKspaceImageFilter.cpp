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
#include <mitkSingleShotEpi.h>
#include <mitkCartesianReadout.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< class TPixelType >
KspaceImageFilter< TPixelType >
::KspaceImageFilter()
    : m_Z(0)
    , m_UseConstantRandSeed(false)
    , m_SpikesPerSlice(0)
    , m_IsBaseline(true)
{
    m_DiffusionGradientDirection.Fill(0.0);

    m_CoilPosition.Fill(0.0);
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{
    m_Spike = vcl_complex<double>(0,0);
    m_SpikeLog = "";

    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    itk::ImageRegion<2> region; region.SetSize(0, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0));  region.SetSize(1, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1));
    outputImage->SetLargestPossibleRegion( region );
    outputImage->SetBufferedRegion( region );
    outputImage->SetRequestedRegion( region );
    outputImage->Allocate();
    outputImage->FillBuffer(m_Spike);

    m_KSpaceImage = InputImageType::New();
    m_KSpaceImage->SetLargestPossibleRegion( region );
    m_KSpaceImage->SetBufferedRegion( region );
    m_KSpaceImage->SetRequestedRegion( region );
    m_KSpaceImage->Allocate();
    m_KSpaceImage->FillBuffer(0.0);

    m_Gamma = 42576000;    // Gyromagnetic ratio in Hz/T (1.5T)
    if ( m_Parameters->m_SignalGen.m_EddyStrength>0 && m_DiffusionGradientDirection.GetNorm()>0.001)
    {
        m_DiffusionGradientDirection.Normalize();
        m_DiffusionGradientDirection = m_DiffusionGradientDirection *  m_Parameters->m_SignalGen.m_EddyStrength/1000 *  m_Gamma;
        m_IsBaseline = false;
    }

    this->SetNthOutput(0, outputImage);

    m_Transform =  m_Parameters->m_SignalGen.m_ImageDirection;
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            m_Transform[i][j] *=  m_Parameters->m_SignalGen.m_ImageSpacing[j];

    double a = m_Parameters->m_SignalGen.m_ImageRegion.GetSize(0)*m_Parameters->m_SignalGen.m_ImageSpacing[0];
    double b = m_Parameters->m_SignalGen.m_ImageRegion.GetSize(1)*m_Parameters->m_SignalGen.m_ImageSpacing[1];
    double diagonal = sqrt(a*a+b*b)/1000;   // image diagonal in m

    switch (m_Parameters->m_SignalGen.m_CoilSensitivityProfile)
    {
    case SignalGenerationParameters::COIL_CONSTANT:
    {
        m_CoilSensitivityFactor = 1;                    // same signal everywhere
        break;
    }
    case SignalGenerationParameters::COIL_LINEAR:
    {
        m_CoilSensitivityFactor = -1/diagonal;          // about 50% of the signal in the image center remaining
        break;
    }
    case SignalGenerationParameters::COIL_EXPONENTIAL:
    {
        m_CoilSensitivityFactor = -log(0.1)/diagonal;   // about 32% of the signal in the image center remaining
        break;
    }
    }

    switch (m_Parameters->m_SignalGen.m_AcquisitionType)
    {
    case SignalGenerationParameters::SingleShotEpi:
        m_ReadoutScheme = new mitk::SingleShotEpi(m_Parameters);
        break;
    case SignalGenerationParameters::SpinEcho:
        m_ReadoutScheme = new mitk::CartesianReadout(m_Parameters);
        break;
    default:
        m_ReadoutScheme = new mitk::SingleShotEpi(m_Parameters);
    }

    m_ReadoutScheme->AdjustEchoTime();
}

template< class TPixelType >
double KspaceImageFilter< TPixelType >::CoilSensitivity(DoubleVectorType& pos)
{
    // *************************************************************************
    // Coil ring is moving with excited slice (FIX THIS SOMETIME)
    m_CoilPosition[2] = pos[2];
    // *************************************************************************

    switch (m_Parameters->m_SignalGen.m_CoilSensitivityProfile)
    {
    case SignalGenerationParameters::COIL_CONSTANT:
        return 1;
    case SignalGenerationParameters::COIL_LINEAR:
    {
        DoubleVectorType diff = pos-m_CoilPosition;
        double sens = diff.GetNorm()*m_CoilSensitivityFactor + 1;
        if (sens<0)
            sens = 0;
        return sens;
    }
    case SignalGenerationParameters::COIL_EXPONENTIAL:
    {
        DoubleVectorType diff = pos-m_CoilPosition;
        double dist = diff.GetNorm();
        return exp(-dist*m_CoilSensitivityFactor);
    }
    default:
        return 1;
    }
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType threadID)
{
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    randGen->SetSeed();
    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        randGen->SetSeed(0);
    else
        randGen->SetSeed();

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

    double kxMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0);
    double kyMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1);
    double xMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(0); // scanner coverage in x-direction
    double yMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(1); // scanner coverage in y-direction
    double yMaxFov = yMax*m_Parameters->m_SignalGen.m_CroppingFactor;               // actual FOV in y-direction (in x-direction FOV=xMax)

    double numPix = kxMax*kyMax;
    double noiseVar = m_Parameters->m_SignalGen.m_PartialFourier*m_Parameters->m_SignalGen.m_NoiseVariance/(kyMax*kxMax); // adjust noise variance since it is the intended variance in physical space and not in k-space

    while( !oit.IsAtEnd() )
    {
        // time from maximum echo
        double t= m_ReadoutScheme->GetTimeFromMaxEcho(oit.GetIndex());

        // time passed since k-space readout started
        double tRead = m_ReadoutScheme->GetRedoutTime(oit.GetIndex());

        // time passes since application of the RF pulse
        double tRf = m_Parameters->m_SignalGen.m_tEcho+t;

        // calculate eddy current decay factor (TODO: vielleicht umbauen dass hier die zeit vom letzten diffusionsgradienten an genommen wird. doku dann auch entsprechend anpassen.)
        double eddyDecay = 0;
        if ( m_Parameters->m_Misc.m_CheckAddEddyCurrentsBox && m_Parameters->m_SignalGen.m_EddyStrength>0)
            eddyDecay = exp(-tRead/m_Parameters->m_SignalGen.m_Tau );

        // calcualte signal relaxation factors
        std::vector< double > relaxFactor;
        if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
            for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
                relaxFactor.push_back( exp(-tRf/m_T2.at(i) -fabs(t)/ m_Parameters->m_SignalGen.m_tInhom)*(1.0-exp(-(m_Parameters->m_SignalGen.m_tRep + tRf)/m_T1.at(i))) );

        // get current k-space index (depends on the schosen k-space readout scheme)
        itk::Index< 2 > kIdx = m_ReadoutScheme->GetActualKspaceIndex(oit.GetIndex());

        // partial fourier
        bool pf = false;
        if (kIdx[1]>kyMax*m_Parameters->m_SignalGen.m_PartialFourier)
            pf = true;

        if (!pf)
        {
            // shift k for DFT: (0 -- N) --> (-N/2 -- N/2)
            double kx = kIdx[0];
            double ky = kIdx[1];
            if ((int)kxMax%2==1)
                kx -= (kxMax-1)/2;
            else
                kx -= kxMax/2;
            if ((int)kyMax%2==1)
                ky -= (kyMax-1)/2;
            else
                ky -= kyMax/2;

            // add ghosting
            if (oit.GetIndex()[1]%2 == 1)
                kx -= m_Parameters->m_SignalGen.m_KspaceLineOffset;    // add gradient delay induced offset
            else
                kx += m_Parameters->m_SignalGen.m_KspaceLineOffset;    // add gradient delay induced offset

            vcl_complex<double> s(0,0);
            InputIteratorType it(m_CompartmentImages.at(0), m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
            while( !it.IsAtEnd() )
            {
                double x = it.GetIndex()[0];
                double y = it.GetIndex()[1];
                if ((int)xMax%2==1)
                    x -= (xMax-1)/2;
                else
                    x -= xMax/2;
                if ((int)yMax%2==1)
                    y -= (yMax-1)/2;
                else
                    y -= yMax/2;

                DoubleVectorType pos; pos[0] = x; pos[1] = y; pos[2] = m_Z;
                pos = m_Transform*pos/1000;   // vector from image center to current position (in meter)

                vcl_complex<double> f(0, 0);

                // sum compartment signals and simulate relaxation
                for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
                    if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
                        f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) * relaxFactor.at(i) *  m_Parameters->m_SignalGen.m_SignalScale, 0);
                    else
                        f += std::complex<double>( m_CompartmentImages.at(i)->GetPixel(it.GetIndex()) *  m_Parameters->m_SignalGen.m_SignalScale );

                if (m_Parameters->m_SignalGen.m_CoilSensitivityProfile!=SignalGenerationParameters::COIL_CONSTANT)
                    f *= CoilSensitivity(pos);

                // simulate eddy currents and other distortions
                double omega = 0;   // frequency offset
                if (  m_Parameters->m_SignalGen.m_EddyStrength>0 && m_Parameters->m_Misc.m_CheckAddEddyCurrentsBox && !m_IsBaseline)
                {
                    omega += (m_DiffusionGradientDirection[0]*pos[0]+m_DiffusionGradientDirection[1]*pos[1]+m_DiffusionGradientDirection[2]*pos[2]) * eddyDecay;
                }
                if (m_Parameters->m_SignalGen.m_FrequencyMap.IsNotNull()) // simulate distortions
                {
                    itk::Point<double, 3> point3D;
                    ItkDoubleImgType::IndexType index; index[0] = it.GetIndex()[0]; index[1] = it.GetIndex()[1]; index[2] = m_Zidx;
                    if (m_Parameters->m_SignalGen.m_DoAddMotion)    // we have to account for the head motion since this also moves our frequency map
                    {
                        m_Parameters->m_SignalGen.m_FrequencyMap->TransformIndexToPhysicalPoint(index, point3D);
                        point3D = m_FiberBundle->TransformPoint(point3D.GetVnlVector(), -m_Rotation[0],-m_Rotation[1],-m_Rotation[2],-m_Translation[0],-m_Translation[1],-m_Translation[2]);
                        omega += InterpolateFmapValue(point3D);
                    }
                    else
                    {
                        omega += m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(index);
                    }
                }

                // if signal comes from outside FOV, mirror it back (wrap-around artifact - aliasing)
                if (y<-yMaxFov/2)
                    y += yMaxFov;
                else if (y>=yMaxFov/2)
                    y -= yMaxFov;

                // actual DFT term
                s += f * exp( std::complex<double>(0, 2 * M_PI * (kx*x/xMax + ky*y/yMaxFov + omega*t/1000 )) );

                ++it;
            }
            s /= numPix;

            if (m_SpikesPerSlice>0 && sqrt(s.imag()*s.imag()+s.real()*s.real()) > sqrt(m_Spike.imag()*m_Spike.imag()+m_Spike.real()*m_Spike.real()) )
                m_Spike = s;

            if (m_Parameters->m_SignalGen.m_NoiseVariance>0)
                s = vcl_complex<double>(s.real()+randGen->GetNormalVariate(0,noiseVar), s.imag()+randGen->GetNormalVariate(0,noiseVar));

            outputImage->SetPixel(kIdx, s);
            m_KSpaceImage->SetPixel(kIdx, sqrt(s.imag()*s.imag()+s.real()*s.real()) );
        }

        ++oit;
    }
}

template< class TPixelType >
void KspaceImageFilter< TPixelType >
::AfterThreadedGenerateData()
{
    delete m_ReadoutScheme;

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    double kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);  // k-space size in x-direction
    double kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);  // k-space size in y-direction

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    while( !oit.IsAtEnd() ) // use hermitian k-space symmetry to fill empty k-space parts resulting from partial fourier acquisition
    {
        itk::Index< 2 > kIdx;
        kIdx[0] = oit.GetIndex()[0];
        kIdx[1] = oit.GetIndex()[1];

        // reverse phase
        if (!m_Parameters->m_SignalGen.m_ReversePhase)
            kIdx[1] = kyMax-1-kIdx[1];

        if (kIdx[1]>kyMax*m_Parameters->m_SignalGen.m_PartialFourier)
        {
            // reverse readout direction
            if (oit.GetIndex()[1]%2 == 1)
                kIdx[0] = kxMax-kIdx[0]-1;

            // calculate symmetric index
            itk::Index< 2 > kIdx2;
            kIdx2[0] = (int)(kxMax-kIdx[0]-(int)kxMax%2)%(int)kxMax;
            kIdx2[1] = (int)(kyMax-kIdx[1]-(int)kyMax%2)%(int)kyMax;

            // use complex conjugate of symmetric index value at current index
            vcl_complex<double> s = outputImage->GetPixel(kIdx2);
            s = vcl_complex<double>(s.real(), -s.imag());
            outputImage->SetPixel(kIdx, s);

            m_KSpaceImage->SetPixel(kIdx, sqrt(s.imag()*s.imag()+s.real()*s.real()) );
        }
        ++oit;
    }

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    randGen->SetSeed();
    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        randGen->SetSeed(0);
    else
        randGen->SetSeed();

    m_Spike *=  m_Parameters->m_SignalGen.m_SpikeAmplitude;
    itk::Index< 2 > spikeIdx;
    for (unsigned int i=0; i<m_SpikesPerSlice; i++)
    {
        spikeIdx[0] = randGen->GetIntegerVariate()%(int)kxMax;
        spikeIdx[1] = randGen->GetIntegerVariate()%(int)kyMax;
        outputImage->SetPixel(spikeIdx, m_Spike);
        m_SpikeLog += "[" + boost::lexical_cast<std::string>(spikeIdx[0]) + "," + boost::lexical_cast<std::string>(spikeIdx[1]) + "," + boost::lexical_cast<std::string>(m_Zidx) + "] Magnitude: " + boost::lexical_cast<std::string>(m_Spike.real()) + "+" + boost::lexical_cast<std::string>(m_Spike.imag()) + "i\n";
    }
}



template< class TPixelType >
double KspaceImageFilter< TPixelType >::InterpolateFmapValue(itk::Point<float, 3> itkP)
{
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    m_Parameters->m_SignalGen.m_FrequencyMap->TransformPhysicalPointToIndex(itkP, idx);
    m_Parameters->m_SignalGen.m_FrequencyMap->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    double pix = 0;
    if ( m_Parameters->m_SignalGen.m_FrequencyMap->GetLargestPossibleRegion().IsInside(idx) )
        pix = m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(idx);
    else
        return pix;

    double frac_x = cIdx[0] - idx[0];
    double frac_y = cIdx[1] - idx[1];
    double frac_z = cIdx[2] - idx[2];
    if (frac_x<0)
    {
        idx[0] -= 1;
        frac_x += 1;
    }
    if (frac_y<0)
    {
        idx[1] -= 1;
        frac_y += 1;
    }
    if (frac_z<0)
    {
        idx[2] -= 1;
        frac_z += 1;
    }
    frac_x = 1-frac_x;
    frac_y = 1-frac_y;
    frac_z = 1-frac_z;

    // int coordinates inside image?
    if (idx[0] >= 0 && idx[0] < m_Parameters->m_SignalGen.m_FrequencyMap->GetLargestPossibleRegion().GetSize(0)-1 &&
            idx[1] >= 0 && idx[1] < m_Parameters->m_SignalGen.m_FrequencyMap->GetLargestPossibleRegion().GetSize(1)-1 &&
            idx[2] >= 0 && idx[2] < m_Parameters->m_SignalGen.m_FrequencyMap->GetLargestPossibleRegion().GetSize(2)-1)
    {
        vnl_vector_fixed<double, 8> interpWeights;
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        pix = m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(idx) * interpWeights[0];
        ItkDoubleImgType::IndexType tmpIdx = idx; tmpIdx[0]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[1];
        tmpIdx = idx; tmpIdx[1]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[2];
        tmpIdx = idx; tmpIdx[2]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[3];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[4];
        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[5];
        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[6];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(tmpIdx) * interpWeights[7];
    }

    return pix;
}

}
#endif
