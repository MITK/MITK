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

#ifndef __itkNonLocalMeansDenoisingFilter_txx
#define __itkNonLocalMeansDenoisingFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodIterator.h"


namespace itk {


template< class TInPixelType, class TOutPixelType >
NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>
::NonLocalMeansDenoisingFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType, class TOutPixelType >
void
NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>
::BeforeThreadedGenerateData()
{
    typename OutputImageType::Pointer outputImage =
            static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    typename OutputImageType::PixelType px;
    px.SetSize(1);
    px.SetElement(0,0);
    outputImage->FillBuffer(px);
}

template< class TInPixelType, class TOutPixelType >
void
NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
    typename OutputImageType::Pointer outputImage =
            static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typedef ConstNeighborhoodIterator <InputImageType> InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = NULL;
    inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    InputIteratorType git(m_V_Radius, inputImagePointer, outputRegionForThread );
    InputIteratorType njit(m_N_Radius, inputImagePointer, outputRegionForThread );
    InputIteratorType niit(m_N_Radius, inputImagePointer, outputRegionForThread );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
      typename OutputImageType::PixelType outpix;
      outpix.SetSize (inputImagePointer->GetVectorLength());

      for (int i = 0; i < inputImagePointer->GetVectorLength(); ++i)
      {
        TInPixelType xi = git.GetCenterPixel()[i];
        double sumj = 0;
        double wj = 0;
        for (int j = 0; j < git.Size(); ++j)
        {
          TInPixelType xj = git.GetPixel(j)[i];

          if (xi == xj && git.Size()/2 != j)
          {
            niit.SetLocation(git.GetIndex());
            njit.SetLocation(git.GetIndex(j));
            double sumk = 0;
            for (int k = 0; k < niit.Size(); ++k)
            {
              sumk += std::pow( (niit.GetPixel(k)[i] - njit.GetPixel(k)[i]), 2);
            }
            sumk = sumk >= 0 ? sumk : 0;
            wj = std::exp( - ( std::sqrt( (1 / niit.Size()) * sumk) / m_H));
            sumj += wj * std::pow(xj, 2) - 2 * std::pow(m_H, 2);
          }
        }
        sumj = sumj >= 0 ? sumj : 0;
        TOutPixelType outval = xi - static_cast<TOutPixelType>(std::sqrt(sumj));
        outpix.SetElement(i, outval);
      }
      oit.Set(outpix);
      ++oit;
      ++git;
    }

    std::cout << "One Thread finished calculation" << std::endl;
}

template< class TInPixelType, class TOutPixelType >
void NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>::PrintSelf(std::ostream& os, Indent indent) const
{
    Superclass::PrintSelf(os,indent);
}

template < class TInPixelType, class TOutPixelType >
void NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>::SetNRadius(unsigned int n)
{
  for (int i = 0; i < InputImageType::ImageDimension; ++i)
  {
    m_N_Radius [i] = n;
  }
}

template < class TInPixelType, class TOutPixelType >
void NonLocalMeansDenoisingFilter< TInPixelType, TOutPixelType>::SetVRadius(unsigned int v)
{
  for (int i = 0; i < InputImageType::ImageDimension; ++i)
  {
    m_V_Radius [i] = v;
  }
}

}

#endif // __itkNonLocalMeansDenoisingFilter_txx
