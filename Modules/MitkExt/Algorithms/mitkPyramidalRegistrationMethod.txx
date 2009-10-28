/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-09-05 13:45:48 +0200 (Mi, 05 Sep 2007) $
Version:   $Revision: 9502 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPyramidalRegistrationMethod.h"

#include "mitkMetricFactory.h"
#include "mitkTransformFactory.h"
#include "mitkOptimizerFactory.h"
#include "mitkTransformParameters.h"
#include "mitkMetricParameters.h"
#include <mitkITKImageImport.h>
#include "mitkRigidRegistrationObserver.h"

#include "itkResampleImageFilter.h"
#include "itkImageRegistrationMethod.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImage.h"

#include "itkImageToImageMetric.h"
#include "itkSingleValuedNonLinearOptimizer.h"


#include "itkDiscreteGaussianImageFilter.h"

#include "mitkRegistrationInterfaceCommand.h"

#include <itkRescaleIntensityImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>
#include <itkCastImageFilter.h>


namespace mitk {

 
  template<typename TPixel, unsigned int VImageDimension>
  void PyramidalRegistrationMethod::GenerateData2(itk::Image<TPixel, VImageDimension>* itkImage1)
  {    

    // TYPEDEFS        
    typedef itk::Image< TPixel, VImageDimension >             ImageType;
		    
		typedef float                                             InternalPixelType;
		typedef itk::Image< InternalPixelType, VImageDimension >  InternalImageType;

    typedef typename itk::Transform
      < double, VImageDimension, VImageDimension >            TransformType;
    typedef mitk::TransformFactory<InternalPixelType, 
                                   VImageDimension>           TransformFactoryType;
    typedef itk::LinearInterpolateImageFunction
      < InternalImageType, double >                           InterpolatorType;
    typedef mitk::MetricFactory
      <InternalPixelType, VImageDimension>                    MetricFactoryType;
    typedef itk::RecursiveMultiResolutionPyramidImageFilter<
																			InternalImageType,
																			InternalImageType >	    ImagePyramidType;
    typedef itk::DiscreteGaussianImageFilter
      <ImageType, InternalImageType>                          GaussianFilterType;    
   


    typedef itk::MultiResolutionImageRegistrationMethod< 
																			InternalImageType, 
																			InternalImageType >     RegistrationType;	
    typedef RegistrationInterfaceCommand
      <RegistrationType, TPixel>                              CommandType;

    typedef itk::CastImageFilter<ImageType, 
                                  InternalImageType>          CastImageFilterType;

    itk::Array<double> initialParameters;
    if(m_TransformParameters->GetInitialParameters().size())
    {
      initialParameters = m_TransformParameters->GetInitialParameters();
    }
   
    // LOAD PARAMETERS
    itk::Array<double> transformValues = m_Preset->getTransformValues(m_Presets[0]);
    itk::Array<double> metricValues = m_Preset->getMetricValues(m_Presets[0]);
    itk::Array<double> optimizerValues = m_Preset->getOptimizerValues(m_Presets[0]);
    m_TransformParameters = ParseTransformParameters(transformValues);
    m_MetricParameters = ParseMetricParameters(metricValues);
    m_OptimizerParameters = ParseOptimizerParameters(optimizerValues);  		


    // The fixed and the moving image
		typename InternalImageType::Pointer fixedImage = InternalImageType::New();
    typename InternalImageType::Pointer movingImage = InternalImageType::New();
		mitk::CastToItkImage(m_ReferenceImage, fixedImage); 

     // Blur the moving image        
    if(m_BlurMovingImage)
    {
      typename GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
      gaussianFilter->SetInput(itkImage1);
      gaussianFilter->SetVariance(6.0);        
      gaussianFilter->Update();
      movingImage = gaussianFilter->GetOutput();
    } else{
      typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
      castImageFilter->SetInput(itkImage1);
      castImageFilter->Update();
      movingImage = castImageFilter->GetOutput();
    }
    
    if(m_MatchHistograms)
    {
      MatchHistograms(movingImage.GetPointer(), fixedImage.GetPointer());
    }
    
 
   
    typename TransformFactoryType::Pointer transFac = TransformFactoryType::New();
    transFac->SetTransformParameters(m_TransformParameters);
    transFac->SetFixedImage(fixedImage);
    transFac->SetMovingImage(movingImage);
    typename TransformType::Pointer transform = transFac->GetTransform();
    
		
    typename InterpolatorType::Pointer   interpolator  = InterpolatorType::New();         
    typename MetricFactoryType::Pointer metFac = MetricFactoryType::New();
    metFac->SetMetricParameters(m_MetricParameters);

    
    typename OptimizerFactory::Pointer optFac = OptimizerFactory::New();
    optFac->SetOptimizerParameters(m_OptimizerParameters);
    optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
    typename OptimizerType::Pointer optimizer = optFac->GetOptimizer();	

    // optimizer scales
    if (m_TransformParameters->GetUseOptimizerScales())
    {
      itk::Array<double> optimizerScales = m_TransformParameters->GetScales();
      typename OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );
      for (unsigned int i = 0; i < scales.Size(); i++)
      {
        scales[i] = optimizerScales[i];
      }
      optimizer->SetScales( scales );
    }
	
		typename ImagePyramidType::Pointer fixedImagePyramid = 	ImagePyramidType::New();
		typename ImagePyramidType::Pointer movingImagePyramid = ImagePyramidType::New();  
    
