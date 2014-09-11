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

#include <mitkImageCast.h>

#include "itkImageFileWriter.h"
#include "itkWarpImageFilter.h"
#include "itkImageRegionIterator.h"

#include "mitkBSplineRegistration.h"
#include "itkBSplineDeformableTransform.h"
//#include "itkLBFGSOptimizer.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkResampleImageFilter.h"
#include "itkImageRegistrationMethod.h"

#include "itkBSplineDeformableTransformInitializer.h"

#include "mitkOptimizerFactory.h"
#include "mitkMetricFactory.h"

#include <itkRescaleIntensityImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>


namespace mitk {


  BSplineRegistration::BSplineRegistration():
    m_Iterations(50),
    m_ResultName("deformedImage.mhd"),
    m_SaveResult(true),
    m_SaveDeformationField(false),
    m_UpdateInputImage(false),
    m_MatchHistograms(true),
    m_Metric(0)
  {
    m_Observer = mitk::RigidRegistrationObserver::New();
  }

  BSplineRegistration::~BSplineRegistration()
  {
  }

  void BSplineRegistration::SetNumberOfIterations(int iterations)
  {
    m_Iterations = iterations;
  }

  void BSplineRegistration::SetSaveResult(bool saveResult)
  {
    m_SaveResult = saveResult;
  }

  void BSplineRegistration::SetResultFileName(const char* resultName)
  {
    m_ResultName = resultName;
  }




