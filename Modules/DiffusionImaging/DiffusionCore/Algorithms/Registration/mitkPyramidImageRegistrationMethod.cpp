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
  AccessTwoImagesFixedDimensionTypeSubsetByItk( m_FixedImage, m_MovingImage, RegisterTwoImages, 3);

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

template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::PyramidImageRegistrationMethod::
RegisterTwoImages(itk::Image<TPixel1, VImageDimension1>* itkImage1, itk::Image<TPixel2, VImageDimension2>* itkImage2)
{
  typedef typename itk::Image< TPixel1, VImageDimension1> FixedImageType;
  typedef typename itk::Image< TPixel2, VImageDimension2> MovingImageType;
  typedef typename itk::MultiResolutionImageRegistrationMethod< FixedImageType, MovingImageType > RegistrationType;
  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;

  typedef itk::MatrixOffsetTransformBase< double, VImageDimension1, VImageDimension2 > BaseTransformType;

  typedef typename itk::MattesMutualInformationImageToImageMetric< FixedImageType, MovingImageType > MMIMetricType;
  typedef typename itk::NormalizedCorrelationImageToImageMetric< FixedImageType, MovingImageType > NCMetricType;
  typedef typename itk::ImageToImageMetric< FixedImageType, MovingImageType> BaseMetricType;

  typename itk::LinearInterpolateImageFunction<MovingImageType, double>::Pointer interpolator =
      itk::LinearInterpolateImageFunction<MovingImageType, double>::New();


  typename BaseMetricType::Pointer metric;

  unsigned int glob_max_threads = itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);

  if( m_CrossModalityRegistration )
  {
    metric = MMIMetricType::New();
    //metric->SetNumberOfThreads( 6 );
  }
  else
  {
    metric = NCMetricType::New();
  }



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

