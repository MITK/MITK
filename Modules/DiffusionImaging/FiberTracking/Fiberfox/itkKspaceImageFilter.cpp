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
#include <math.h>

#include "itkKspaceImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <mitkSingleShotEpi.h>
#include <mitkConventionalSpinEcho.h>
#include <mitkFastSpinEcho.h>
#include <mitkDiffusionFunctionCollection.h>
#include <itkImageFileWriter.h>

namespace itk {

  template< class ScalarType >
  KspaceImageFilter< ScalarType >::KspaceImageFilter()
    : m_Z(0)
    , m_RandSeed(-1)
    , m_SpikesPerSlice(0)
    , m_IsBaseline(true)
    , m_StoreTimings(false)
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

    kxMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0);
    kyMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1);
    xMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(0); // scanner coverage in x-direction
    yMax = m_CompartmentImages.at(0)->GetLargestPossibleRegion().GetSize(1); // scanner coverage in y-direction
    yMaxFov = yMax;
    if (m_Parameters->m_Misc.m_DoAddAliasing)
    {
      // actual FOV in y-direction (in x-direction FOV=xMax)
      yMaxFov = static_cast<int>(yMaxFov * m_Parameters->m_SignalGen.m_CroppingFactor);
    }
    yMaxFov_half = (yMaxFov-1)/2;
    numPix = kxMax*kyMax;

    float ringing_factor = static_cast<float>(m_Parameters->m_SignalGen.m_ZeroRinging)/100.0;
    ringing_lines_x = static_cast<int>(ceil(kxMax/2 * ringing_factor));
    ringing_lines_y = static_cast<int>(ceil(kyMax/2 * ringing_factor));

    // Adjust noise variance since it is the intended variance in physical space and not in k-space:
    float noiseVar = m_Parameters->m_SignalGen.m_PartialFourier*m_Parameters->m_SignalGen.m_NoiseVariance/(kyMax*kxMax);

    m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    if (m_RandSeed>=0)  // always generate the same random numbers?
      m_RandGen->SetSeed(m_RandSeed);
    else
      m_RandGen->SetSeed();

    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    itk::ImageRegion<2> region;
    region.SetSize(0, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0));
    region.SetSize(1, m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1));
    outputImage->SetLargestPossibleRegion( region );
    outputImage->SetBufferedRegion( region );
    outputImage->SetRequestedRegion( region );
    outputImage->Allocate();
    vcl_complex<ScalarType> zero = vcl_complex<ScalarType>(0, 0);
    outputImage->FillBuffer(zero);
    if (m_Parameters->m_SignalGen.m_NoiseVariance>0 && m_Parameters->m_Misc.m_DoAddNoise)
    {
      ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
      while( !oit.IsAtEnd() )
      {
        oit.Set(vcl_complex<ScalarType>(m_RandGen->GetNormalVariate(0, noiseVar), m_RandGen->GetNormalVariate(0, noiseVar)));
        ++oit;
      }
    }

    m_KSpaceImage = InputImageType::New();
    m_KSpaceImage->SetLargestPossibleRegion( region );
    m_KSpaceImage->SetBufferedRegion( region );
    m_KSpaceImage->SetRequestedRegion( region );
    m_KSpaceImage->Allocate();
    m_KSpaceImage->FillBuffer(0.0);

    if (m_StoreTimings)
    {
        m_TickImage = InputImageType::New();
        m_TickImage->SetLargestPossibleRegion( region );
        m_TickImage->SetBufferedRegion( region );
        m_TickImage->SetRequestedRegion( region );
        m_TickImage->Allocate();
        m_TickImage->FillBuffer(-1.0);

        m_RfImage = InputImageType::New();
        m_RfImage->SetLargestPossibleRegion( region );
        m_RfImage->SetBufferedRegion( region );
        m_RfImage->SetRequestedRegion( region );
        m_RfImage->Allocate();
        m_RfImage->FillBuffer(-1.0);
    }
    else
    {
        m_TickImage = nullptr;
        m_RfImage = nullptr;
    }

    m_Gamma = 42576000*itk::Math::twopi;    // Gyromagnetic ratio in Hz/T
    if ( m_Parameters->m_SignalGen.m_EddyStrength>0 && m_DiffusionGradientDirection.GetNorm()>0.001)
    {
      m_DiffusionGradientDirection = m_DiffusionGradientDirection *  m_Parameters->m_SignalGen.m_EddyStrength/1000 *  m_Gamma;
      m_IsBaseline = false;
    }
    else {
      m_IsBaseline = true;
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
      case SignalGenerationParameters::ConventionalSpinEcho:
        m_ReadoutScheme = new mitk::ConventionalSpinEcho(m_Parameters);
      break;
      case SignalGenerationParameters::FastSpinEcho:
        m_ReadoutScheme = new mitk::FastSpinEcho(m_Parameters);
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

    // calculate T1 relaxation (independent of actual readout)
    m_T1Relax.clear();
    if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
      for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
      {
        // account for T1 relaxation (how much magnetization is available since last excitation?)
        float relaxation = (1.0-std::exp(-m_Parameters->m_SignalGen.m_tRep/m_T1[i]));

        // account for inversion pulse and TI
        if (m_Parameters->m_SignalGen.m_tInv > 0)
          relaxation *= (1.0-std::exp(std::log(2) - m_Parameters->m_SignalGen.m_tInv/m_T1[i]));

        m_T1Relax.push_back(relaxation);
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
  ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
  {
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

    // precalculate shifts for DFT
    float x_shift = 0;
    float y_shift = 0;
    if (static_cast<int>(xMax)%2==1)
        x_shift = (xMax-1)/2;
    else
        x_shift = xMax/2;
    if (static_cast<int>(yMax)%2==1)
        y_shift = (yMax-1)/2;
    else
        y_shift = yMax/2;

    float kx_shift = 0;
    float ky_shift = 0;
    if (static_cast<int>(kxMax)%2==1)
        kx_shift = (kxMax-1)/2;
    else
        kx_shift = kxMax/2;
    if (static_cast<int>(kyMax)%2==1)
        ky_shift = (kyMax-1)/2;
    else
        ky_shift = kyMax/2;

    vcl_complex<ScalarType> zero = vcl_complex<ScalarType>(0, 0);
    while( !oit.IsAtEnd() )
    {
      int tick = oit.GetIndex()[1] * kxMax + oit.GetIndex()[0];

      // get current k-space index (depends on the chosen k-space readout scheme)
      itk::Index< 2 > kIdx = m_ReadoutScheme->GetActualKspaceIndex(tick);

      // we have to adjust the ticks to obtain correct times since the DFT is not completely symmetric in the even  number of lines case
      if (static_cast<int>(kyMax)%2 == 0 && !m_Parameters->m_SignalGen.m_ReversePhase)
      {
        tick += kxMax;
        tick %= static_cast<int>(numPix);
      }

      // partial fourier
      // two cases because we always want to skip the "later" parts of k-space
      // in "normal" phase direction, the higher k-space indices are acquired first
      // in reversed phase direction, the higher k-space indices are acquired later
      // if the image has an even number of lines, never skip line zero since it is missing on the other side (DFT not completely syymetric in even case)
      if ((m_Parameters->m_SignalGen.m_ReversePhase && kIdx[1]>std::ceil(kyMax*m_Parameters->m_SignalGen.m_PartialFourier)) ||
          (!m_Parameters->m_SignalGen.m_ReversePhase && kIdx[1]<std::floor(kyMax*(1.0 - m_Parameters->m_SignalGen.m_PartialFourier)) &&
           (kIdx[1]>0 || static_cast<int>(kyMax)%2 == 1)))
      {
        outputImage->SetPixel(kIdx, zero);
        ++oit;
        continue;
      }
      if (m_StoreTimings)
        m_TickImage->SetPixel(kIdx, tick);

      // gibbs ringing by setting high frequencies to zero (alternative to using smaller k-space than input image space)
      if (m_Parameters->m_SignalGen.m_DoAddGibbsRinging && m_Parameters->m_SignalGen.m_ZeroRinging>0)
      {
        if (kIdx[0] < ringing_lines_x || kIdx[1] < ringing_lines_y ||
            kIdx[0] >= kxMax - ringing_lines_x || kIdx[1] >= kyMax - ringing_lines_y)
        {
          outputImage->SetPixel(kIdx, zero);
          ++oit;
          continue;
        }
      }

      // time from maximum echo
      float t = m_ReadoutScheme->GetTimeFromMaxEcho(tick);

      // calculate eddy current decay factor
      float eddyDecay = 0;
      if ( m_Parameters->m_Misc.m_DoAddEddyCurrents && m_Parameters->m_SignalGen.m_EddyStrength>0 && !m_IsBaseline)
      {
        // time passed since k-space readout started
        float tRead = m_ReadoutScheme->GetTimeFromLastDiffusionGradient(tick);
        eddyDecay = std::exp(-tRead/m_Parameters->m_SignalGen.m_Tau ) * t/1000; // time in seconds here
      }

      // calcualte signal relaxation factors
      std::vector< float > relaxFactor;
      if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
      {
        // time passes since application of the RF pulse
        float tRf = m_ReadoutScheme->GetTimeFromRf(tick);
        if (m_StoreTimings)
            m_RfImage->SetPixel(kIdx, tRf);

        for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
        {
          // account for T2 relaxation (how much transverse magnetization is left since applicatiohn of RF pulse?)
          relaxFactor.push_back(m_T1Relax[i] * std::exp(-tRf/m_T2[i] -fabs(t)/ m_Parameters->m_SignalGen.m_tInhom));
        }
      }

      // shift k for DFT: (0 -- N) --> (-N/2 -- N/2)
      float kx = kIdx[0] - kx_shift;
      float ky = kIdx[1] - ky_shift;

      // add ghosting by adding gradient delay induced offset
      if (m_Parameters->m_Misc.m_DoAddGhosts)
      {
        if (kIdx[1]%2 == 1)
          kx -= m_Parameters->m_SignalGen.m_KspaceLineOffset;
        else
          kx += m_Parameters->m_SignalGen.m_KspaceLineOffset;
      }

      // pull stuff out of inner loop
      t /= 1000; // time in seconds
      kx /= xMax;
      ky /= yMaxFov;

      // calculate signal s at k-space position (kx, ky)
      vcl_complex<ScalarType> s(0,0);
      InputIteratorType it(m_CompartmentImages[0], m_CompartmentImages[0]->GetLargestPossibleRegion() );
      while( !it.IsAtEnd() )
      {
        typename InputImageType::IndexType input_idx = it.GetIndex();

        // shift x,y for DFT: (0 -- N) --> (-N/2 -- N/2)
        float x = input_idx[0] - x_shift;
        float y = input_idx[1] - y_shift;

        // sum compartment signals and simulate relaxation
        ScalarType f_real = 0;
        for (unsigned int i=0; i<m_CompartmentImages.size(); i++)
          if ( m_Parameters->m_SignalGen.m_DoSimulateRelaxation)
            f_real += m_CompartmentImages[i]->GetPixel(input_idx) * relaxFactor[i];
          else
            f_real += m_CompartmentImages[i]->GetPixel(input_idx);

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
        float phi = 0;   // phase shift
        if (  m_Parameters->m_Misc.m_DoAddEddyCurrents && m_Parameters->m_SignalGen.m_EddyStrength>0 && !m_IsBaseline)
        {
          // duration (tRead) already included in "eddyDecay"
          phi += (m_DiffusionGradientDirection[0]*pos[0]+m_DiffusionGradientDirection[1]*pos[1]+m_DiffusionGradientDirection[2]*pos[2]) * eddyDecay;
        }

        // simulate distortions
        if (m_Parameters->m_Misc.m_DoAddDistortions)
        {
          if (m_MovedFmap.IsNotNull())    // if we have headmotion, use moved map
            phi += m_MovedFmap->GetPixel(input_idx) * t;
          else if (m_Parameters->m_SignalGen.m_FrequencyMap.IsNotNull())
          {
            itk::Image<float, 3>::IndexType index; index[0] = input_idx[0]; index[1] = input_idx[1]; index[2] = m_Zidx;
            phi += m_Parameters->m_SignalGen.m_FrequencyMap->GetPixel(index) * t;
          }
        }

        // if signal comes from outside FOV, mirror it back (wrap-around artifact - aliasing
        if (m_Parameters->m_Misc.m_DoAddAliasing)
        {
          if (y<-yMaxFov_half)
            y += yMaxFov;
          else if (y>yMaxFov_half)
            y -= yMaxFov;
        }

        // actual DFT term
        vcl_complex<ScalarType> f(f_real * m_Parameters->m_SignalGen.m_SignalScale, 0);
        s += f * std::exp( std::complex<ScalarType>(0, itk::Math::twopi * (kx*x + ky*y + phi )) );

        ++it;
      }
      s /= numPix;

      if (m_SpikesPerSlice>0 && sqrt(s.imag()*s.imag()+s.real()*s.real()) > sqrt(m_Spike.imag()*m_Spike.imag()+m_Spike.real()*m_Spike.real()) )
        m_Spike = s;

      s += outputImage->GetPixel(kIdx); // add precalculated noise
      outputImage->SetPixel(kIdx, s);
      m_KSpaceImage->SetPixel(kIdx, sqrt(s.imag()*s.imag()+s.real()*s.real()) );

      ++oit;
    }
  }

  template< class ScalarType >
  void KspaceImageFilter< ScalarType >
  ::AfterThreadedGenerateData()
  {
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    int kxMax = outputImage->GetLargestPossibleRegion().GetSize(0);  // k-space size in x-direction
    int kyMax = outputImage->GetLargestPossibleRegion().GetSize(1);  // k-space size in y-direction

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    while( !oit.IsAtEnd() ) // use hermitian k-space symmetry to fill empty k-space parts resulting from partial fourier acquisition
    {
      int tick = oit.GetIndex()[1] * kxMax + oit.GetIndex()[0];
      auto kIdx = m_ReadoutScheme->GetActualKspaceIndex(tick);

      if ((m_Parameters->m_SignalGen.m_ReversePhase && kIdx[1]>std::ceil(kyMax*m_Parameters->m_SignalGen.m_PartialFourier)) ||
          (!m_Parameters->m_SignalGen.m_ReversePhase && kIdx[1]<std::floor(kyMax*(1.0 - m_Parameters->m_SignalGen.m_PartialFourier)) &&
          (kIdx[1]>0 || static_cast<int>(kyMax)%2 == 1)))
      {
        // calculate symmetric index
        auto sym = m_ReadoutScheme->GetSymmetricIndex(kIdx);

        // use complex conjugate of symmetric index value at current index
        vcl_complex<ScalarType> s = outputImage->GetPixel(sym);
        s = vcl_complex<ScalarType>(s.real(), -s.imag());
        outputImage->SetPixel(kIdx, s);

        m_KSpaceImage->SetPixel(kIdx, sqrt(s.imag()*s.imag()+s.real()*s.real()) );
      }
      ++oit;
    }

    m_Spike *=  m_Parameters->m_SignalGen.m_SpikeAmplitude;
    itk::Index< 2 > spikeIdx;
    for (unsigned int i=0; i<m_SpikesPerSlice; i++)
    {
      spikeIdx[0] = m_RandGen->GetIntegerVariate()%kxMax;
      spikeIdx[1] = m_RandGen->GetIntegerVariate()%kyMax;
      outputImage->SetPixel(spikeIdx, m_Spike);
      m_SpikeLog += "[" + boost::lexical_cast<std::string>(spikeIdx[0]) + "," + boost::lexical_cast<std::string>(spikeIdx[1]) + "," + boost::lexical_cast<std::string>(m_Zidx) + "] Magnitude: " + boost::lexical_cast<std::string>(m_Spike.real()) + "+" + boost::lexical_cast<std::string>(m_Spike.imag()) + "i\n";
    }
    delete m_ReadoutScheme;
  }
}
#endif