  template < typename TPixel, unsigned int VImageDimension >
    void BSplineRegistration::GenerateData2( const itk::Image<TPixel, VImageDimension>* itkImage1)
  {
    std::cout << "start bspline registration" << std::endl;

    // Typedefs
    typedef typename itk::Image< TPixel, VImageDimension >  InternalImageType;

    typedef typename itk::Vector< float, VImageDimension >    VectorPixelType;
    typedef typename itk::Image<  VectorPixelType, VImageDimension > DeformationFieldType;

    typedef itk::BSplineDeformableTransform<
                                double,
                                VImageDimension,
                                3 >                         TransformType;

    typedef typename TransformType::ParametersType          ParametersType;




    //typedef itk::LBFGSOptimizer                             OptimizerType;
    typedef itk::SingleValuedNonLinearOptimizer             OptimizerType;
    //typedef itk::SingleValuedCostFunction                   MetricType;

    typedef itk::MattesMutualInformationImageToImageMetric<
                                InternalImageType,
                                InternalImageType >           MetricType;

    typedef itk::MeanSquaresImageToImageMetric<
                                InternalImageType,
                                InternalImageType >           MetricTypeMS;

    typedef itk::LinearInterpolateImageFunction<
                                InternalImageType,
                                double >                    InterpolatorType;

    typedef itk::ImageRegistrationMethod<
                                InternalImageType,
                                InternalImageType >           RegistrationType;

    typedef typename itk::WarpImageFilter<
                            InternalImageType,
                            InternalImageType,
                            DeformationFieldType  >         WarperType;

    typedef typename TransformType::SpacingType                      SpacingType;

    typedef typename TransformType::OriginType                       OriginType;

    typedef itk::ResampleImageFilter<
                                InternalImageType,
                                InternalImageType >            ResampleFilterType;

    typedef itk::Image< TPixel, VImageDimension >           OutputImageType;


    // Sample new image with the same image type as the fixed image
    typedef itk::CastImageFilter<
                                InternalImageType,
                                InternalImageType >            CastFilterType;


    typedef itk::Vector< float, VImageDimension >           VectorType;
    typedef itk::Image< VectorType, VImageDimension >       DeformationFieldType;


    typedef itk::BSplineDeformableTransformInitializer <
                                TransformType,
                                InternalImageType >            InitializerType;


    typename InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
    typename RegistrationType::Pointer   registration  = RegistrationType::New();
    typename InitializerType::Pointer    initializer   = InitializerType::New();
    typename TransformType::Pointer      transform     = TransformType::New();


    if(m_Metric==0 || m_Metric==1)
    {
      typename MetricType::Pointer metric = MetricType::New();
      metric->SetNumberOfHistogramBins( 32);
      metric->SetNumberOfSpatialSamples(90000);
      registration->SetMetric(          metric       );
    }
    else{
      typename MetricTypeMS::Pointer metric = MetricTypeMS::New();
      registration->SetMetric(          metric       );
    }

    typename OptimizerFactory::Pointer optFac = OptimizerFactory::New();
    optFac->SetOptimizerParameters(m_OptimizerParameters);
    optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
    OptimizerType::Pointer optimizer = optFac->GetOptimizer();

    optimizer->AddObserver(itk::AnyEvent(), m_Observer);

    //typedef mitk::MetricFactory <TPixel, VImageDimension> MetricFactoryType;
    //typename MetricFactoryType::Pointer metricFac = MetricFactoryType::New();
    //metricFac->SetMetricParameters(m_MetricParameters);
    ////MetricType::Pointer metric = metricFac->GetMetric();


    typename InternalImageType::Pointer fixedImage = InternalImageType::New();
    mitk::CastToItkImage(m_ReferenceImage, fixedImage);
    typename InternalImageType::ConstPointer movingImage = itkImage1;
    typename InternalImageType::RegionType fixedRegion = fixedImage->GetBufferedRegion();
    typename InternalImageType::RegionType movingRegion = movingImage->GetBufferedRegion();


    if(m_MatchHistograms)
    {
      typedef itk::RescaleIntensityImageFilter<InternalImageType,InternalImageType> FilterType;
      typedef itk::HistogramMatchingImageFilter<InternalImageType,InternalImageType> HEFilterType;

      typename FilterType::Pointer inputRescaleFilter = FilterType::New();
      typename FilterType::Pointer referenceRescaleFilter = FilterType::New();

      referenceRescaleFilter->SetInput(fixedImage);
      inputRescaleFilter->SetInput(movingImage);

      TPixel desiredMinimum =  0;
      TPixel desiredMaximum =  255;

      referenceRescaleFilter->SetOutputMinimum( desiredMinimum );
      referenceRescaleFilter->SetOutputMaximum( desiredMaximum );
      referenceRescaleFilter->UpdateLargestPossibleRegion();
      inputRescaleFilter->SetOutputMinimum( desiredMinimum );
      inputRescaleFilter->SetOutputMaximum( desiredMaximum );
      inputRescaleFilter->UpdateLargestPossibleRegion();

      // Histogram match the images
      typename HEFilterType::Pointer intensityEqualizeFilter = HEFilterType::New();

      intensityEqualizeFilter->SetReferenceImage( inputRescaleFilter->GetOutput() );
      intensityEqualizeFilter->SetInput( referenceRescaleFilter->GetOutput() );
      intensityEqualizeFilter->SetNumberOfHistogramLevels( 64 );
      intensityEqualizeFilter->SetNumberOfMatchPoints( 12 );
      intensityEqualizeFilter->ThresholdAtMeanIntensityOn();
      intensityEqualizeFilter->Update();

      //fixedImage = referenceRescaleFilter->GetOutput();
      //movingImage = IntensityEqualizeFilter->GetOutput();

      fixedImage = intensityEqualizeFilter->GetOutput();
      movingImage = inputRescaleFilter->GetOutput();
    }


    //
    registration->SetOptimizer(       optimizer     );
    registration->SetInterpolator(    interpolator  );
    registration->SetFixedImage(      fixedImage    );
    registration->SetMovingImage(     movingImage   );
    registration->SetFixedImageRegion(fixedRegion   );

    initializer->SetTransform(transform);
    initializer->SetImage(fixedImage);
    initializer->SetNumberOfGridNodesInsideTheImage( m_NumberOfGridPoints );
    initializer->InitializeTransform();

    registration->SetTransform( transform );

    const unsigned int numberOfParameters = transform->GetNumberOfParameters();

    typename itk::BSplineDeformableTransform<
                                double,
                                VImageDimension,
                                3 >::ParametersType  parameters;

    parameters.set_size(numberOfParameters);
    parameters.Fill( 0.0 );
    transform->SetParameters( parameters );

    // We now pass the parameters of the current transform as the initial
    // parameters to be used when the registration process starts.
    registration->SetInitialTransformParameters( transform->GetParameters() );

    std::cout << "Intial Parameters = " << std::endl;
    std::cout << transform->GetParameters() << std::endl;


    std::cout << std::endl << "Starting Registration" << std::endl;

    try
    {
      double tstart(clock());
      registration->Update();
      double time = clock() - tstart;
      time = time / CLOCKS_PER_SEC;
      MITK_INFO << "Registration time: " << time;
    }
    catch( itk::ExceptionObject & err )
    {
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
    }

    typename OptimizerType::ParametersType finalParameters =
                      registration->GetLastTransformParameters();

    std::cout << "Last Transform Parameters" << std::endl;
    std::cout << finalParameters << std::endl;

    transform->SetParameters( finalParameters );

/*
    ResampleFilterType::Pointer       resampler = ResampleFilterType::New();
    resampler->SetTransform(          transform );
    resampler->SetInput(              movingImage );
    resampler->SetSize(               fixedImage->GetLargestPossibleRegion().GetSize() );
    resampler->SetOutputOrigin(       fixedImage->GetOrigin() );
    resampler->SetOutputSpacing(      fixedImage->GetSpacing() );
    resampler->SetOutputDirection(    fixedImage->GetDirection() );
    resampler->SetDefaultPixelValue(  100 );
    resampler->SetInterpolator(       interpolator);
    resampler->Update();*/



    // Generate deformation field
    typename DeformationFieldType::Pointer field = DeformationFieldType::New();
    field->SetRegions( movingRegion );
    field->SetOrigin( movingImage->GetOrigin() );
    field->SetSpacing( movingImage->GetSpacing() );
    field->SetDirection( movingImage->GetDirection() );
    field->Allocate();


    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi( field, movingRegion );
    fi.GoToBegin();

    typename TransformType::InputPointType  fixedPoint;
    typename TransformType::OutputPointType movingPoint;
    typename DeformationFieldType::IndexType index;

    VectorType displacement;

    while( ! fi.IsAtEnd() )
    {
      index = fi.GetIndex();
      field->TransformIndexToPhysicalPoint( index, fixedPoint );
      movingPoint = transform->TransformPoint( fixedPoint );
      displacement = movingPoint - fixedPoint;
      fi.Set( displacement );
      ++fi;
    }


    // Use the deformation field to warp the moving image
    typename WarperType::Pointer warper = WarperType::New();
    warper->SetInput( movingImage );
    warper->SetInterpolator( interpolator );
    warper->SetOutputSpacing( movingImage->GetSpacing() );
    warper->SetOutputOrigin( movingImage->GetOrigin() );
    warper->SetOutputDirection( movingImage->GetDirection() );
    warper->SetDisplacementField( field );
    warper->Update();

    typename InternalImageType::Pointer result = warper->GetOutput();

    if(m_UpdateInputImage)
    {
      Image::Pointer outputImage = this->GetOutput();
      mitk::CastToMitkImage( result, outputImage );
    }


    // Save the deformationfield resulting from the registration
    if(m_SaveDeformationField)
    {
      typedef itk::ImageFileWriter< DeformationFieldType >  FieldWriterType;
      typename FieldWriterType::Pointer fieldWriter = FieldWriterType::New();

      fieldWriter->SetInput( field );

      fieldWriter->SetFileName( m_DeformationFileName );
      try
      {
        fieldWriter->Update();
      }
      catch( itk::ExceptionObject & excp )
      {
        std::cerr << "Exception thrown " << std::endl;
        std::cerr << excp << std::endl;
        //return EXIT_FAILURE;
      }
    }



  }
} // end namespace
