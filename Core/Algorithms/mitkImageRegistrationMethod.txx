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

#include "mitkImageRegistrationMethod.h"

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
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkImageToImageMetric.h"
#include "itkSingleValuedNonLinearOptimizer.h"

#include "itkNormalizeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"

namespace mitk {
  template<typename TPixel, unsigned int VImageDimension>
    void ImageRegistrationMethod::GenerateData2(itk::Image<TPixel, VImageDimension>* itkImage1)
  {
    if (m_MetricParameters->GetMetric() != mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC || (m_MetricParameters->GetMetric() == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC && m_MetricParameters->GetUseNormalizerAndSmootherMutualInformation() == false))
    {
      typedef typename itk::Image<TPixel, VImageDimension> FixedImageType;
      typedef typename itk::Image<TPixel, VImageDimension> MovingImageType;

      typedef typename itk::Transform<double, VImageDimension, VImageDimension> TransformType;
      typedef typename itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
      typedef itk::NearestNeighborInterpolateImageFunction<MovingImageType, double> InterpolatorType2;

      typedef typename itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;

      typedef MetricFactory<TPixel, VImageDimension> MetricFactoryType;
      typedef TransformFactory<TPixel, VImageDimension> TransformFactoryType;
      typedef typename itk::SingleValuedNonLinearOptimizer OptimizerType;
      
      typename FixedImageType::Pointer fixedImage = FixedImageType::New();
      typename MovingImageType::Pointer movingImage = itkImage1;
      
      mitk::CastToItkImage(m_ReferenceImage, fixedImage);

      typename MetricFactoryType::Pointer metFac = MetricFactoryType::New();
      metFac->SetMetricParameters(m_MetricParameters);
      
      typename TransformFactoryType::Pointer transFac = TransformFactoryType::New();
      transFac->SetTransformParameters(m_TransformParameters);
      transFac->SetFixedImage(fixedImage);
      transFac->SetMovingImage(movingImage);
      
      OptimizerFactory::Pointer optFac = OptimizerFactory::New();
      optFac->SetOptimizerParameters(m_OptimizerParameters);
      
      typename TransformType::Pointer transform = transFac->GetTransform();
      optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
      typename OptimizerType::Pointer optimizer = optFac->GetOptimizer();
      
      //mitk::TransformParameters* transformParameters = mitk::TransformParameters::GetInstance();
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
      
      typename RegistrationType::Pointer registration = RegistrationType::New();
      
      registration->SetMetric(metFac->GetMetric());
      registration->SetOptimizer(optimizer);
      registration->SetTransform(transform);
      if (m_Interpolator == LINEARINTERPOLATOR)
      {
        typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
        registration->SetInterpolator(interpolator);
      }
      else if (m_Interpolator == NEARESTNEIGHBORINTERPOLATOR)
      {
        typename InterpolatorType2::Pointer interpolator = InterpolatorType2::New();
        registration->SetInterpolator(interpolator);
      }
      registration->SetFixedImage(fixedImage);
      registration->SetMovingImage(movingImage);
      registration->SetFixedImageRegion(fixedImage->GetBufferedRegion());
      registration->SetInitialTransformParameters( transform->GetParameters() );
  	  
	    // Register the Command observer with the optimizer.
	    //
	    if (m_Observer.IsNotNull())
	    {
	      m_Observer->AddStepsToDo(20);
	      optimizer->AddObserver(itk::AnyEvent(), m_Observer);
        registration->AddObserver(itk::AnyEvent(), m_Observer);
        transform->AddObserver(itk::AnyEvent(), m_Observer);
      }
      try 
      {
        registration->Update(); 
      } 
      catch(itk::ExceptionObject & err) 
      { 
        std::cerr << "ExceptionObject caught!" << std::endl; 
        std::cerr << err << std::endl; 
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
    else
    {
      typedef   float     InternalPixelType;
      typedef itk::Image< InternalPixelType,  VImageDimension> InternalImageType;

      typedef typename itk::Image<TPixel, VImageDimension> FixedImageType;
      typedef typename itk::Image<TPixel, VImageDimension> MovingImageType;

      typedef typename itk::Transform<double, VImageDimension, VImageDimension> TransformType;
      typedef typename itk::LinearInterpolateImageFunction<InternalImageType, double> InterpolatorType;
      typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double> InterpolatorType2;

      typedef typename itk::ImageRegistrationMethod<InternalImageType, InternalImageType> RegistrationType;

      typedef MetricFactory<InternalPixelType, VImageDimension> MetricFactoryType;
      typedef TransformFactory<TPixel, VImageDimension> TransformFactoryType;
      typedef typename itk::SingleValuedNonLinearOptimizer OptimizerType;
      
      typename FixedImageType::Pointer fixedImage = FixedImageType::New();
      typename MovingImageType::Pointer movingImage = itkImage1;
      
      mitk::CastToItkImage(m_ReferenceImage, fixedImage);

      typedef typename itk::NormalizeImageFilter<FixedImageType, InternalImageType> FixedNormalizeFilterType;
      typedef typename itk::NormalizeImageFilter<MovingImageType, InternalImageType> MovingNormalizeFilterType;
      typename FixedNormalizeFilterType::Pointer fixedNormalizer = FixedNormalizeFilterType::New();
      typename MovingNormalizeFilterType::Pointer movingNormalizer = MovingNormalizeFilterType::New();

      typedef itk::DiscreteGaussianImageFilter< InternalImageType, InternalImageType> GaussianFilterType;
      typename GaussianFilterType::Pointer fixedSmoother  = GaussianFilterType::New();
      typename GaussianFilterType::Pointer movingSmoother = GaussianFilterType::New();

      fixedSmoother->SetVariance(m_MetricParameters->GetFixedSmootherVarianceMutualInformation());
      movingSmoother->SetVariance(m_MetricParameters->GetMovingSmootherVarianceMutualInformation());

      fixedNormalizer->SetInput(  fixedImage );
      movingNormalizer->SetInput( movingImage );

      fixedSmoother->SetInput( fixedNormalizer->GetOutput() );
      movingSmoother->SetInput( movingNormalizer->GetOutput() );

      typename MetricFactoryType::Pointer metFac = MetricFactoryType::New();
      metFac->SetMetricParameters(m_MetricParameters);
      
      typename TransformFactoryType::Pointer transFac = TransformFactoryType::New();
      transFac->SetTransformParameters(m_TransformParameters);
      transFac->SetFixedImage(fixedImage);
      transFac->SetMovingImage(movingImage);
      
      OptimizerFactory::Pointer optFac = OptimizerFactory::New();
      optFac->SetOptimizerParameters(m_OptimizerParameters);
      
      typename TransformType::Pointer transform = transFac->GetTransform();
      optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
      typename OptimizerType::Pointer optimizer = optFac->GetOptimizer();
      
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
      
      typename RegistrationType::Pointer registration = RegistrationType::New();
      
      registration->SetMetric(metFac->GetMetric());
      registration->SetOptimizer(optimizer);
      registration->SetTransform(transform);
      if (m_Interpolator == LINEARINTERPOLATOR)
      {
        typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
        registration->SetInterpolator(interpolator);
      }
      else if (m_Interpolator == NEARESTNEIGHBORINTERPOLATOR)
      {
        typename InterpolatorType2::Pointer interpolator = InterpolatorType2::New();
        registration->SetInterpolator(interpolator);
      }
      registration->SetFixedImage(    fixedSmoother->GetOutput()    );
      registration->SetMovingImage(   movingSmoother->GetOutput()   );
      fixedNormalizer->Update();
      typename FixedImageType::RegionType fixedImageRegion = fixedNormalizer->GetOutput()->GetBufferedRegion();
      registration->SetFixedImageRegion( fixedImageRegion );
      registration->SetInitialTransformParameters( transform->GetParameters() );
  	  
	    // Register the Command observer with the optimizer.
	    //
	    if (m_Observer.IsNotNull())
	    {
	      m_Observer->AddStepsToDo(20);
	      optimizer->AddObserver(itk::AnyEvent(), m_Observer);
        registration->AddObserver(itk::AnyEvent(), m_Observer);
        transform->AddObserver(itk::AnyEvent(), m_Observer);
      }
      try 
      {
        registration->Update(); 
      } 
      catch(itk::ExceptionObject & err) 
      { 
        std::cerr << "ExceptionObject caught!" << std::endl; 
        std::cerr << err << std::endl; 
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
  }
} // end namespace
