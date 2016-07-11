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


#ifndef DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_CXX
#define DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_CXX

#include "itkDiffusionKurtosisReconstructionImageFilter.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

template< class TInputPixelType>
static void FitSingleVoxel( const itk::VariableLengthVector< TInputPixelType > &input, const vnl_vector<double>& bvalues, vnl_vector<double>& result, bool omit_bzero = true)
{
  // check for length
  assert( input.Size() == bvalues.size() );

  // assembly data vectors for fitting
  auto bvalueIter = bvalues.begin();
  unsigned int unused_values = 0;
  while( bvalueIter != bvalues.end() )
  {
    if( *bvalueIter < vnl_math::eps && omit_bzero )
    {
      ++unused_values;
    }
    ++bvalueIter;
  }

  // initialize data vectors with the estimated size (after filtering)
  vnl_vector<double> fit_measurements( input.Size() - unused_values, 0 );
  vnl_vector<double> fit_bvalues( input.Size() - unused_values, 0 );

  bvalueIter = bvalues.begin();
  unsigned int running_index = 0;
  unsigned int skip_count = 0;
  while( bvalueIter != bvalues.end() )
  {
    if( *bvalueIter < vnl_math::eps && omit_bzero )
    {
      ++skip_count;
    }
    else
    {
      fit_measurements[ running_index ]  = input.GetElement(running_index + skip_count);
      fit_bvalues[ running_index ] = *bvalueIter;
    }

    ++running_index;
    ++bvalueIter;
  }


  // perform fit on data vectors
  if( omit_bzero )
  {
   itk::kurtosis_fit_omit_unweighted kurtosis_cost_fn( fit_measurements.size() );
   kurtosis_cost_fn.initialize( fit_measurements, fit_bvalues );

   vnl_levenberg_marquardt nonlinear_fit( kurtosis_cost_fn );
   nonlinear_fit.minimize( result );
  }
  else
  {
   itk::kurtosis_fit_lsq_function kurtosis_cost_fn( fit_measurements.size() );
   kurtosis_cost_fn.initialize( fit_measurements, fit_bvalues );
   kurtosis_cost_fn.use_bounds();

   vnl_levenberg_marquardt nonlinear_fit( kurtosis_cost_fn );
   nonlinear_fit.minimize(result);
  }
}



template< class TInputPixelType, class TOutputPixelType>
itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::DiffusionKurtosisReconstructionImageFilter()
  : m_ReferenceBValue(-1),
    m_OmitBZero(false)
{
  this->m_InitialPosition = vnl_vector<double>(3, 0);
  this->m_InitialPosition[2] = 1000.0; // S_0
  this->m_InitialPosition[0] = 0.001; // D
  this->m_InitialPosition[1] = 1; // K

  // single input image
  this->SetNumberOfRequiredInputs(1);

  // two output images (D, K)
  this->SetNumberOfRequiredOutputs(2);
  typename OutputImageType::Pointer outputPtr1 = OutputImageType::New();
  typename OutputImageType::Pointer outputPtr2 = OutputImageType::New();

  this->SetNthOutput(0, outputPtr1.GetPointer() );
  this->SetNthOutput(1, outputPtr2.GetPointer() );

}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::GenerateOutputInformation()
{
  // first call superclass
  Superclass::GenerateOutputInformation();
}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::BeforeThreadedGenerateData()
{

}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::AfterThreadedGenerateData()
{
 /* // initialize buffer to zero overall, but don't forget the requested region pointer
  for( unsigned int i=0; i<this->GetNumberOfOutputs(); ++i)
  {
    typename OutputImageType::Pointer output = this->GetOutput(i);

    // after generating, set the buffered region to max, we have taken care about filling it with valid data in
    // UpdateOutputInformation, if not set, a writer (or any filter using the LargestPossibleRegion() during update may
    // complain about not getting the requested region
    output->SetBufferedRegion( output->GetLargestPossibleRegion() );

    std::cout << "[DiffusionKurtosisReconstructionImageFilter.After]" << output->GetLargestPossibleRegion() << std::endl;
  }*/

}

