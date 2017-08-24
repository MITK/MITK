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
#include <mitkBallStickFitter.h>


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

  m_UnWeightedIndices.clear();
  m_WeightedIndices.clear();
  m_B_values.clear();

  for (unsigned int i=0; i<m_GradientDirections->Size(); i++)
  {
    GradientDirectionType g = m_GradientDirections->GetElement(i);
    double twonorm = g.two_norm();
    double b = m_B_value*twonorm*twonorm;
    m_B_values.push_back(b);

    if (b>100)
      m_WeightedIndices.push_back(i);
    else
      m_UnWeightedIndices.push_back(i);
  }

  if (m_UnWeightedIndices.empty())
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

  m_OutDwi = InputImageType::New();
  m_OutDwi->SetSpacing( spacing3 );
  m_OutDwi->SetOrigin( origin3 );
  m_OutDwi->SetDirection( direction3 );
  m_OutDwi->SetRegions( imageRegion3 );
  m_OutDwi->SetVectorLength(m_GradientDirections->Size());
  m_OutDwi->Allocate();

  MITK_INFO << "Initial tensor fit";
  typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
  typename TensorRecFilterType::Pointer tensorReconstructionFilter = TensorRecFilterType::New();
  tensorReconstructionFilter->SetBValue( m_B_value );
  tensorReconstructionFilter->SetGradientImage( m_GradientDirections, inputImagePointer );
  tensorReconstructionFilter->Update();
  m_TensorImage = tensorReconstructionFilter->GetOutput();
}

template< class TInPixelType, class TOutPixelType >
vnl_vector<double>
BallAndSticksImageFilter< TInPixelType, TOutPixelType>::FitSingleVoxel( const typename InputImageType::PixelType &input, const typename InputImageType::IndexType &idx)
{
  double S0 = 0;
  for (auto i : m_UnWeightedIndices)
    S0 += input[i];
  S0 /= m_UnWeightedIndices.size();

  // Linear tensor fit
  double md = 0.001;
  double f = 0.5;
  double theta = 0;
  double phi = 0;
  {
    TensorType tensor = m_TensorImage->GetPixel(idx);
    TensorType::EigenValuesArrayType eigenvalues;
    TensorType::EigenVectorsMatrixType eigenvectors;
    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
    vnl_vector_fixed<double,3> ev;
    ev[0] = eigenvectors(2, 0);
    ev[1] = eigenvectors(2, 1);
    ev[2] = eigenvectors(2, 2);
    if (ev.magnitude()>mitk::eps)
      ev.normalize();
    else
      ev.fill(0.0);

    md = fabs(eigenvalues[0]+eigenvalues[1]+eigenvalues[2])/3;
    f = tensor.GetFractionalAnisotropy();
    if (f<0.1)
      f = 0.1;
    else if (f>0.9)
      f = 0.9;

    mitk::AbstractFitter::Cart2Sph(ev, theta, phi);
  }

  int num_params = 4; // f, d, phi, theta

  vnl_vector<double> x; x.set_size(num_params); x.fill(0.0);
  x[0] = f; // f (volume fraction)
  x[1] = md; // d
  x[2] = theta;
  x[3] = phi;

  mitk::BallStickFitter bs_fit(num_params, m_GradientDirections->size());
  bs_fit.set_S0(S0);
  bs_fit.set_weightedIndices(m_WeightedIndices);
  bs_fit.set_bvalues(m_B_values);
  bs_fit.set_gradient_directions(m_GradientDirections);
  bs_fit.set_measurements(input);

  vnl_levenberg_marquardt lm(bs_fit);
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
  typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  InputIteratorType git( inputImagePointer, outputRegionForThread );
  git.GoToBegin();
  while( !git.IsAtEnd() )
  {
    typename InputImageType::PixelType pix = git.Get();
    vnl_vector<double> x = FitSingleVoxel(pix, git.GetIndex());

    /// PEAKS
    PeakImageType::IndexType idx4;
    idx4[0] = oit.GetIndex()[0];
    idx4[1] = oit.GetIndex()[1];
    idx4[2] = oit.GetIndex()[2];

    oit.Set( x[0] );
    vnl_vector_fixed<double,3> dir;
    mitk::AbstractFitter::Sph2Cart(dir, x[2], x[3]);

    idx4[3] = 0;
    m_PeakImage->SetPixel(idx4, dir[0]);
    idx4[3] = 1;
    m_PeakImage->SetPixel(idx4, dir[1]);
    idx4[3] = 2;
    m_PeakImage->SetPixel(idx4, dir[2]);

    /// DWI from ball-stick
    typename InputImageType::PixelType dPix; dPix.SetSize(m_GradientDirections->Size()); dPix.Fill(0.0);
    for (unsigned int i=0; i<m_GradientDirections->Size(); i++)
    {
      GradientDirectionType g = m_GradientDirections->GetElement(i);
      double twonorm = g.two_norm();
      double b = m_B_value*twonorm*twonorm;
      g.normalize();

      double s_iso = 1000 * std::exp(-b * x[1]);

      double dot = dot_product(g, dir);
      double s_aniso = 1000 * std::exp(-b * x[1] * dot*dot );

      double approx = (1-x[0])*s_iso + x[0]*s_aniso;

      dPix[i] = approx;
    }
    m_OutDwi->SetPixel(oit.GetIndex(), dPix);

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
