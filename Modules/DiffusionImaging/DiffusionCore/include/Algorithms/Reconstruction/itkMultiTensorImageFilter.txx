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

#ifndef __itkMultiTensorImageFilter_txx
#define __itkMultiTensorImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include <itkDiffusionTensor3D.h>
#include <mitkMultiTensorFitter.h>

namespace itk {


template< class TInPixelType, class TOutPixelType >
MultiTensorImageFilter< TInPixelType, TOutPixelType>
::MultiTensorImageFilter()
  : m_B_value(0)
  , m_NumTensors(2)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template< class TInPixelType, class TOutPixelType >
void
MultiTensorImageFilter< TInPixelType, TOutPixelType>
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
  imageRegion4.SetSize(3, 3*m_NumTensors);

  m_PeakImage = PeakImageType::New();
  m_PeakImage->SetSpacing( spacing4 );
  m_PeakImage->SetOrigin( origin4 );
  m_PeakImage->SetDirection( direction4 );
  m_PeakImage->SetRegions( imageRegion4 );
  m_PeakImage->Allocate();
  m_PeakImage->FillBuffer(0.0);

  m_TensorImages.clear();
  for (int i=0; i<m_NumTensors; i++)
  {
    TensorImageType::Pointer tImg = TensorImageType::New();
    tImg->SetSpacing( spacing3 );
    tImg->SetOrigin( origin3 );
    tImg->SetDirection( direction3 );
    tImg->SetRegions( imageRegion3 );
    tImg->Allocate();
    m_TensorImages.push_back(tImg);
  }
}

template< class TInPixelType, class TOutPixelType >
vnl_vector<double>
MultiTensorImageFilter< TInPixelType, TOutPixelType>::FitSingleVoxel( const typename InputImageType::PixelType &input)
{
  double S0 = 0;
  for (auto i : m_UnWeightedIndices)
    S0 += input[i];
  S0 /= m_UnWeightedIndices.size();

  // signle tensor fit
  mitk::MultiTensorFitter ls_fit(1, m_GradientDirections->size());
  ls_fit.set_S0(S0);
  ls_fit.set_weightedIndices(m_WeightedIndices);
  ls_fit.set_bvalues(m_B_values);
  ls_fit.set_gradient_directions(m_GradientDirections);
  ls_fit.set_measurements(input);

  vnl_levenberg_marquardt lm(ls_fit);
  vnl_vector<double> x;
  x.set_size(6);
  x.fill(0.0);

  lm.minimize(x);

//  TensorType tensor;

  vnl_vector<double> y;
  y.set_size(7*m_NumTensors);
  y.fill(0.0);
  for (int i=0; i<6; i++)
  {
    y[i] = x[i];
//    tensor[i] = x[i];
  }
  y[6]=1;
//  double fa = tensor.GetFractionalAnisotropy();

  if (m_NumTensors>1)
  {
    mitk::MultiTensorFitter ls_fit(m_NumTensors, m_GradientDirections->size());
    ls_fit.set_S0(S0);
    ls_fit.set_weightedIndices(m_WeightedIndices);
    ls_fit.set_bvalues(m_B_values);
    ls_fit.set_gradient_directions(m_GradientDirections);
    ls_fit.set_measurements(input);

    vnl_levenberg_marquardt lm(ls_fit);

    for (int i=0; i<m_NumTensors; i++)
      y[6+i*7] = 1.0/m_NumTensors;

    lm.minimize(y);
  }

  return y;
}

template< class TInPixelType, class TOutPixelType >
void
MultiTensorImageFilter< TInPixelType, TOutPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
  typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  InputIteratorType git( inputImagePointer, outputRegionForThread );
  git.GoToBegin();
  while( !git.IsAtEnd() )
  {
    typename InputImageType::PixelType pix = git.Get();
    vnl_vector<double> x = FitSingleVoxel(pix);

    typedef itk::DiffusionTensor3D<float>    TensorType;

    int elements = 7;
    for (int t=0; t<m_NumTensors; t++)
    {
      TensorType tensor;
      double f = x[6+t*elements];
      for (int i=0; i<6; i++)
        tensor[i] = f * x[i+t*elements];
      m_TensorImages.at(t)->SetPixel(git.GetIndex(), tensor);
    }

//    TensorType tensor;
//    tensor.Fill(0.0);
//    tensor[0] = x[0]*x[1];
//    tensor[3] = tensor[0];
//    tensor[5] = tensor[0];
//    m_TensorImages.at(m_NumTensors+1)->SetPixel(git.GetIndex(), tensor);

//    /// DWI from tensor
//    int elements = 7;
//    int num_ten = m_NumTensors;
//    typename DiffusionImageType::PixelType dPix; dPix.SetSize(m_GradientDirections->Size()); dPix.Fill(0.0);
//    for (unsigned int i=0; i<m_GradientDirections->Size(); i++)
//    {
//      GradientDirectionType g = m_GradientDirections->GetElement(i);
//      double twonorm = g.two_norm();
//      double b = m_B_value*twonorm*twonorm;

//      itk::DiffusionTensor3D< double > S;
//      S[0] = g[0]*g[0];
//      S[1] = g[1]*g[0];
//      S[2] = g[2]*g[0];
//      S[3] = g[1]*g[1];
//      S[4] = g[2]*g[1];
//      S[5] = g[2]*g[2];

//      double approx = 0;
//      for (int i=0; i<num_ten; i++)
//      {
//        double D = x[0+i*elements]*S[0] + x[1+i*elements]*S[1] + x[2+i*elements]*S[2] +
//                   x[1+i*elements]*S[1] + x[3+i*elements]*S[3] + x[4+i*elements]*S[4] +
//                   x[2+i*elements]*S[2] + x[4+i*elements]*S[4] + x[5+i*elements]*S[5];
//        approx += x[elements-1+i*elements] * 1000 * std::exp ( -b * D );
//      }

//      dPix[i] = approx;
//    }
//    m_OutDwi->SetPixel(oit.GetIndex(), dPix);

    ++git;
  }

  std::cout << "One Thread finished calculation" << std::endl;
}

template< class TInPixelType, class TOutPixelType >
void
MultiTensorImageFilter< TInPixelType, TOutPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}

#endif // __itkMultiTensorImageFilter_txx
