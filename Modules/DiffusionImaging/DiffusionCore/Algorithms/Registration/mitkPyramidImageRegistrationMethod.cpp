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

#include "mitkPyramidImageRegistrationMethod.h"

#include "mitkException.h"
#include "mitkImageAccessByItk.h"

mitk::PyramidImageRegistrationMethod::PyramidImageRegistrationMethod()
  : m_FixedImage(NULL),
    m_MovingImage(NULL),
    m_CrossModalityRegistration(true),
    m_UseAffineTransform(true),
    m_UseWindowedSincInterpolator(false),
    m_UseNearestNeighborInterpolator(false),
    m_UseMask(false),
    m_EstimatedParameters(0),
    m_InitialParameters(0),
    m_Verbose(false)
{

}

mitk::PyramidImageRegistrationMethod::~PyramidImageRegistrationMethod()
{
  if( m_EstimatedParameters != NULL)
  {
    delete [] m_EstimatedParameters;
  }

}

void mitk::PyramidImageRegistrationMethod::SetFixedImage(mitk::Image::Pointer fixed)
{
  if( fixed.IsNotNull() )
  {
    m_FixedImage = fixed;
  }
}

void mitk::PyramidImageRegistrationMethod::SetMovingImage(mitk::Image::Pointer moving)
{
  if( moving.IsNotNull() )
  {
    m_MovingImage = moving;
  }
}


void mitk::PyramidImageRegistrationMethod::SetFixedImageMask(mitk::Image::Pointer mask)
{
  m_FixedImageMask = mask;
}

void mitk::PyramidImageRegistrationMethod::Update()
{
  if( m_MovingImage.IsNull() )
  {
    mitkThrow() << " Moving image is null";
  }

  if( m_FixedImage.IsNull() )
  {
    mitkThrow() << " Moving image is null";
  }

  unsigned int allowedDimension = 3;

  if( m_FixedImage->GetDimension() != allowedDimension ||
      m_MovingImage->GetDimension() != allowedDimension )
  {
    mitkThrow() << " Only 3D Images supported.";
  }

  //
  // One possibility: use the FixedDimesnionByItk, but this instantiates ALL possible
  // pixel type combinations!
  // AccessTwoImagesFixedDimensionByItk( m_FixedImage, m_MovingImage, RegisterTwoImages, 3);
  // in helper: TypeSubset : short, float
  AccessTwoImagesFixedDimensionTypeSubsetByItk( m_FixedImage, m_MovingImage, RegisterTwoImagesV4, 3);

}

mitk::PyramidImageRegistrationMethod::TransformMatrixType mitk::PyramidImageRegistrationMethod
::GetLastRotationMatrix()
{
  TransformMatrixType output;
  if( m_EstimatedParameters == NULL )
  {
    output.set_identity();
    return output;
  }

  typedef itk::MatrixOffsetTransformBase< double, 3, 3> BaseTransformType;
  BaseTransformType::Pointer base_transform = BaseTransformType::New();

  if( this->m_UseAffineTransform )
  {
    typedef itk::AffineTransform< double > TransformType;
    TransformType::Pointer transform = TransformType::New();

    TransformType::ParametersType affine_params( TransformType::ParametersDimension );
    this->GetParameters( &affine_params[0] );

    transform->SetParameters( affine_params );
    base_transform = transform;
  }
  else
  {
    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();

    RigidTransformType::ParametersType rigid_params( RigidTransformType::ParametersDimension  );
    this->GetParameters( &rigid_params[0] );

    rtransform->SetParameters( rigid_params );

    base_transform = rtransform;
  }

  return base_transform->GetMatrix().GetVnlMatrix();

}

mitk::Image::Pointer mitk::PyramidImageRegistrationMethod
::GetResampledMovingImage()
{

  mitk::Image::Pointer output = mitk::Image::New();
  //output->Initialize( this->m_FixedImage );

  AccessFixedDimensionByItk_1( this->m_MovingImage, ResampleMitkImage, 3, output );

  return output;

}

mitk::Image::Pointer mitk::PyramidImageRegistrationMethod::GetResampledMovingImage(mitk::Image::Pointer movingImage, double* transform)
{
  mitk::Image::Pointer output = mitk::Image::New();


  unsigned int dim = 12;
  if( !m_UseAffineTransform )
    dim = 6;

  if (m_EstimatedParameters == NULL)
    m_EstimatedParameters = new double[dim];

  double tmpParams[12];
  // save and set temporal transformation values
  for( unsigned int i=0; i<dim; i++)
  {
    tmpParams[i] = m_EstimatedParameters[i];
    m_EstimatedParameters[i] = transform[i];
  }

  AccessFixedDimensionByItk_1( movingImage, ResampleMitkImage, 3, output );

  // Restore old values
  for( unsigned int i=0; i<dim; i++)
  {
    m_EstimatedParameters[i] = tmpParams[i];
  }

  return output;

}