template< class TInputPixelType, class TOutputPixelType>
typename itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>::KurtosisSnapshot
itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::GetSnapshot(const itk::VariableLengthVector<TInputPixelType> &input, vnl_vector<double> bvalues, bool omit_bzero)
{
  // initialize
  vnl_vector<double> initial_position;
  if( !this->m_OmitBZero )
  {
    initial_position.set_size(2);
    initial_position[0] = this->m_InitialPosition[0];
    initial_position[1] = this->m_InitialPosition[1];

  }
  else
  {
    initial_position.set_size(3);
    initial_position = this->m_InitialPosition;
  }

  // fit
  FitSingleVoxel( input, bvalues, initial_position, omit_bzero );

  // assembly result snapshot
  KurtosisSnapshot result;
  result.m_D = initial_position[0];
  result.m_K = initial_position[1];

  if( omit_bzero )
  {
    result.m_f = 1 - initial_position[2] / std::max(0.01, input.GetElement(0));
  }
  else
    result.m_f = 1;

  return result;
}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::SetGradientDirections(GradientDirectionContainerType::Pointer gradients)
{
  if( this->m_ReferenceBValue < 0)
  {
    itkExceptionMacro( << "Specify reference b-value prior to setting the gradient directions." );
  }

  if( gradients->Size() == 0 )
  {
    itkExceptionMacro( << "Empty gradient directions container retrieved" );
  }

  vnl_vector<double> vnl_bvalues( gradients->Size(), 0 );

  // initialize the b-values
  auto grIter = gradients->Begin();
  unsigned int index = 0;
  while( grIter != gradients->End() )
  {
    const double twonorm = grIter.Value().two_norm();
    vnl_bvalues( index++ ) = this->m_ReferenceBValue * twonorm * twonorm;

    ++grIter;
  }

  this->m_BValues = vnl_bvalues;


}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::SetInitialSolution(const vnl_vector<double>& x0 )
{
  unsigned int param_size = 2 + static_cast<int>( this->m_OmitBZero );
  assert( x0.size() == param_size );

  this->m_InitialPosition = x0;
}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType /*threadId*/)
{
  std::cout << "[ThreadedGenerateData]" << outputRegionForThread;

  typename OutputImageType::Pointer dImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
  itk::ImageRegionIteratorWithIndex< OutputImageType > dImageIt(dImage, outputRegionForThread);
  dImageIt.GoToBegin();

  typename OutputImageType::Pointer kImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(1));
  itk::ImageRegionIteratorWithIndex< OutputImageType > kImageIt(kImage, outputRegionForThread);
  kImageIt.GoToBegin();

  typedef itk::ImageRegionConstIteratorWithIndex< InputImageType > InputIteratorType;
  InputIteratorType inputIter( this->GetInput(), outputRegionForThread );
  inputIter.GoToBegin();

  vnl_vector<double> initial_position;
  if( !this->m_OmitBZero )
  {
    initial_position.set_size(2);
    initial_position[0] = this->m_InitialPosition[0];
    initial_position[1] = this->m_InitialPosition[1];

  }
  else
  {
    initial_position.set_size(3);
    initial_position = this->m_InitialPosition;
  }

  while( !inputIter.IsAtEnd() )
  {
    // set (reset) each iteration
    vnl_vector<double> result = initial_position;

    // fit single voxel

    if( m_MapOutputRegion.IsInside( inputIter.GetIndex() ) )
    {
      FitSingleVoxel( inputIter.Get(), this->m_BValues, result, this->m_OmitBZero );
    }
    else
    {
      result.fill(0);
    }

    // regardless the fit type, the parameters are always in the first two position
    // of the results vector
    dImageIt.Set( result[0] );
    kImageIt.Set( result[1] );

    //std::cout << "[Kurtosis.Fit]" << inputIter.GetIndex() << " --> " << dImageIt.GetIndex() << " result: " << result << std::endl;

    ++inputIter;
    ++dImageIt;
    ++kImageIt;
  }

}

#endif // guards
