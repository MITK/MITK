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

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "itkDftImageFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

namespace itk {

template< class TPixelType >
DftImageFilter< TPixelType >
::DftImageFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TPixelType >
void DftImageFilter< TPixelType >
::BeforeThreadedGenerateData()
{

}

template< class TPixelType >
void DftImageFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType)
{
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);

  typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
  typename InputImageType::Pointer inputImage  = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  float szx = outputImage->GetLargestPossibleRegion().GetSize(0);
  float szy = outputImage->GetLargestPossibleRegion().GetSize(1);

  float x_shift = 0;
  float y_shift = 0;
  if (static_cast<int>(szx)%2==1)
      x_shift = (szx-1)/2;
  else
      x_shift = szx/2;
  if (static_cast<int>(szy)%2==1)
      y_shift = (szy-1)/2;
  else
      y_shift = szy/2;

  while( !oit.IsAtEnd() )
  {
    float kx = oit.GetIndex()[0] - x_shift;
    float ky = oit.GetIndex()[1] - y_shift;
    kx /= szx;
    ky /= szy;

    vcl_complex<TPixelType> s(0,0);
    InputIteratorType it(inputImage, inputImage->GetLargestPossibleRegion() );
    while( !it.IsAtEnd() )
    {
      float x = it.GetIndex()[0] - x_shift;
      float y = it.GetIndex()[1] - y_shift;

      s += it.Get() * exp( std::complex<TPixelType>(0, -itk::Math::twopi * (kx*x + ky*y) ) );
      ++it;
    }

    oit.Set(s);
    ++oit;
  }
}

}
#endif
