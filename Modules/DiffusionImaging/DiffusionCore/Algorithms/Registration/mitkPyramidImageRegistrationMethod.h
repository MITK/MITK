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

#ifndef MITKPYRAMIDIMAGEREGISTRATION_H
#define MITKPYRAMIDIMAGEREGISTRATION_H

#include <DiffusionCoreExports.h>

#include <itkObject.h>
#include "itkImageMaskSpatialObject.h"
#include "itkNotImageFilter.h"
#include <itkCenteredVersorTransformInitializer.h>
#include <vnl/vnl_matrix_fixed.h>

#include "mitkImage.h"
#include "mitkBaseProcess.h"

#include "mitkPyramidRegistrationMethodHelper.h"
#include <itkWindowedSincInterpolateImageFunction.h>

#include "mitkImageToItk.h"
#include "mitkImageCast.h"
#include "mitkImageCaster.h"
#include "mitkITKImageImport.h"
#include "mitkIOUtil.h"


namespace mitk
{

/**
 * @brief The PyramidImageRegistration class implements a multi-scale registration method
 *
 * The PyramidImageRegistration class is suitable for aligning (f.e.) brain MR images. The method offers two
 * transform types
 *   - Rigid: optimizing translation and rotation only and
 *   - Affine ( default ): with scaling in addition ( 12 DOF )
 *
 * The error metric is internally chosen based on the selected task type ( @sa SetCrossModalityOn )
 * It uses
 *   - MattesMutualInformation for CrossModality=on ( default ) and
 *   - NormalizedCorrelation for CrossModality=off.
 */
class DiffusionCore_EXPORT PyramidImageRegistrationMethod :
    public itk::Object
{
public:

  /** Typedefs */
  mitkClassMacro(PyramidImageRegistrationMethod, itk::Object)

  /** Smart pointer support */
  itkNewMacro(Self)

  /** Typedef for the transformation matrix, corresponds to the InternalMatrixType from ITK transforms */
  typedef vnl_matrix_fixed< double, 3, 3> TransformMatrixType;

  typedef itk::AffineTransform< double >  AffineTransformType;
  typedef itk::Euler3DTransform< double >  RigidTransformType;

  /** Registration is between modalities - will take MattesMutualInformation as error metric */
  void SetCrossModalityOn()
  {
    m_CrossModalityRegistration = true;
  }

  /** Registration is between modalities - will take NormalizedCorrelation as error metric */
  void SetCrossModalityOff()
  {
    m_CrossModalityRegistration = false;
  }

  /** Turn the cross-modality on/off */
  void SetCrossModality(bool flag)
  {
    if( flag )
      this->SetCrossModalityOn();
    else
      this->SetCrossModalityOff();
  }

  /** Controll the verbosity of the registration output
   *
   * for true, each iteration step of the optimizer is watched and passed to the std::cout
   */
  void SetVerbose(bool flag)
  {
    if( flag )
      this->SetVerboseOn();
    else
      this->SetVerboseOff();
  }

  /** Turn verbosity on, \sa SetVerbose */
  void SetVerboseOn()
  {
    m_Verbose = true;
  }

  /** Turn verbosity off, \sa SetVerbose */
  void SetVerboseOff()
  {
    m_Verbose = false;
  }

  /** A rigid ( 6dof : translation + rotation ) transform is optimized */
  void SetTransformToRigid()
  {
    m_UseAffineTransform = false;
  }

  /** An affine ( 12dof : Rigid + scale + skew ) transform is optimized */
  void SetTransformToAffine()
  {
    m_UseAffineTransform = true;
  }

  /** Input image, the reference one */
  void SetFixedImage( mitk::Image::Pointer );

  /** Input image, the one to be transformed */
  void SetMovingImage( mitk::Image::Pointer );

  /** Fixed image mask, excludes the masked voxels from the registration metric*/
  void SetFixedImageMask( mitk::Image::Pointer mask);


  void SetInitializeByGeometry(bool flag)
  {
    m_InitializeByGeometry = flag;
  }

  void Update();

  /**
   * @brief Get the number of parameters optimized ( 12 or 6 )
   *
   * @return number of paramters
   */
  unsigned int GetNumberOfParameters()
  {
    unsigned int retValue = 12;
    if(!m_UseAffineTransform)
      retValue = 6;

    return retValue;
  }

  /**
   * @brief Copies the estimated parameters to the given array
   * @param paramArray, target array for copy, make sure to allocate enough space
   */
  void GetParameters( double* paramArray)
  {
    if( m_EstimatedParameters == NULL )
    {
      mitkThrow() << "No parameters were estimated yet, call Update() first.";
    }

    unsigned int dim = 12;
    if( !m_UseAffineTransform )
      dim = 6;

    for( unsigned int i=0; i<dim; i++)
    {
      *(paramArray+i) = m_EstimatedParameters[i];
    }
  }

  /**
   * @brief Control the interpolator used for resampling.
   *
   * The class uses the
   *  - Linear interpolator on default ( flag = false )
   *  - WindowedSinc interpolator if called with true
   */
  void SetUseAdvancedInterpolation( bool flag)
  {
    m_UseWindowedSincInterpolator = flag;
  }

  /**
   * @brief Control the interpolator used for resampling.
   *
   * The class uses the
   *  - Linear interpolator on default ( flag = false )
   *  - Nearest neighbor interpolation if this is true
   *
   *  used to resample e.g. segmentations.
   */
  void SetUseNearestNeighborInterpolation( bool flag)
  {
    m_UseNearestNeighborInterpolator = flag;
  }

  /**
   * @brief Set if fixed image mask is used to exclude a region.
   * @param flag , true if mask is to be used, false if mask is to be ignored (default)
   */
  void SetUseFixedImageMask( bool flag)
  {
    m_UseMask = flag;
  }

  /**
   * @brief Returns the moving image transformed according to the estimated transformation and resampled
   * to the geometry of the fixed/resampling reference image
   *
   */
  mitk::Image::Pointer GetResampledMovingImage();

  /**
   * @brief Returns a provided moving image transformed according to the given transformation and resampled
   * to the geometry of the fixed/resampling reference image
   */
  mitk::Image::Pointer GetResampledMovingImage(mitk::Image::Pointer movingImage, double *transform );

  /**
   * @brief Get the rotation part of the transformation as a vnl_fixed_matrix<double, 3,3>
   *
   * It returns identity if the internal parameters are not-yet allocated ( i.e. the filter did not run yet )
   *
   * @return \sa TransformMatrixType
   */
  TransformMatrixType GetLastRotationMatrix();


protected:
  PyramidImageRegistrationMethod();

  ~PyramidImageRegistrationMethod();

  /** Fixed image, used as reference for registration */
  mitk::Image::Pointer m_FixedImage;

  /** Moving image, will be transformed */
  mitk::Image::Pointer m_MovingImage;

  mitk::Image::Pointer m_FixedImageMask;

  bool m_CrossModalityRegistration;

  bool m_UseAffineTransform;

  bool m_UseWindowedSincInterpolator;

  bool m_UseNearestNeighborInterpolator;

  bool m_UseMask;

  double* m_EstimatedParameters;

  /** Control the verbosity of the regsitistration output */
  bool m_Verbose;

  bool m_InitializeByGeometry;

  template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
  void RegisterTwoImages(itk::Image<TPixel1, VImageDimension1>* itkImage1, itk::Image<TPixel2, VImageDimension2>* itkImage2)
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

    if( m_CrossModalityRegistration )
    {
      metric = MMIMetricType::New();
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

    registration->AddObserver( itk::IterationEvent(), pyramid_observer );

    try
    {
      registration->Update();
    }
    catch (itk::ExceptionObject &e)
    {
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

    // remove optimizer tag if used
    if( vopt_tag )
    {
      optimizer->RemoveObserver( vopt_tag );
    }


  }

  template< typename TPixel, unsigned int VDimension>
  void ResampleMitkImage( itk::Image<TPixel, VDimension>* itkImage,
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

    resampler->SetInput( itkImage );
    resampler->SetTransform( base_transform );
    resampler->SetReferenceImage( reference_image );
    resampler->UseReferenceImageOn();

    resampler->Update();

    mitk::GrabItkImageMemory( resampler->GetOutput(), outputImage);
  }

};

} // end namespace

#endif // MITKPYRAMIDIMAGEREGISTRATION_H