// ITK 4 Includes
#include <itkImageRegistrationMethodv4.h>
#include <itkGradientDescentLineSearchOptimizerv4.h>
#include <itkMattesMutualInformationImageToImageMetricv4.h>
#include <itkJointHistogramMutualInformationImageToImageMetricv4.h>
#include <itkCorrelationImageToImageMetricv4.h>
#include <itkOptimizerParameterScalesEstimator.h>
#include <itkRegistrationParameterScalesFromPhysicalShift.h>

template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::PyramidImageRegistrationMethod::
RegisterTwoImagesV4(itk::Image<TPixel1, VImageDimension1>* itkImage1, itk::Image<TPixel2, VImageDimension2>* itkImage2)
{
  // Basic typedefs needed
  typedef typename itk::Image<TPixel1, VImageDimension1> ItkImageTypeFixed;
  typedef typename itk::Image<TPixel2, VImageDimension1> ItkImageTypeMoving;

  typedef itk::MatrixOffsetTransformBase< double, VImageDimension1, VImageDimension2 > BaseTransformType;

  typedef itk::Image< double, 3> ItkRegistrationImageType;
  typedef itk::CastImageFilter< ItkImageTypeFixed, ItkRegistrationImageType> FixedCastFilterType;
  typedef itk::CastImageFilter< ItkImageTypeMoving, ItkRegistrationImageType> MovingCastFilterType;

  typedef typename itk::ImageRegistrationMethodv4< ItkRegistrationImageType, ItkRegistrationImageType, AffineTransformType> RegistrationType;
  typedef typename itk::MattesMutualInformationImageToImageMetricv4< ItkRegistrationImageType, ItkRegistrationImageType> MIMetricType;
  typedef typename itk::CorrelationImageToImageMetricv4< ItkRegistrationImageType, ItkRegistrationImageType > NCMetricType;
  typedef typename itk::ImageToImageMetricv4< ItkRegistrationImageType, ItkRegistrationImageType > BaseMetricType;

  typedef itk::GradientDescentLineSearchOptimizerv4 OptimizerType;

  typename ItkImageTypeFixed::Pointer referenceImage = itkImage1;
  typename ItkImageTypeMoving::Pointer movingImage = itkImage2;

  // initial parameter dimension ( affine = default )
  unsigned int paramDim = 12;
  typename BaseTransformType::Pointer transform;
  if( m_UseAffineTransform )
  {
    transform = AffineTransformType::New();
  }
  else
  {
    transform = RigidTransformType::New();
    paramDim = 6;
  }

  typename BaseTransformType::ParametersType initialParams(paramDim);

  initialParams.Fill(0);
  if( m_UseAffineTransform )
  {
    initialParams[0] = initialParams[4] = initialParams[8] = 1;
  }


  // [Prepare registration]
  //  The ITKv4 Methods ( the MI Metric ) require double-type images so we need to perform cast first
  typename FixedCastFilterType::Pointer caster_f = FixedCastFilterType::New();
  typename MovingCastFilterType::Pointer caster_m = MovingCastFilterType::New();

  try
  {
    caster_f->SetInput(0, referenceImage );
    caster_m->SetInput(0, movingImage );

    caster_f->Update();
    caster_m->Update();

  }
  catch( const itk::ExceptionObject &/*e*/ )
  {
    return ;
  }

  // [Prepare Registration]
  //  Estimate the size of the pyramid based on image dimension
  //  here the image size on the topmost level must remain higher than the threshold
  unsigned int max_pyramid_lvl = 4;
  unsigned int max_schedule_val = 8;
  const unsigned int min_required_image_size = 12;
  typename ItkImageTypeFixed::RegionType::SizeType image_size = referenceImage->GetLargestPossibleRegion().GetSize();
  unsigned int min_value = std::min( image_size[0], std::min( image_size[1], image_size[2]));

  // Adapt also the optimizer settings to the number of levels
  float optmaxstep = 10;
  float optminstep = 0.1f;
  unsigned int iterations = 40;
  unsigned int convergence_win_size = 8;
  double convergence_tolerance = 1e-5;
  while( min_value / max_schedule_val < min_required_image_size )
  {
    max_schedule_val /= 2;
    max_pyramid_lvl--;
    optmaxstep -= 2;
    optminstep *= 0.5f;
    convergence_win_size += 4;
    convergence_tolerance *= 0.5;
  }

  typename RegistrationType::ShrinkFactorsArrayType shrink_factors(max_pyramid_lvl);
  shrink_factors.Fill(1);

  shrink_factors[0] = max_schedule_val;
  for( unsigned int i=1; i<max_pyramid_lvl; i++)
  {
    shrink_factors[i] = std::max( shrink_factors[i-1]/2, 1ul);
  }

  if(m_Verbose)
  {
    MITK_INFO("dw.pyramid.reg") << "  : Pyramid size set to : " << max_pyramid_lvl;
  }

  // [Prepare Registration]
  //  Initialize the optimizer
  //  (i) Use the scales estimator ( depends on metric )
  typename OptimizerType::Pointer optimizer = OptimizerType::New();
  typename BaseMetricType::Pointer base_metric;
  if( m_CrossModalityRegistration )
  {
    typename MIMetricType::Pointer metric = MIMetricType::New();

    typename itk::RegistrationParameterScalesFromPhysicalShift< MIMetricType >::Pointer mi_estimator =
             itk::RegistrationParameterScalesFromPhysicalShift< MIMetricType >::New();
    mi_estimator->SetMetric( metric );

    optimizer->SetScalesEstimator( mi_estimator );

    base_metric = metric;
  }
  else
  {
    typename NCMetricType::Pointer metric = NCMetricType::New();

    typename itk::RegistrationParameterScalesFromPhysicalShift< NCMetricType >::Pointer nc_estimator =
             itk::RegistrationParameterScalesFromPhysicalShift< NCMetricType >::New();

    nc_estimator->SetMetric( metric );

    optimizer->SetScalesEstimator( nc_estimator );

    base_metric = metric;
  }

  optimizer->SetDoEstimateLearningRateOnce( false );
  optimizer->SetDoEstimateLearningRateAtEachIteration( true );
  optimizer->SetNumberOfIterations( iterations );
  optimizer->SetConvergenceWindowSize( convergence_win_size );
  optimizer->SetMaximumStepSizeInPhysicalUnits( optmaxstep );
  optimizer->SetMinimumConvergenceValue( convergence_tolerance );

  // line search options
  optimizer->SetEpsilon( 1e-3 );
  optimizer->SetLowerLimit( 0.3 );
  optimizer->SetUpperLimit( 1.7 );
  optimizer->SetMaximumLineSearchIterations( 20 );

  // add observer tag if verbose
  unsigned long vopt_tag = 0;
  if(m_Verbose)
  {
    MITK_INFO << "  :: Starting at " << initialParams;

    OptimizerIterationCommandv4< OptimizerType >::Pointer iterationObserver =
        OptimizerIterationCommandv4<OptimizerType>::New();

    vopt_tag = optimizer->AddObserver( itk::IterationEvent(), iterationObserver );
  }


  // Initializing by Geometry
  if( !m_UseAffineTransform && m_InitializeByGeometry )
  {
    typedef itk::CenteredVersorTransformInitializer< ItkImageTypeFixed, ItkImageTypeMoving> TransformInitializerType;
    typename TransformInitializerType::TransformType::Pointer rigidTransform = TransformInitializerType::TransformType::New() ;
    MITK_INFO << "Initializer starting at : " << rigidTransform->GetParameters();

    typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    initializer->SetTransform( rigidTransform);
    initializer->SetFixedImage( referenceImage.GetPointer() );
    initializer->SetMovingImage( movingImage.GetPointer() );
    initializer->MomentsOn();
    initializer->InitializeTransform();

    MITK_INFO << "Initialized Rigid position :  " << rigidTransform->GetParameters();
    initialParams[3] = rigidTransform->GetParameters()[3];
    initialParams[4] = rigidTransform->GetParameters()[4];
    initialParams[5] = rigidTransform->GetParameters()[5];
  }

  // [Prepare Registration]
  //  Masking (Optional)
  if( m_UseMask )
  {
    typedef itk::Image<unsigned char, 3> BinaryImageType;
    BinaryImageType::Pointer itkFixedImageMask = BinaryImageType::New();
    itk::ImageMaskSpatialObject< 3 >::Pointer fixedMaskSpatialObject = itk::ImageMaskSpatialObject< 3 >::New();

    CastToItkImage( m_FixedImageMask, itkFixedImageMask);
    itk::NotImageFilter<BinaryImageType, BinaryImageType>::Pointer notFilter = itk::NotImageFilter<BinaryImageType, BinaryImageType>::New();
    notFilter->SetInput(itkFixedImageMask);
    notFilter->Update();
    fixedMaskSpatialObject->SetImage( notFilter->GetOutput() );
    base_metric->SetFixedImageMask( fixedMaskSpatialObject );
  }

  // [Prepare Registration]
  //  combine all components to set-up registration
  typename RegistrationType::Pointer registration = RegistrationType::New();

  registration->SetFixedImage( 0, caster_f->GetOutput() );
  registration->SetMovingImage( 0, caster_m->GetOutput() );
  registration->SetMetric( base_metric );
  registration->SetOptimizer( optimizer );
  registration->SetMovingInitialTransform( transform.GetPointer() );
  registration->SetNumberOfLevels(max_pyramid_lvl);
  registration->SetShrinkFactorsPerLevel( shrink_factors );

  // observe the pyramid level change in order to adapt parameters
  typename PyramidOptControlCommandv4<RegistrationType>::Pointer pyramid_observer =
      PyramidOptControlCommandv4<RegistrationType>::New();
  unsigned int pyramid_tag =  registration->AddObserver( itk::InitializeEvent(), pyramid_observer );


  try
  {
    registration->Update();
  }
  catch( const itk::ExceptionObject &e)
  {
    registration->Print( std::cout );

    MITK_ERROR << "[Registration Update] Caught ITK exception: ";
    MITK_ERROR("itk.exception") << e.what();

    mitkThrow() << "Registration failed with exception: " << e.what();
  }

  // [Post Registration]
  //   Retrieve the last transformation parameters from the performed registration task
  if( m_EstimatedParameters != NULL)
  {
    delete [] m_EstimatedParameters;
  }

  m_EstimatedParameters = new double[paramDim];

  typename BaseTransformType::ParametersType finalParameters = registration->GetOptimizer()->GetCurrentPosition();
  for( unsigned int i=0; i<paramDim; i++)
  {
    m_EstimatedParameters[i] = finalParameters[i];
  }

  MITK_INFO("Params") << optimizer->GetValue() << " :: " << finalParameters;

  if( m_Verbose )
  {
    MITK_INFO("Termination") << optimizer->GetStopConditionDescription();
  }

  // remove optimizer tag if used
  if( vopt_tag )
  {
    optimizer->RemoveObserver( vopt_tag );
  }

  if( pyramid_tag )
  {
    registration->RemoveObserver( pyramid_tag );
  }


}

