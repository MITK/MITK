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
//#endif

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "itkKspaceImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <mitkSingleShotEpi.h>
#include <mitkCartesianReadout.h>
#include <mitkDiffusionFunctionCollection.h>

namespace itk {

  template< class ScalarType >
  KspaceImageFilter< ScalarType >::KspaceImageFilter()
    : m_Z(0)
    , m_UseConstantRandSeed(false)
    , m_SpikesPerSlice(0)
    , m_IsBaseline(true)
  {
    m_DiffusionGradientDirection.Fill(0.0);
    m_CoilPosition.Fill(0.0);
  }

  template< class ScalarType >
  void KspaceImageFilter< ScalarType >
  ::BeforeThreadedGenerateData()
  {
    m_Spike = vcl_complex<ScalarType>(0,0);
    m_SpikeLog = "";
    m_TransX = -m_Translation[0];
    m_TransY = -m_Translation[1];
    m_TransZ = -m_Translation[2];

    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    itk::ImageRegion<2> region;
    region.SetSize(0, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0));
    region.SetSize(1, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1));
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

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        m_Transform[i][j] = m_Parameters->m_SignalGen.m_ImageDirection[i][j] * m_Parameters->m_SignalGen.m_ImageSpacing[j]/1000;

    float a = m_Parameters->m_SignalGen.m_ImageRegion.GetSize(0)*m_Parameters->m_SignalGen.m_ImageSpacing[0];
    float b = m_Parameters->m_SignalGen.m_ImageRegion.GetSize(1)*m_Parameters->m_SignalGen.m_ImageSpacing[1];
    float diagonal = sqrt(a*a+b*b)/1000;   // image diagonal in m

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

    m_MovedFmap = nullptr;
    if (m_Parameters->m_Misc.m_DoAddDistortions && m_Parameters->m_SignalGen.m_FrequencyMap.IsNotNull() && m_Parameters->m_SignalGen.m_DoAddMotion)
    {
      // we have to account for the head motion since this also moves our frequency map
      itk::LinearInterpolateImageFunction< itk::Image< float, 3 >, float >::Pointer   fmapInterpolator;
      fmapInterpolator = itk::LinearInterpolateImageFunction< itk::Image< float, 3 >, float >::New();
      fmapInterpolator->SetInputImage(m_Parameters->m_SignalGen.m_FrequencyMap);

      m_MovedFmap = itk::Image< ScalarType, 2 >::New();
      m_MovedFmap->SetLargestPossibleRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
      m_MovedFmap->SetBufferedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
      m_MovedFmap->SetRequestedRegion( m_CompartmentImages.at(0)->GetLargestPossibleRegion() );
      m_MovedFmap->Allocate();
      m_MovedFmap->FillBuffer(0);

      ImageRegionIterator< InputImageType > it(m_MovedFmap, m_MovedFmap->GetLargestPossibleRegion() );
      while( !it.IsAtEnd() )
      {
        itk::Image<float, 3>::IndexType index;
        index[0] = it.GetIndex()[0];
        index[1] = it.GetIndex()[1];
        index[2] = m_Zidx;

        itk::Point<float, 3> point3D;
        m_Parameters->m_SignalGen.m_FrequencyMap->TransformIndexToPhysicalPoint(index, point3D);
        m_FiberBundle->TransformPoint<float>( point3D, m_RotationMatrix, m_TransX, m_TransY, m_TransZ );
        it.Set(mitk::imv::GetImageValue<float>(point3D, true, fmapInterpolator));
        ++it;
      }
    }
  }

  template< class ScalarType >
  float KspaceImageFilter< ScalarType >::CoilSensitivity(VectorType& pos)
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
        VectorType diff = pos-m_CoilPosition;
        float sens = diff.GetNorm()*m_CoilSensitivityFactor + 1;
        if (sens<0)
          sens = 0;
        return sens;
      }
      case SignalGenerationParameters::COIL_EXPONENTIAL:
      {
        VectorType diff = pos-m_CoilPosition;
        float dist = static_cast<float>(diff.GetNorm());
        return std::exp(-dist*m_CoilSensitivityFactor);
      }
      default:
      return 1;
    }
  }

  template< class ScalarType >
  void KspaceImageFilter< ScalarType >
  ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType)
  {
    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    randGen->SetSeed();
    if (m_UseConstantRandSeed)  // always generate the same random numbers?
    {
      randGen->SetSeed(0);
    }
    else
    {
      randGen->SetSeed();
    }

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

    float kxMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0);
    float kyMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1);
    float xMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(0); // scanner coverage in x-direction
    float yMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(1); // scanner coverage in y-direction
    float yMaxFov = yMax;
    if (m_Parameters->m_Misc.m_DoAddAliasing)
        yMaxFov *= m_Parameters->m_SignalGen.m_CroppingFactor;               // actual FOV in y-direction (in x-direction FOV=xMax)

    float numPix = kxMax*kyMax;
    // Adjust noise variance since it is the intended variance in physical space and not in k-space:
    float noiseVar = m_Parameters->m_SignalGen.m_PartialFourier*m_Parameters->m_SignalGen.m_NoiseVariance/(kyMax*kxMax);

    while( !oit.IsAtEnd() )
    {
      typename OutputImageType::IndexType out_idx = oit.GetIndex();

      // time from maximum echo
      float t = m_ReadoutScheme->GetTimeFromMaxEcho(out_idx);

      // time passed since k-space readout started
      float tRead = m_ReadoutScheme->GetRedoutTime(out_idx);

      // time passes since application of the RF pulse
      float tRf = m_Parameters->m_SignalGen.m_tEcho+t;

      // calculate eddy current decay factor
      // (TODO: vielleicht umbauen dass hier die zeit vom letzten diffusionsgradienten an genommen wird. doku dann auch entsprechend anpassen.)
      float eddyDecay = 0;
      if ( m_Parameters->m_Misc.m_DoAddEddyCurrents && m_Parameters->m_SignalGen.m_EddyStrength>0)
        eddyDecay = std::exp(-tRead/m_Parameters->m_SignalGen.m_Tau );

      // calcualte signal relaxation factors
      std::vector< float > relaxFactor;
      if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
      {
        for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
          relaxFactor.push_back( std::exp(-tRf/m_T2[i] -fabs(t)/ m_Parameters->m_SignalGen.m_tInhom)
                                 * (1.0-std::exp(-(m_Parameters->m_SignalGen.m_tRep + tRf)/m_T1[i])) );
      }

      // get current k-space index (depends on the chosen k-space readout scheme)
      itk::Index< 2 > kIdx = m_ReadoutScheme->GetActualKspaceIndex(out_idx);

      // partial fourier
      bool partial_fourier_skip = false;
      if (kIdx[1]>kyMax*m_Parameters->m_SignalGen.m_PartialFourier)
        partial_fourier_skip = true;

      if (!partial_fourier_skip)
      {
        // shift k for DFT: (0 -- N) --> (-N/2 -- N/2)
        float kx = kIdx[0];
        float ky = kIdx[1];
        if (static_cast<int>(kxMax)%2==1)
          kx -= (kxMax-1)/2;
        else
          kx -= kxMax/2;

        if (static_cast<int>(kyMax)%2==1)
          ky -= (kyMax-1)/2;
        else
          ky -= kyMax/2;

        // add ghosting by adding gradient delay induced offset
        if (m_Parameters->m_Misc.m_DoAddGhosts)
        {
          if (out_idx[1]%2 == 1)
            kx -= m_Parameters->m_SignalGen.m_KspaceLineOffset;
          else
            kx += m_Parameters->m_SignalGen.m_KspaceLineOffset;
        }

        // pull stuff out of inner loop
        t /= 1000;
        kx /= xMax;
        ky /= yMaxFov;
        auto yMaxFov_half = yMaxFov/2;

        // calculate signal s at k-space position (kx, ky)
        vcl_complex<ScalarType> s(0,0);
        InputIteratorType it(m_CompartmentImages[0], m_CompartmentImages[0]->GetLargestPossibleRegion() );
        while( !it.IsAtEnd() )
        {
          typename InputImageType::IndexType input_idx = it.GetIndex();

          // shift x,y for DFT: (0 -- N) --> (-N/2 -- N/2)
          float x = input_idx[0];
          float y = input_idx[1];
          if (static_cast<int>(xMax)%2==1)
            x -= (xMax-1)/2;
          else
            x -= xMax/2;
          if (static_cast<int>(yMax)%2==1)
            y -= (yMax-1)/2;
          else
            y -= yMax/2;

          // sum compartment signals and simulate relaxation
          ScalarType f_real = 0;
          for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
            if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
              f_real += m_CompartmentImages[i]->GetPixel(input_idx) * relaxFactor[i] *  m_Parameters->m_SignalGen.m_SignalScale;
            else
              f_real += m_CompartmentImages[i]->GetPixel(input_idx) * m_Parameters->m_SignalGen.m_SignalScale;

          // vector from image center to current position (in meter)
          // only necessary for eddy currents and non-constant coil sensitivity
          VectorType pos;
          if ((m_Parameters->m_Misc.m_DoAddEddyCurrents && m_Parameters->m_SignalGen.m_EddyStrength>0 && !m_IsBaseline) ||
              m_Parameters->m_SignalGen.m_CoilSensitivityProfile!=SignalGenerationParameters::COIL_CONSTANT)
          {
            pos[0] = x; pos[1] = y; pos[2] = m_Z;
            pos = m_Transform*pos;
          }

          if (m_Parameters->m_SignalGen.m_CoilSensitivityProfile!=SignalGenerationParameters::COIL_CONSTANT)
            f_real *= CoilSensitivity(pos);

          // simulate eddy currents and other distortions
          float omega = 0;   // frequency offset
          if (  m_Parameters->m_Misc.m_DoAddEddyCurrents && m_Parameters->m_SignalGen.m_EddyStrength>0 && !m_IsBaseline)
            omega += (m_DiffusionGradientDirection[0]*pos[0]+m_DiffusionGradientDirection[1]*pos[1]+m_DiffusionGradientDirection[2]*pos[2]) * eddyDecay;

          // simulate distortions
          if (m_Parameters->m_Misc.m_DoAddDistortions)
          {
            if (m_MovedFmap.IsNotNull())    // if we have headmotion, use moved map
              omega += m_MovedFmap->GetPixel(input_idx);
            else if (m_Parameters->m_SignalGen.m_FrequencyMap.IsNotNull())
            {
              itk::Image<float, 3>::IndexType index; index[0] = input_idx[0]; index[1] = input_idx[1]; index[2] = m_Zidx;
              omega += m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(index);
            }
          }

          // if signal comes from outside FOV, mirror it back (wrap-around artifact - aliasing
          if (m_Parameters->m_Misc.m_DoAddAliasing)
          {
            if (y<-yMaxFov_half)
              y += yMaxFov;
            else if (y>=yMaxFov_half)
              y -= yMaxFov;
          }

          // actual DFT term
          vcl_complex<ScalarType> f(f_real, 0);
          s += f * std::exp( std::complex<ScalarType>(0, itk::Math::twopi * (kx*x + ky*y + omega*t )) );

          ++it;
        }
        s /= numPix;

        if (m_SpikesPerSlice>0 && sqrt(s.imag()*s.imag()+s.real()*s.real()) > sqrt(m_Spike.imag()*m_Spike.imag()+m_Spike.real()*m_Spike.real()) )
          m_Spike = s;

        if (m_Parameters->m_SignalGen.m_NoiseVariance>0 && m_Parameters->m_Misc.m_DoAddNoise)
          s = vcl_complex<ScalarType>(s.real()+randGen->GetNormalVariate(0,noiseVar), s.imag()+randGen->GetNormalVariate(0,noiseVar));

        outputImage->SetPixel(kIdx, s);
        m_KSpaceImage->SetPixel(kIdx, sqrt(s.imag()*s.imag()+s.real()*s.real()) );
      }

      ++oit;
    }
  }

  template< class ScalarType >
  void KspaceImageFilter< ScalarType >
  ::AfterThreadedGenerateData()
  {
    delete m_ReadoutScheme;

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    int kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);  // k-space size in x-direction
    int kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);  // k-space size in y-direction

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    while( !oit.IsAtEnd() ) // use hermitian k-space symmetry to fill empty k-space parts resulting from partial fourier acquisition
    {
      itk::Index< 2 > kIdx;
      kIdx[0] = oit.GetIndex()[0];
      kIdx[1] = oit.GetIndex()[1];

      // reverse phase
      if (!m_Parameters->m_SignalGen.m_ReversePhase)
        kIdx[1] = static_cast<int>(kyMax-1-kIdx[1]);

      if (kIdx[1]>kyMax*m_Parameters->m_SignalGen.m_PartialFourier)
      {
        // reverse readout direction
        if (oit.GetIndex()[1]%2 == 1)
          kIdx[0] = kxMax-kIdx[0]-1;

        // calculate symmetric index
        itk::Index< 2 > kIdx2;
        kIdx2[0] = (kxMax-kIdx[0]-kxMax%2)%kxMax;
        kIdx2[1] = (kyMax-kIdx[1]-kyMax%2)%kyMax;

        // use complex conjugate of symmetric index value at current index
        vcl_complex<ScalarType> s = outputImage->GetPixel(kIdx2);
        s = vcl_complex<ScalarType>(s.real(), -s.imag());
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
      spikeIdx[0] = randGen->GetIntegerVariate()%kxMax;
      spikeIdx[1] = randGen->GetIntegerVariate()%kyMax;
      outputImage->SetPixel(spikeIdx, m_Spike);
      m_SpikeLog += "[" + boost::lexical_cast<std::string>(spikeIdx[0]) + "," + boost::lexical_cast<std::string>(spikeIdx[1]) + "," + boost::lexical_cast<std::string>(m_Zidx) + "] Magnitude: " + boost::lexical_cast<std::string>(m_Spike.real()) + "+" + boost::lexical_cast<std::string>(m_Spike.imag()) + "i\n";
    }
  }
}
#endif