    if(m_FixedSchedule.size() > 0 && m_MovingSchedule.size() > 0)
    {
      fixedImagePyramid->SetSchedule(m_FixedSchedule);
      movingImagePyramid->SetSchedule(m_MovingSchedule);
      // Otherwise just use the default schedule
    }


    

		typename RegistrationType::Pointer registration = RegistrationType::New();
		registration->SetOptimizer(           optimizer     );
		registration->SetTransform(           transform     );
		registration->SetInterpolator(        interpolator  );
    registration->SetMetric(              metFac->GetMetric()  );        
    registration->SetFixedImagePyramid(   fixedImagePyramid );
		registration->SetMovingImagePyramid(  movingImagePyramid );
	  registration->SetFixedImage(          fixedImage );     
		registration->SetMovingImage(         movingImage );	
		registration->SetFixedImageRegion(    fixedImage->GetBufferedRegion() );     
    
    if(transFac->GetTransformParameters()->GetInitialParameters().size())
    {
      registration->SetInitialTransformParameters( transFac->GetTransformParameters()->GetInitialParameters() );
    }
    else
    {
      itk::Array<double> zeroInitial;
      zeroInitial.set_size(transform->GetNumberOfParameters());
      zeroInitial.fill(0.0);
      zeroInitial[0] = 1.0;
      zeroInitial[4] = 1.0;
      zeroInitial[8] = 1.0;
      registration->SetInitialTransformParameters( zeroInitial );
    }

   
    if(m_UseMask)
    {
      itk::ImageMaskSpatialObject< VImageDimension>* mask = 
        dynamic_cast< itk::ImageMaskSpatialObject< VImageDimension>* > ( m_BrainMask.GetPointer() );
      registration->GetMetric()->SetFixedImageMask(mask);
    }
    
    // registering command observer with the optimizer        
		if (m_Observer.IsNotNull())
		{
		  m_Observer->AddStepsToDo(20);
		  optimizer->AddObserver(itk::AnyEvent(), m_Observer);
		  registration->AddObserver(itk::AnyEvent(), m_Observer);
		  transform->AddObserver(itk::AnyEvent(), m_Observer);
		}			

    typename CommandType::Pointer command = CommandType::New();
    command->observer = m_Observer;
    command->m_Presets = m_Presets;
    command->m_UseMask = m_UseMask;
    command->m_BrainMask = m_BrainMask;

		registration->AddObserver( itk::IterationEvent(), command );        
    registration->SetSchedules(m_FixedSchedule, m_MovingSchedule);        
    

    // Start the registration process
		try 
		{ 
		  registration->StartRegistration(); 
		} 
		catch( itk::ExceptionObject & err ) 
		{ 
		  std::cout << "ExceptionObject caught !" << std::endl; 
		  std::cout << err << std::endl; 				
		}
    if (m_Observer.IsNotNull())
    {
      optimizer->RemoveAllObservers();
      registration->RemoveAllObservers();
      transform->RemoveAllObservers();
      m_Observer->SetRemainingProgress(15);
    }
    if (m_Observer.IsNotNull())
    {
      m_Observer->SetRemainingProgress(5);
    }

  } 

  template < typename T, unsigned int Dim >
  void PyramidalRegistrationMethod::MatchHistograms(itk::Image<T, Dim> *referenceImage, itk::Image<T, Dim> *inputImage)
  {
    // First scale both histograms to values [0, 255]
    // Then align the quantiles of the inputImage histogram with the corresponding quantiles of
    // the reference Image
    typedef itk::Image<T, Dim>  ImageType;
    typedef itk::RescaleIntensityImageFilter<ImageType,ImageType> FilterType;
   
    typedef itk::HistogramMatchingImageFilter<ImageType,ImageType> HEFilterType;

    typename FilterType::Pointer inputRescalefilter = FilterType::New();  
    typename FilterType::Pointer referenceRescalefilter = FilterType::New();  

    referenceRescalefilter->SetInput(referenceImage);
    inputRescalefilter->SetInput(inputImage);

    const float desiredMinimum =  0.0;
    const float desiredMaximum =  255.0;
    
    referenceRescalefilter->SetOutputMinimum( desiredMinimum );
    referenceRescalefilter->SetOutputMaximum( desiredMaximum );
    referenceRescalefilter->UpdateLargestPossibleRegion();  
    inputRescalefilter->SetOutputMinimum( desiredMinimum );
    inputRescalefilter->SetOutputMaximum( desiredMaximum );
    inputRescalefilter->UpdateLargestPossibleRegion();

    // Histogram match the images
    
    typename HEFilterType::Pointer IntensityEqualizeFilter = HEFilterType::New();

    IntensityEqualizeFilter->SetReferenceImage( referenceRescalefilter->GetOutput() );
    IntensityEqualizeFilter->SetInput( inputRescalefilter->GetOutput() );
    IntensityEqualizeFilter->SetNumberOfHistogramLevels( 64 );
    IntensityEqualizeFilter->SetNumberOfMatchPoints( 12 );
    IntensityEqualizeFilter->ThresholdAtMeanIntensityOn();
    IntensityEqualizeFilter->Update();

    referenceImage = referenceRescalefilter->GetOutput();
    inputImage = IntensityEqualizeFilter->GetOutput();


  }

} // end namespace
