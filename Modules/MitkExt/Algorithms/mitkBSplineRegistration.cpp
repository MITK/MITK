/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-05 19:02:32 +0200 (Tue, 05 May 2009) $
Version:   $Revision: 17105 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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


namespace mitk {

  
  BSplineRegistration::BSplineRegistration():
    m_Iterations(50),    
    m_ResultName("deformedImage.mhd"),   
    m_SaveResult(true),
    m_SaveDeformationField(false),
    m_UpdateInputImage(false)
  {

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
    void BSplineRegistration::GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1)
  {
    std::cout << "start bspline registration" << std::endl;
    
    // Typedefs
    typedef typename itk::Image< TPixel, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixel, VImageDimension >  MovingImageType;

   
    typedef typename itk::Vector< float, VImageDimension >    VectorPixelType;
    typedef typename itk::Image<  VectorPixelType, VImageDimension > DeformationFieldType;

    typedef itk::BSplineDeformableTransform<
                                double,
                                VImageDimension,
                                3 >                         TransformType;

    typedef typename TransformType::ParametersType          ParametersType;
    
    //typedef itk::LBFGSOptimizer                             OptimizerType;
    typedef itk::SingleValuedNonLinearOptimizer             OptimizerType;
    typedef itk::MattesMutualInformationImageToImageMetric<
                                FixedImageType,
                                MovingImageType >           MetricType;

    /*typedef itk::MeanSquaresImageToImageMetric<
                                FixedImageType,
                                MovingImageType >           MetricType;*/

    typedef itk::LinearInterpolateImageFunction<
                                MovingImageType,
                                double >                    InterpolatorType;

    typedef itk::ImageRegistrationMethod<
                                FixedImageType,
                                MovingImageType >           RegistrationType;

    typedef typename itk::WarpImageFilter<
                            MovingImageType, 
                            MovingImageType,
                            DeformationFieldType  >         WarperType;

    typedef typename TransformType::SpacingType                      SpacingType;

    typedef typename TransformType::OriginType                       OriginType;

    typedef itk::ResampleImageFilter< 
                                MovingImageType, 
                                FixedImageType >            ResampleFilterType;

    typedef itk::Image< TPixel, VImageDimension >           OutputImageType;
  

    // Sample new image with the same image type as the fixed image
    typedef itk::CastImageFilter< 
                                FixedImageType,
                                FixedImageType >            CastFilterType;
                    
    
    typedef itk::Vector< float, VImageDimension >           VectorType;
    typedef itk::Image< VectorType, VImageDimension >       DeformationFieldType;


    typedef itk::BSplineDeformableTransformInitializer <
                                TransformType,
                                FixedImageType >            InitializerType;
    

    typename InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
    typename RegistrationType::Pointer   registration  = RegistrationType::New();   
    typename InitializerType::Pointer    initializer   = InitializerType::New();
    typename TransformType::Pointer      transform     = TransformType::New();
    typename MetricType::Pointer         metric        = MetricType::New();
    
    metric->SetNumberOfHistogramBins( 24);
    metric->SetNumberOfSpatialSamples(100000);
     
    typename OptimizerFactory::Pointer optFac = OptimizerFactory::New();
    optFac->SetOptimizerParameters(m_OptimizerParameters);
    optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
    OptimizerType::Pointer optimizer = optFac->GetOptimizer();    

    typename FixedImageType::Pointer fixedImage = FixedImageType::New();
    mitk::CastToItkImage(m_ReferenceImage, fixedImage);    
    typename MovingImageType::Pointer movingImage = itkImage1;
    typename FixedImageType::RegionType fixedRegion = fixedImage->GetBufferedRegion();
    typename MovingImageType::RegionType movingRegion = movingImage->GetBufferedRegion();

    registration->SetMetric(          metric        );
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
      registration->StartRegistration();       
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
    warper->SetDeformationField( field );
    warper->Update();

    typename MovingImageType::Pointer result = warper->GetOutput();    

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
