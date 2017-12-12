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

#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkComposeImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>

template< class TInputPixelType>
static void FitSingleVoxel( const itk::VariableLengthVector< TInputPixelType > &input,
                            const vnl_vector<double>& bvalues,
                            vnl_vector<double>& result,
                            itk::KurtosisFitConfiguration kf_config)
{
  // check for length
  assert( input.Size() == bvalues.size() );

  // assembly data vectors for fitting
  auto bvalueIter = bvalues.begin();
  unsigned int unused_values = 0;
  while( bvalueIter != bvalues.end() )
  {
    if( *bvalueIter < vnl_math::eps && kf_config.omit_bzero )
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
        // skip b=0 if the corresponding flag is set
    if( ( kf_config.omit_bzero && *bvalueIter < vnl_math::eps )
        // skip bvalues higher than the limit provided, if the flag is activated
        || ( kf_config.exclude_high_b && *bvalueIter > kf_config.b_upper_threshold ) )
    {
      ++skip_count;
    }
    else
    {
      fit_measurements[ running_index - skip_count ]  = input.GetElement(running_index);
      fit_bvalues[ running_index - skip_count] = *bvalueIter;
    }

    ++running_index;
    ++bvalueIter;
  }


  MITK_DEBUG("KurtosisFilter.FitSingleVoxel.Meas") << fit_measurements;
  MITK_DEBUG("KurtosisFilter.FitSingleVoxel.Bval") << fit_bvalues;

  // perform fit on data vectors
  if( kf_config.omit_bzero )
  {
   itk::kurtosis_fit_omit_unweighted kurtosis_cost_fn( fit_measurements.size() );
   // configuration
   kurtosis_cost_fn.set_fit_logscale( static_cast<bool>(kf_config.fit_scale) );
   kurtosis_cost_fn.initialize( fit_measurements, fit_bvalues );

   if( kf_config.use_K_limits)
   {
    kurtosis_cost_fn.set_K_bounds( kf_config.K_limits );
   }

   vnl_levenberg_marquardt nonlinear_fit( kurtosis_cost_fn );
   nonlinear_fit.minimize( result );
  }
  else
  {
   itk::kurtosis_fit_lsq_function kurtosis_cost_fn( fit_measurements.size() );
   // configuration
   kurtosis_cost_fn.set_fit_logscale( static_cast<bool>(kf_config.fit_scale) );
   kurtosis_cost_fn.initialize( fit_measurements, fit_bvalues );

   if( kf_config.use_K_limits)
   {
    kurtosis_cost_fn.set_K_bounds( kf_config.K_limits );
   }


   vnl_levenberg_marquardt nonlinear_fit( kurtosis_cost_fn );
   nonlinear_fit.minimize(result);
  }

  MITK_DEBUG("KurtosisFilter.FitSingleVoxel.Rslt") << result;
}