/* FIXME : Review removal, occured during rebasing on master <30 Jan>
  if( m_InitialParameters.Size() == paramDim )
  {
    initialParams = m_InitialParameters;
  }
*/
  typename FixedImageType::Pointer referenceImage = itkImage1;
  typename MovingImageType::Pointer movingImage = itkImage2;
  typename FixedImageType::RegionType maskedRegion = referenceImage->GetLargestPossibleRegion();

  typename RegistrationType::Pointer registration = RegistrationType::New();
  unsigned int max_pyramid_lvl = 3;
  unsigned int max_schedule_val = 4;
  typename FixedImageType::RegionType::SizeType image_size = referenceImage->GetLargestPossibleRegion().GetSize();
  unsigned int min_value = std::min( image_size[0], std::min( image_size[1], image_size[2]));

  // condition for the top level pyramid image
  float optmaxstep = 6;
  float optminstep = 0.05f;
  unsigned int iterations = 100;
  if( min_value / max_schedule_val < 12 )
  {
    max_schedule_val /= 2;
    max_pyramid_lvl--;
    optmaxstep *= 0.25f;
    optminstep *= 0.1f;
  }

  typename RegistrationType::ScheduleType fixedSchedule(max_pyramid_lvl,3);
  fixedSchedule.Fill(1);

  fixedSchedule[0][0] = max_schedule_val;
  fixedSchedule[0][1] = max_schedule_val;
  fixedSchedule[0][2] = max_schedule_val;

  for( unsigned int i=1; i<max_pyramid_lvl; i++)
  {
    fixedSchedule[i][0] = std::max( fixedSchedule[i-1][0]/2, 1u);
    fixedSchedule[i][1] = std::max( fixedSchedule[i-1][1]/2, 1u);
    fixedSchedule[i][2] = std::max( fixedSchedule[i-1][2]/2, 1u);
  }

  typename OptimizerType::Pointer optimizer = OptimizerType::New();
  typename OptimizerType::ScalesType optScales( paramDim );
  optScales.Fill(10.0);

  optScales[paramDim-3] = 1.0/1000;
  optScales[paramDim-2] = 1.0/1000;
  optScales[paramDim-1] = 1.0/1000;

  optimizer->SetScales( optScales );
  optimizer->SetInitialPosition( initialParams );
  optimizer->SetNumberOfIterations( iterations );
  optimizer->SetGradientMagnitudeTolerance( 1e-4 );
  optimizer->SetRelaxationFactor( 0.7 );
  optimizer->SetMaximumStepLength( optmaxstep );
  optimizer->SetMinimumStepLength( optminstep );

  // Initializing by Geometry
  if (!m_UseAffineTransform && m_InitializeByGeometry)
  {
    typedef itk::CenteredVersorTransformInitializer< FixedImageType, MovingImageType> TransformInitializerType;
    typename TransformInitializerType::TransformType::Pointer rigidTransform = TransformInitializerType::TransformType::New() ;
    MITK_INFO << "Initializer starting at : " << rigidTransform->GetParameters();

    typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    initializer->SetTransform( rigidTransform);
    initializer->SetFixedImage( referenceImage.GetPointer() );
    initializer->SetMovingImage(  movingImage.GetPointer() );
    initializer->MomentsOn();
    initializer->InitializeTransform();
    MITK_INFO << "Initialized Rigid position :  " << rigidTransform->GetParameters();
    initialParams[3]=rigidTransform->GetParameters()[3];
    initialParams[4]=rigidTransform->GetParameters()[4];
    initialParams[5]=rigidTransform->GetParameters()[5];
  }

  // add observer tag if verbose */
  unsigned long vopt_tag = 0;
  if(m_Verbose)
  {
    MITK_INFO << "  :: Starting at " << initialParams;
    MITK_INFO << "  :: Scales  =  " << optScales;

    OptimizerIterationCommand< OptimizerType >::Pointer iterationObserver =
        OptimizerIterationCommand<OptimizerType>::New();

    vopt_tag = optimizer->AddObserver( itk::IterationEvent(), iterationObserver );
  }


  // INPUT IMAGES
  registration->SetFixedImage( referenceImage );
  registration->SetFixedImageRegion( maskedRegion );
  registration->SetMovingImage( movingImage );
  registration->SetSchedules( fixedSchedule, fixedSchedule);
  // SET MASKED AREA
  typedef itk::Image<unsigned char, 3> BinaryImageType;
  BinaryImageType::Pointer itkFixedImageMask = BinaryImageType::New();
  itk::ImageMaskSpatialObject< 3 >::Pointer fixedMaskSpatialObject = itk::ImageMaskSpatialObject< 3 >::New();
  if (m_UseMask)
  {
    CastToItkImage(m_FixedImageMask,itkFixedImageMask);
    itk::NotImageFilter<BinaryImageType, BinaryImageType>::Pointer notFilter = itk::NotImageFilter<BinaryImageType, BinaryImageType>::New();
    notFilter->SetInput(itkFixedImageMask);
    notFilter->Update();
    fixedMaskSpatialObject->SetImage( notFilter->GetOutput() );
    metric->SetFixedImageMask( fixedMaskSpatialObject );
  }
  // OTHER INPUTS
  registration->SetMetric( metric );
  registration->SetOptimizer( optimizer );
  registration->SetTransform( transform.GetPointer() );
  registration->SetInterpolator( interpolator );
  registration->SetInitialTransformParameters( initialParams );

  typename PyramidOptControlCommand<RegistrationType>::Pointer pyramid_observer =
      PyramidOptControlCommand<RegistrationType>::New();


  unsigned int pyramid_tag = 0;
  if(m_Verbose)
  {
      pyramid_tag = registration->AddObserver( itk::IterationEvent(), pyramid_observer );
  }

  try
  {
    registration->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    registration->Print( std::cout );
    /*registration->Print( std::cout );
    MITK_INFO << "========== reference ";
    itkImage1->Print( std::cout );

    MITK_INFO << "========== moving ";
    itkImage2->Print( std::cout );*/

    MITK_ERROR << "[Registration Update] Caught ITK exception: ";
    mitkThrow() << "Registration failed with exception: " << e.what();
  }

  if( m_EstimatedParameters != NULL)
  {
    delete [] m_EstimatedParameters;
  }

  m_EstimatedParameters = new double[paramDim];

  typename BaseTransformType::ParametersType finalParameters = registration->GetLastTransformParameters();
  for( unsigned int i=0; i<paramDim; i++)
  {
    m_EstimatedParameters[i] = finalParameters[i];
  }

  MITK_INFO("Params") << optimizer->GetValue() << " :: " << finalParameters;

  // remove optimizer tag if used
  if( vopt_tag )
  {
    optimizer->RemoveObserver( vopt_tag );
  }

  if( pyramid_tag )
  {
    registration->RemoveObserver( pyramid_tag );
  }

  itk::MultiThreader::SetGlobalMaximumNumberOfThreads( glob_max_threads );

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
