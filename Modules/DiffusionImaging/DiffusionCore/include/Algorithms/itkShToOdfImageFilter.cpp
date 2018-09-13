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
#ifndef __itkShToOdfImageFilter_cpp
#define __itkShToOdfImageFilter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkShToOdfImageFilter.h"
#include <itkImageRegionIterator.h>
#include <mitkDiffusionFunctionCollection.h>

namespace itk {

template< class PixelType, int ShOrder >
ShToOdfImageFilter< PixelType, ShOrder >::ShToOdfImageFilter()
  : m_Toolkit(Toolkit::MRTRIX)
{

}

template< class PixelType, int ShOrder >
void ShToOdfImageFilter< PixelType, ShOrder >::BeforeThreadedGenerateData()
{
  CalcShBasis();
}

template< class PixelType, int ShOrder >
void ShToOdfImageFilter< PixelType, ShOrder >::ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType)
{
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

  typename InputImageType::Pointer inputImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
  InputIteratorType it(inputImage, outputRegionForThread);

  typedef ImageRegionIterator< OutputImageType > OutputIteratorType;
  OutputIteratorType oit(outputImage, outputRegionForThread);

  while(!it.IsAtEnd())
  {
    auto pix = it.Get();
    vnl_vector<PixelType> coeffs = pix.GetVnlVector();
    OutputPixelType odf = (m_ShBasis * coeffs).data_block();
    oit.Set(odf);

    ++it;
    ++oit;
  }
}

// generate spherical harmonic values of the desired order for each input direction
template< class PixelType, int ShOrder >
void ShToOdfImageFilter< PixelType, ShOrder >::CalcShBasis()
{
  vnl_matrix_fixed<double, 3, ODF_SAMPLING_SIZE>* U = itk::PointShell<ODF_SAMPLING_SIZE, vnl_matrix_fixed<double, 3, ODF_SAMPLING_SIZE> >::DistributePointShell();
  if (m_Toolkit==Toolkit::MRTRIX)
    m_ShBasis = mitk::sh::CalcShBasisForDirections(ShOrder, U->as_matrix());
  else
    m_ShBasis = mitk::sh::CalcShBasisForDirections(ShOrder, U->as_matrix(), false);
}

}

#endif // __itkShToOdfImageFilter_cpp
