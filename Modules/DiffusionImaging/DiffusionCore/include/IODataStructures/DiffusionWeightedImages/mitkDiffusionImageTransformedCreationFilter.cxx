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

#ifndef DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_CXX
#define DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_CXX

#include "mitkDiffusionImageTransformedCreationFilter.h"
#include "mitkDiffusionImageCorrectionFilter.h"

#include "mitkImageCast.h"
#include "mitkImageWriteAccessor.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkIOUtil.h"


#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>


template <typename TTransformType, typename ItkImageType>
static void ResampleImage( typename ItkImageType::Pointer itk_reference,
                           mitk::Image::Pointer mitk_input,
                           typename TTransformType::Pointer transform,
                           unsigned int interpolator,
                           mitk::Image::Pointer output_target, unsigned int position )
{


  typedef itk::LinearInterpolateImageFunction< ItkImageType, double > InterpolatorType;
  typename InterpolatorType::Pointer linear_interpolator = InterpolatorType::New();

  typedef itk::NearestNeighborInterpolateImageFunction< ItkImageType, double > NearestNeighborInterpolatorType;
  typename NearestNeighborInterpolatorType::Pointer nn_interpolator = NearestNeighborInterpolatorType::New();

  typedef itk::WindowedSincInterpolateImageFunction< ItkImageType, 7> WindowedSincInterpolatorType;
  typename WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  typedef itk::BSplineInterpolateImageFunction< ItkImageType, double, double> BSplineInterpolatorType;
  typename BSplineInterpolatorType::Pointer bicubic_interpolator = BSplineInterpolatorType::New();
  bicubic_interpolator->SetSplineOrder(3);

  typedef typename itk::ResampleImageFilter<  ItkImageType, ItkImageType, double> ResampleImageFilterType;
  typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();

  // Create interpolator pool
  typedef itk::InterpolateImageFunction< ItkImageType, double> InterpolateFunctionBaseType;
  std::vector< InterpolateFunctionBaseType* > InterpolateFunctionsPool;

  InterpolateFunctionsPool.push_back( nn_interpolator.GetPointer() );
  InterpolateFunctionsPool.push_back( linear_interpolator.GetPointer() );
  InterpolateFunctionsPool.push_back( bicubic_interpolator.GetPointer() );
  InterpolateFunctionsPool.push_back( sinc_interpolator.GetPointer() );

  // select interpolator by the selection flag
  resampler->SetInterpolator( InterpolateFunctionsPool.at( interpolator ) );

  typename ItkImageType::Pointer itk_input;
  mitk::CastToItkImage< ItkImageType >( mitk_input, itk_input );

  resampler->SetInput( itk_input );
  resampler->SetTransform( transform );
  resampler->SetReferenceImage( itk_reference );
  resampler->UseReferenceImageOn();
  resampler->Update();

  mitk::Image::Pointer current_resampled = mitk::Image::New();
  current_resampled->InitializeByItk( resampler->GetOutput() );
  current_resampled->SetImportChannel( resampler->GetOutput()->GetBufferPointer(),
                                       mitk::Image::CopyMemory );

  mitk::ImageWriteAccessor imac( current_resampled );
  output_target->SetImportVolume( imac.GetData(),
                                  position, 0, mitk::Image::CopyMemory );

}
/*
template< typename TTransformType >
static mitk::DiffusionPropertyHelper::GradientDirectionType
TransformGradientDirection( mitk::DiffusionPropertyHelper::GradientDirectionType vec_in,
                            typename TTransformType::Pointer transform )
{
  mitk::DiffusionPropertyHelper::GradientDirectionType vec_out;
  vec_out.fill(1.0);

  typedef typename TTransformType::MatrixType TMatrixType;

  return vec_out;
}
*/
template< typename TTransformType>
mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::DiffusionImageTransformedCreationFilter()
{
  this->m_OutputPrefix = "/tmp/";
}

template< typename TTransformType>
mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::~DiffusionImageTransformedCreationFilter()
{

}

template< typename TTransformType>
mitk::DiffusionImageHeaderDescriptor
mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::GetTransformedHeaderInformation()
{
  typedef mitk::DiffusionPropertyHelper DPH;

  DiffusionImageHeaderDescriptor dwhdesc;
  dwhdesc.m_BValue = DPH::GetReferenceBValue( this->m_DiffusionReferenceImage );
  // TODO : here comes transformation of the gradients
  dwhdesc.m_GradientDirections = DPH::GetGradientContainer( this->m_DiffusionReferenceImage );
  dwhdesc.m_MeasurementFrame = DPH::GetMeasurementFrame( this->m_DiffusionReferenceImage );
  dwhdesc.m_BValueMapType = DPH::GetBValueMap( this->m_DiffusionReferenceImage );

  return dwhdesc;
}