template< typename TPixel, unsigned int VDimension>
void mitk::PyramidImageRegistrationMethod::
ResampleMitkImage( itk::Image<TPixel, VDimension>* itkImage,
                        mitk::Image::Pointer& outputImage )
{
  typedef typename itk::Image< TPixel, VDimension> ImageType;
  typedef typename itk::ResampleImageFilter<  ImageType, ImageType, double> ResampleImageFilterType;

  typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
  typename InterpolatorType::Pointer linear_interpolator = InterpolatorType::New();


  typedef itk::NearestNeighborInterpolateImageFunction< ImageType, double > NearestNeighborInterpolatorType;
  typename NearestNeighborInterpolatorType::Pointer nn_interpolator = NearestNeighborInterpolatorType::New();


  typedef itk::WindowedSincInterpolateImageFunction< ImageType, 7> WindowedSincInterpolatorType;
  typename WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  typename ImageType::Pointer reference_image = ImageType::New();
  CastToItkImage(m_FixedImage,reference_image);

  typedef itk::MatrixOffsetTransformBase< double, 3, 3> BaseTransformType;
  BaseTransformType::Pointer base_transform = BaseTransformType::New();

  if( this->m_UseAffineTransform )
  {
    typedef itk::AffineTransform< double > TransformType;
    TransformType::Pointer transform = TransformType::New();

    TransformType::ParametersType affine_params( TransformType::ParametersDimension );
    this->GetParameters( &affine_params[0] );

    transform->SetParameters( affine_params );

    base_transform = transform;
  }
  // Rigid
  else
  {
    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();

    RigidTransformType::ParametersType rigid_params( RigidTransformType::ParametersDimension  );
    this->GetParameters( &rigid_params[0] );

    rtransform->SetParameters( rigid_params );

    base_transform = rtransform;
  }

  typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();

  resampler->SetInterpolator( linear_interpolator );
  if( m_UseWindowedSincInterpolator )
    resampler->SetInterpolator( sinc_interpolator );
  if ( m_UseNearestNeighborInterpolator)
    resampler->SetInterpolator ( nn_interpolator );

  //resampler->SetNumberOfThreads(1);

  resampler->SetInput( itkImage );
  resampler->SetTransform( base_transform );
  resampler->SetReferenceImage( reference_image );
  resampler->UseReferenceImageOn();

  resampler->Update();

  mitk::GrabItkImageMemory( resampler->GetOutput(), outputImage);
}
