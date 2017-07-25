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

#ifndef __itkBallAndSticksImageFilter_txx
#define __itkBallAndSticksImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include <itkAdcImageFilter.h>

namespace itk {


template< class TInPixelType, class TOutPixelType >
BallAndSticksImageFilter< TInPixelType, TOutPixelType>
::BallAndSticksImageFilter()
  : m_B_value(0)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType, class TOutPixelType >
void
BallAndSticksImageFilter< TInPixelType, TOutPixelType>
::BeforeThreadedGenerateData()
{
  typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
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


  itk::Vector<double, 3> spacing3 = outputImage->GetSpacing();
  itk::Point<float, 3> origin3 = outputImage->GetOrigin();
  itk::Matrix<double, 3, 3> direction3 = outputImage->GetDirection();
  itk::ImageRegion<3> imageRegion3 = outputImage->GetLargestPossibleRegion();

  itk::Vector<double, 4> spacing4;
  itk::Point<float, 4> origin4;
  itk::Matrix<double, 4, 4> direction4;
  itk::ImageRegion<4> imageRegion4;

  spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
  origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin4[3] = 0;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction4[r][c] = direction3[r][c];
  direction4[3][3] = 1;
  imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
  imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
  imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
  imageRegion4.SetSize(3, 3);

  m_PeakImage = PeakImageType::New();
  m_PeakImage->SetSpacing( spacing4 );
  m_PeakImage->SetOrigin( origin4 );
  m_PeakImage->SetDirection( direction4 );
  m_PeakImage->SetRegions( imageRegion4 );
  m_PeakImage->Allocate();
  m_PeakImage->FillBuffer(0.0);
}

template< class TInPixelType, class TOutPixelType >
vnl_vector<double>
BallAndSticksImageFilter< TInPixelType, TOutPixelType>::FitSingleVoxel( const typename InputImageType::PixelType &input)
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

  double adc = 0.001;
  {
    typename itk::AdcImageFilter< TInPixelType, TOutPixelType>::adcLeastSquaresFunction adc_fit(m_Nonzero_B_values.size());
    adc_fit.set_bvalues(m_Nonzero_B_values);
    adc_fit.set_S0(S0);
    adc_fit.set_measurements(m);

    vnl_levenberg_marquardt lm_adc(adc_fit);
    vnl_vector_fixed<double, 1> x; x.fill(0);
    lm_adc.minimize(x);
    adc = x[0];
  }

  int num_params = 5;

  ballStickLeastSquaresFunction bs_fit(num_params, m_Nonzero_B_values.size());
  bs_fit.set_bvalues(m_Nonzero_B_values);
  bs_fit.set_S0(S0);
  bs_fit.set_gradient_directions(m_GradientDirections);
  bs_fit.set_measurements(m);
  vnl_levenberg_marquardt lm(bs_fit);

  vnl_vector<double> x; x.set_size(num_params); x.fill(0.0); x[0]=0.5; x[1]=0.001;
  lm.minimize(x);

  return x;
}

template< class TInPixelType, class TOutPixelType >
void
BallAndSticksImageFilter< TInPixelType, TOutPixelType>
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
    vnl_vector<double> outval = FitSingleVoxel(pix);
    oit.Set( outval[0] );

    PeakImageType::IndexType idx4;
    idx4[0] = oit.GetIndex()[0];
    idx4[1] = oit.GetIndex()[1];
    idx4[2] = oit.GetIndex()[2];

    // TODO: SphericalToCartesian
    idx4[3] = 0;
    m_PeakImage->SetPixel(idx4, outval[3]);
    idx4[3] = 1;
    m_PeakImage->SetPixel(idx4, outval[4]);
    idx4[3] = 2;
    m_PeakImage->SetPixel(idx4, outval[5]);

    ++oit;
    ++git;
  }

  std::cout << "One Thread finished calculation" << std::endl;
}

template< class TInPixelType, class TOutPixelType >
void
BallAndSticksImageFilter< TInPixelType, TOutPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}

#endif // __itkBallAndSticksImageFilter_txx