template< class TInputPixelType, class TOutputPixelType>
itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::DiffusionKurtosisReconstructionImageFilter()
  : m_ReferenceBValue(-1),
    m_OmitBZero(false),
    m_ApplyPriorSmoothing(false),
    m_SmoothingSigma(1.5),
    m_UseKBounds( false ),
    m_MaxFitBValue( 3000 ),
    m_ScaleForFitting( STRAIGHT )
{
  this->m_InitialPosition = vnl_vector<double>(3, 0);
  this->m_InitialPosition[2] = 1000.0; // S_0
  this->m_InitialPosition[0] = 0.001; // D
  this->m_InitialPosition[1] = 1; // K

  this->m_KurtosisBounds.fill(0);

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
::SetImageMask(MaskImageType::Pointer mask)
{
  this->m_MaskImage = mask;
}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::BeforeThreadedGenerateData()
{
  // if we have a region set, convert it to a mask image, which is to be used as default for telling
  // we need to set the image anyway, so by default the mask is overall 1
  if( m_MaskImage.IsNull() )
  {
    m_MaskImage = MaskImageType::New();
    m_MaskImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
    m_MaskImage->CopyInformation( this->GetInput() );
    m_MaskImage->Allocate();

    if( this->m_MapOutputRegion.GetNumberOfPixels() > 0 )
    {

      m_MaskImage->FillBuffer(0);

      typedef itk::ImageRegionIteratorWithIndex< MaskImageType > MaskIteratorType;
      MaskIteratorType maskIter( this->m_MaskImage, this->m_MapOutputRegion );
      maskIter.GoToBegin();

      while( !maskIter.IsAtEnd() )
      {
        maskIter.Set( 1 );
        ++maskIter;
      }

    }
    else
    {
      m_MaskImage->FillBuffer(1);
    }


  }


  // apply smoothing to the input image
  if( this->m_ApplyPriorSmoothing )
  {
    // filter typedefs
    typedef itk::DiscreteGaussianImageFilter< itk::Image<TInputPixelType, 3 >, itk::Image<TInputPixelType, 3 > > GaussianFilterType;
    typedef itk::VectorIndexSelectionCastImageFilter< InputImageType, itk::Image<TInputPixelType, 3 > > IndexSelectionType;
    typedef itk::ComposeImageFilter< itk::Image<TInputPixelType, 3>, InputImageType > ComposeFilterType;


    auto vectorImage = this->GetInput();
    typename IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetInput( vectorImage );

    typename ComposeFilterType::Pointer vec_composer = ComposeFilterType::New();

    for( unsigned int i=0; i<vectorImage->GetVectorLength(); ++i)
    {
      typename GaussianFilterType::Pointer gaussian_filter = GaussianFilterType::New();

      indexSelectionFilter->SetIndex( i );

      gaussian_filter->SetInput( indexSelectionFilter->GetOutput() );
      gaussian_filter->SetVariance( m_SmoothingSigma );

      vec_composer->SetInput(i, gaussian_filter->GetOutput() );

      gaussian_filter->Update();
    }

    try
    {
      vec_composer->Update();
    }
    catch(const itk::ExceptionObject &e)
    {
      mitkThrow() << "[VectorImage.GaussianSmoothing] !! Failed with ITK Exception: " << e.what();
    }

    this->m_ProcessedInputImage = vec_composer->GetOutput();
  }
  else
  {
    this->m_ProcessedInputImage = const_cast<InputImageType*>( this->GetInput() );
  }
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
::GetSnapshot(const itk::VariableLengthVector<TInputPixelType> &input, GradientDirectionContainerType::Pointer gradients, float bvalue, KurtosisFitConfiguration kf_conf)
{
  // initialize bvalues from reference value and the gradients provided on input
  this->SetReferenceBValue(bvalue);
  this->SetGradientDirections( gradients );

  // call the other method
  return this->GetSnapshot( input, this->m_BValues, kf_conf );
}


template< class TInputPixelType, class TOutputPixelType>
typename itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>::KurtosisSnapshot
itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::GetSnapshot(const itk::VariableLengthVector<TInputPixelType> &input, vnl_vector<double> bvalues, KurtosisFitConfiguration kf_conf)
{
  // initialize
  vnl_vector<double> initial_position;
  bool omit_bzero = kf_conf.omit_bzero;

  if( !omit_bzero )
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
  FitSingleVoxel( input, bvalues, initial_position, kf_conf );

  // assembly result snapshot
  KurtosisSnapshot result;
  result.m_D = initial_position[0];
  result.m_K = initial_position[1];

  if( omit_bzero )
  {
    result.m_f = 1 - initial_position[2] / std::fmax(0.01, input.GetElement(0));
    result.m_BzeroFit = initial_position[2];
  }
  else
    result.m_f = 1;

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

  // original data
  vnl_vector<double> orig_measurements( input.Size(), 0 );

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
      fit_measurements[ running_index - skip_count ]  = input.GetElement(running_index);
      fit_bvalues[ running_index - skip_count ] = *bvalueIter;
    }

    orig_measurements[ running_index ] = input.GetElement(running_index);

    ++running_index;
    ++bvalueIter;
  }

  result.fit_bvalues = fit_bvalues;
  result.fit_measurements = fit_measurements;

  result.bvalues = bvalues;
  result.measurements = orig_measurements;

  result.m_fittedBZero = omit_bzero;

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
  assert( x0.size() == (2 + static_cast<int>( this->m_OmitBZero )) );

  this->m_InitialPosition = x0;
}

template< class TInputPixelType, class TOutputPixelType>
void itk::DiffusionKurtosisReconstructionImageFilter<TInputPixelType, TOutputPixelType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType /*threadId*/)
{
  typename OutputImageType::Pointer dImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
  itk::ImageRegionIteratorWithIndex< OutputImageType > dImageIt(dImage, outputRegionForThread);
  dImageIt.GoToBegin();

  typename OutputImageType::Pointer kImage = static_cast< OutputImageType * >(this->ProcessObject::GetOutput(1));
  itk::ImageRegionIteratorWithIndex< OutputImageType > kImageIt(kImage, outputRegionForThread);
  kImageIt.GoToBegin();

  typedef itk::ImageRegionConstIteratorWithIndex< InputImageType > InputIteratorType;
  InputIteratorType inputIter( m_ProcessedInputImage, outputRegionForThread );
  inputIter.GoToBegin();

  typedef itk::ImageRegionConstIteratorWithIndex< MaskImageType > MaskIteratorType;
  MaskIteratorType maskIter( this->m_MaskImage, outputRegionForThread );
  maskIter.GoToBegin();

  KurtosisFitConfiguration fit_config;
  fit_config.omit_bzero = this->m_OmitBZero;
  fit_config.fit_scale = this->m_ScaleForFitting;

  fit_config.use_K_limits = this->m_UseKBounds;
  fit_config.K_limits = this->m_KurtosisBounds;

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

    // fit single voxel (if inside mask )
    if( maskIter.Get() > 0 )
    {
      FitSingleVoxel( inputIter.Get(), this->m_BValues, result, fit_config );
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

    ++maskIter;
    ++inputIter;
    ++dImageIt;
    ++kImageIt;
  }

}

#endif // guards