template< typename TTransformType>
void mitk::DiffusionImageTransformedCreationFilter< TTransformType >
::GenerateOutputInformation()
{
  MITK_INFO << "Debug info";
}


template< typename TTransformType>
void mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::GenerateData()
{

  mitk::Image::Pointer input = this->GetInput();

  // validity checks
  if( m_InternalTransforms.size() != input->GetTimeSteps() )
  {
    mitkThrow() << "Number of transforms" << m_InternalTransforms.size()  << "differ from number of timesteps" << input->GetTimeSteps();
  }

  typedef itk::Image< DiffusionPropertyHelper::DiffusionPixelType, 3> ImageType;

  // create intermediate output
  mitk::Image::Pointer resampled_output = mitk::Image::New();
  resampled_output->Initialize( input );

  ImageType::Pointer current_itk_reference = ImageType::New();
  CastToItkImage( this->m_ResamplingReferenceImage, current_itk_reference );

  unsigned int time_dim = input->GetDimension(3);
  for( unsigned int time_idx = 0; time_idx < time_dim; time_idx++ )
  {

    mitk::ImageTimeSelector::Pointer t_sel = mitk::ImageTimeSelector::New();
    t_sel->SetInput( input );
    t_sel->SetTimeNr( time_idx );
    t_sel->Update();
    ResampleImage< TTransformType, ImageType>( current_itk_reference,
                                               t_sel->GetOutput(),
                                               this->m_InternalTransforms.at(time_idx),
                                               this->m_InterpolationLevel,
                                               resampled_output, time_idx);
  }

  // call here creation filter
  mitk::DiffusionImageCreationFilter::Pointer creator =
      mitk::DiffusionImageCreationFilter::New();

  creator->SetInput( resampled_output );
  creator->SetHeaderDescriptor( this->GetTransformedHeaderInformation() );
  creator->Update();

  mitk::Image::Pointer output = creator->GetOutput();
  mitk::DiffusionPropertyHelper::CopyProperties(resampled_output, output, true);

  typedef mitk::DiffusionPropertyHelper DPH;
  float BValue = mitk::DiffusionPropertyHelper::GetReferenceBValue( creator->GetOutput() );

  mitk::BValueMapProperty::BValueMap BValueMap = mitk::BValueMapProperty::CreateBValueMap( mitk::DiffusionPropertyHelper::GetGradientContainer( creator->GetOutput() ), BValue );
  output->GetPropertyList()->ReplaceProperty( DPH::GRADIENTCONTAINERPROPERTYNAME.c_str(),  mitk::GradientDirectionsProperty::New( mitk::DiffusionPropertyHelper::GetGradientContainer( creator->GetOutput() ))  );
  output->GetPropertyList()->ReplaceProperty( DPH::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( mitk::DiffusionPropertyHelper::GetMeasurementFrame( creator->GetOutput() ) ) );
  output->GetPropertyList()->ReplaceProperty( DPH::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New( BValueMap ) );
  output->GetPropertyList()->ReplaceProperty( DPH::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( BValue ) );

  // correct gradients
  mitk::DiffusionImageCorrectionFilter::Pointer corrector = mitk::DiffusionImageCorrectionFilter::New();
  corrector->SetImage( output) ;
  corrector->CorrectDirections( this->m_RotationMatrices );

  DPH pHelper( output );
  pHelper.InitializeImage();

  m_OutputCache = output;
  this->SetPrimaryOutput( m_OutputCache );
  m_OutputCache->Modified();
}

template< typename TTransformType>
void mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::SetTransformParameters(const TransformParametersContainerType &params)
{
  if( params.empty() )
  {
    MITK_ERROR << "Empty parameter list given.";
    return;
  }

  TransformContainerType transforms;

  auto iter = std::begin( params );
  while( iter != std::end( params ) )
  {
    typename TTransformType::Pointer transform = TTransformType::New();
    transform->SetParameters( (*iter) );

    transforms.push_back( transform );

    ++iter;
  }

  this->SetTransforms( transforms );
}

template< typename TTransformType>
void mitk::DiffusionImageTransformedCreationFilter<TTransformType>
::SetTransforms(const TransformContainerType &transforms)
{
  if( transforms.empty() )
  {
    MITK_ERROR << "Empty transform list given.";
    return;
  }

  this->m_InternalTransforms.reserve( transforms.size() );
  std::copy( transforms.begin(), transforms.end(),
             std::back_inserter(this->m_InternalTransforms ) );

  MatrixType E;
  E.set_identity();

  for( auto iter = std::begin(this->m_InternalTransforms); iter != std::end( this->m_InternalTransforms); ++iter)
  {
    MatrixType A = E * (*iter)->GetMatrix().GetVnlMatrix();
    this->m_RotationMatrices.push_back( A );
  }
}



#endif // DIFFUSIONIMAGETRANSFORMEDCREATIONFILTER_CXX
