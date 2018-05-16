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

#ifndef __itkAdcImageFilter_txx
#define __itkAdcImageFilter_txx

#include <ctime>
#include <cstdio>
#include <cstdlib>

#define _USE_MATH_DEFINES
#include <cmath>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"

namespace itk {


template< class TInPixelType, class TOutPixelType >
AdcImageFilter< TInPixelType, TOutPixelType>
::AdcImageFilter()
  : m_FitSignal(true)
  , m_B_value(0)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType, class TOutPixelType >
void
AdcImageFilter< TInPixelType, TOutPixelType>
::BeforeThreadedGenerateData()
{
  typename OutputImageType::Pointer outputImage =
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
  outputImage->FillBuffer(0.0);

  m_B_values.set_size(m_GradientDirections->Size());
  int nonzero = 0;
  for (unsigned int i=0; i<m_GradientDirections->Size(); i++)
  {
    GradientDirectionType g = m_GradientDirections->GetElement(i);
    double twonorm = g.two_norm();
    double b = m_B_value*twonorm*twonorm;
    m_B_values[i] = b;

    if (g.magnitude()>0.001)
      nonzero++;
  }

  m_Nonzero_B_values.set_size(nonzero);
  nonzero = 0;
  for (unsigned int i=0; i<m_B_values.size(); i++)
  {
    if (m_B_values[i]>1)
    {
      m_Nonzero_B_values[nonzero] = m_B_values[i];
      nonzero++;
    }
  }

  if (m_B_values.size() == m_Nonzero_B_values.size())
    mitkThrow() << "Unweighted (b=0 s/mm²) image volume missing!";

}

template< class TInPixelType, class TOutPixelType >
double
AdcImageFilter< TInPixelType, TOutPixelType>::FitSingleVoxel( const typename InputImageType::PixelType &input)
{
  double S0 = 0;
  int nonzero = 0;
  vnl_vector<double> m; m.set_size(m_Nonzero_B_values.size());
  for (unsigned int i=0; i<m_B_values.size(); i++)
  {
    if (m_B_values[i]>1)
    {
      m[nonzero] = input[i];
      nonzero++;
    }
    else
      S0 += input[i];
  }
  S0 /= (m_B_values.size() - m_Nonzero_B_values.size());

  adcLeastSquaresFunction f(m_Nonzero_B_values.size());
  f.set_bvalues(m_Nonzero_B_values);
  f.set_S0(S0);
  f.set_measurements(m);

  vnl_levenberg_marquardt lm(f);

  vnl_vector_fixed<double, 1> x; x.fill(0);
  lm.minimize(x);

  return x[0];
}

template< class TInPixelType, class TOutPixelType >
void
AdcImageFilter< TInPixelType, TOutPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  typename OutputImageType::Pointer outputImage =
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
  typename InputImageType::Pointer inputImagePointer = NULL;
  inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  InputIteratorType git( inputImagePointer, outputRegionForThread );
  git.GoToBegin();
  while( !git.IsAtEnd() )
  {
    typename InputImageType::PixelType pix = git.Get();
    TOutPixelType outval = 0;

    if (!m_FitSignal)
    {
      double S0 = 0;
      int c = 0;
      for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
      {
        GradientDirectionType g = m_GradientDirections->GetElement(i);
        if (g.magnitude()<0.001)
        {
          if (pix[i]>0)
          {
            S0 += pix[i];
            c++;
          }
        }
      }
      if (c>0)
        S0 /= c;

      if (S0>0)
      {
        c = 0;
        for (unsigned int i=0; i<inputImagePointer->GetVectorLength(); i++)
        {
          GradientDirectionType g = m_GradientDirections->GetElement(i);
          if (g.magnitude()>0.001)
          {
            double twonorm = g.two_norm();
            double b = m_B_value*twonorm*twonorm;
            if (b>0)
            {
              double S = pix[i];
              if (S>0 && S0>0)
              {
                outval -= std::log(S/S0)/b;
                c++;
              }
            }
          }
        }

        if (c>0)
          outval /= c;
      }
    }
    else
    {
      outval = FitSingleVoxel(pix);
    }

    //        if (outval<-0.00001)
    //          outval = -0.00001;

    if (outval==outval && outval<1000)
      oit.Set( outval );
    //        else
    //          oit.Set( -0.00001 );

    ++oit;
    ++git;
  }
}

template< class TInPixelType, class TOutPixelType >
void
AdcImageFilter< TInPixelType, TOutPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}

#endif // __itkAdcImageFilter_txx
