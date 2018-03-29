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

#ifndef __itkTdiToVolumeFractionFilter_txx
#define __itkTdiToVolumeFractionFilter_txx

#include "itkTdiToVolumeFractionFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <math.h>

namespace itk {

template< class TPixelType >
TdiToVolumeFractionFilter< TPixelType >::TdiToVolumeFractionFilter()
  : m_TdiThreshold(0.3)
  , m_Sqrt(1.0)
{
  this->SetNumberOfRequiredInputs(5);
  this->SetNumberOfRequiredOutputs(4);
}

template< class TPixelType >
void TdiToVolumeFractionFilter< TPixelType >::BeforeThreadedGenerateData()
{
  typename InputImageType::Pointer input_tdi  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  for (int i=0; i<4; ++i)
  {
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetOrigin( input_tdi->GetOrigin() );
    outputImage->SetRegions( input_tdi->GetLargestPossibleRegion() );
    outputImage->SetSpacing( input_tdi->GetSpacing() );
    outputImage->SetDirection( input_tdi->GetDirection() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);
    this->SetNthOutput(i, outputImage);
  }
}

template< class TPixelType >
void TdiToVolumeFractionFilter< TPixelType >::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType)
{
  typename OutputImageType::Pointer o_intra_ax = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
  typename OutputImageType::Pointer o_inter_ax = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(1));
  typename OutputImageType::Pointer o_gray_matter = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(2));
  typename OutputImageType::Pointer o_csf = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(3));

  ImageRegionIterator< OutputImageType > oit0(o_intra_ax, outputRegionForThread);
  ImageRegionIterator< OutputImageType > oit1(o_inter_ax, outputRegionForThread);
  ImageRegionIterator< OutputImageType > oit2(o_gray_matter, outputRegionForThread);
  ImageRegionIterator< OutputImageType > oit3(o_csf, outputRegionForThread);

  typename InputImageType::Pointer i_wm = static_cast< InputImageType * >(this->ProcessObject::GetInput(1));
  typename InputImageType::Pointer i_gm = static_cast< InputImageType * >(this->ProcessObject::GetInput(2));
  typename InputImageType::Pointer i_dgm = static_cast< InputImageType * >(this->ProcessObject::GetInput(3));
  typename InputImageType::Pointer i_csf = static_cast< InputImageType * >(this->ProcessObject::GetInput(4));

  ImageRegionIterator< OutputImageType > iit0(i_wm, outputRegionForThread);
  ImageRegionIterator< OutputImageType > iit1(i_gm, outputRegionForThread);
  ImageRegionIterator< OutputImageType > iit2(i_dgm, outputRegionForThread);
  ImageRegionIterator< OutputImageType > iit3(i_csf, outputRegionForThread);

  typename InputImageType::Pointer input_tdi  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
  ImageRegionConstIterator< InputImageType > tdi_it(input_tdi, outputRegionForThread);

  while( !tdi_it.IsAtEnd() )
  {
    TPixelType intra_ax_val = tdi_it.Get();
    intra_ax_val = std::pow(intra_ax_val, 1.0/m_Sqrt);
    if (intra_ax_val>m_TdiThreshold)
      intra_ax_val = m_TdiThreshold;
    intra_ax_val /= m_TdiThreshold;
    if (intra_ax_val<0)
      intra_ax_val = 0;

    TPixelType inter_ax_val = 0.0;
    TPixelType wm_val = iit0.Get();
    TPixelType gm_val = iit1.Get();
    TPixelType dgm_val = iit2.Get();
    TPixelType csf_val = iit3.Get();

    if (dgm_val>0)
    {
      TPixelType ex = 1.0 - intra_ax_val;
      gm_val += dgm_val * ex;
    }

    if (intra_ax_val < 0.0001 && wm_val > 0.0001)
    {
      if (csf_val+gm_val == 0 )
        gm_val += wm_val;
      else
      {
        if (csf_val>gm_val)
          csf_val += wm_val;
        else
          gm_val += wm_val;
      }
    }
    else if ( intra_ax_val>0.0001 )
    {
      auto comp_sum = csf_val+gm_val+intra_ax_val;
      if (comp_sum<1.0)
          inter_ax_val = 1.0 - comp_sum;
      else if (comp_sum>1)
          intra_ax_val = 1.0 - (csf_val+gm_val);
      if (intra_ax_val<-0.01)
        MITK_INFO << "Corrupted volume fraction. intra_ax_val=" << intra_ax_val;
      if (intra_ax_val<0)
        intra_ax_val = 0;
    }
    auto comp_sum = csf_val+gm_val+intra_ax_val+inter_ax_val;
    if (std::fabs(comp_sum-1.0)>0.00001 && comp_sum > 0.0001)
    {
      csf_val /= comp_sum;
      gm_val /= comp_sum;
      intra_ax_val /= comp_sum;
      inter_ax_val /= comp_sum;
    }

    oit0.Set(intra_ax_val);
    oit1.Set(inter_ax_val);
    oit2.Set(gm_val);
    oit3.Set(csf_val);

    ++tdi_it;

    ++iit0;
    ++iit1;
    ++iit2;
    ++iit3;

    ++oit0;
    ++oit1;
    ++oit2;
    ++oit3;
  }
}

}
#endif
