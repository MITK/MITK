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

#include "mitkPyramidalRegistrationMethodAccessFunctor.h"
#include "mitkPyramidalRegistrationMethod.h"

#include <mitkImageCast.h>

#include <itkLinearInterpolateImageFunction.h>
#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>

#include "mitkMetricFactory.h"
#include "mitkTransformFactory.h"
#include "mitkOptimizerFactory.h"
#include "mitkRegistrationInterfaceCommand.h"

namespace mitk {

template<typename TPixel, unsigned int VImageDimension>
void PyramidalRegistrationMethodAccessFunctor::AccessItkImage(const itk::Image<TPixel, VImageDimension>* itkImage1,
                                                              PyramidalRegistrationMethod* method)
{

  // TYPEDEFS
  typedef itk::Image< TPixel, VImageDimension >                                                  ImageType;

  typedef float                                                                                  InternalPixelType;
  typedef itk::Image<InternalPixelType, VImageDimension>                                         InternalImageType;

  typedef typename itk::Transform<double, VImageDimension, VImageDimension>                      TransformType;
  typedef mitk::TransformFactory<InternalPixelType, VImageDimension>                             TransformFactoryType;
  typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                         InterpolatorType;
  typedef mitk::MetricFactory<InternalPixelType, VImageDimension>                                MetricFactoryType;
  typedef itk::RecursiveMultiResolutionPyramidImageFilter<InternalImageType, InternalImageType > ImagePyramidType;
  typedef itk::DiscreteGaussianImageFilter<ImageType, InternalImageType>                         GaussianFilterType;

  typedef itk::MultiResolutionImageRegistrationMethod<InternalImageType, InternalImageType >     RegistrationType;
  typedef RegistrationInterfaceCommand<RegistrationType, TPixel>                                 CommandType;

  typedef itk::CastImageFilter<ImageType, InternalImageType>                                     CastImageFilterType;

  itk::Array<double> initialParameters;
  if(method->m_TransformParameters->GetInitialParameters().size())
  {
    initialParameters = method->m_TransformParameters->GetInitialParameters();
  }

  // LOAD PARAMETERS
  itk::Array<double> transformValues = method->m_Preset->getTransformValues(method->m_Presets[0]);
  itk::Array<double> metricValues = method->m_Preset->getMetricValues(method->m_Presets[0]);
  itk::Array<double> optimizerValues = method->m_Preset->getOptimizerValues(method->m_Presets[0]);
  method->m_TransformParameters = method->ParseTransformParameters(transformValues);
  method->m_MetricParameters = method->ParseMetricParameters(metricValues);
  method->m_OptimizerParameters = method->ParseOptimizerParameters(optimizerValues);


  // The fixed and the moving image
  typename InternalImageType::Pointer fixedImage = InternalImageType::New();
  typename InternalImageType::Pointer movingImage = InternalImageType::New();

  mitk::CastToItkImage(method->m_ReferenceImage, fixedImage);

  // Blur the moving image
  if(method->m_BlurMovingImage)
  {
    typename GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
    gaussianFilter->SetInput(itkImage1);
    gaussianFilter->SetVariance(6.0);
    gaussianFilter->SetMaximumError( 0.1 );
    //gaussianFilter->SetMaximumKernelWidth ( 3 );
    gaussianFilter->Update();
    movingImage = gaussianFilter->GetOutput();
  }
  else
  {
    typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
    castImageFilter->SetInput(itkImage1);
    castImageFilter->Update();
    movingImage = castImageFilter->GetOutput();
  }

  if(method->m_MatchHistograms)
  {
    typedef itk::RescaleIntensityImageFilter<InternalImageType,InternalImageType> FilterType;
    typedef itk::HistogramMatchingImageFilter<InternalImageType,InternalImageType> HEFilterType;

    typename FilterType::Pointer inputRescaleFilter = FilterType::New();
    typename FilterType::Pointer referenceRescaleFilter = FilterType::New();

    referenceRescaleFilter->SetInput(fixedImage);
    inputRescaleFilter->SetInput(movingImage);

    const float desiredMinimum =  0.0;
    const float desiredMaximum =  255.0;

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

  typename TransformFactoryType::Pointer transFac = TransformFactoryType::New();
  transFac->SetTransformParameters(method->m_TransformParameters);
  transFac->SetFixedImage(fixedImage);
  transFac->SetMovingImage(movingImage);
  typename TransformType::Pointer transform = transFac->GetTransform();

  typename InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  typename MetricFactoryType::Pointer metFac = MetricFactoryType::New();
  metFac->SetMetricParameters(method->m_MetricParameters);

  typename OptimizerFactory::Pointer optFac = OptimizerFactory::New();
  optFac->SetOptimizerParameters(method->m_OptimizerParameters);
  optFac->SetNumberOfTransformParameters(transform->GetNumberOfParameters());
  typename PyramidalRegistrationMethod::OptimizerType::Pointer optimizer = optFac->GetOptimizer();

  // optimizer scales
  if (method->m_TransformParameters->GetUseOptimizerScales())
  {
    itk::Array<double> optimizerScales = method->m_TransformParameters->GetScales();
    typename PyramidalRegistrationMethod::OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );
    for (unsigned int i = 0; i < scales.Size(); i++)
    {
      scales[i] = optimizerScales[i];
    }
    optimizer->SetScales( scales );
  }

  typename ImagePyramidType::Pointer fixedImagePyramid = ImagePyramidType::New();
  typename ImagePyramidType::Pointer movingImagePyramid = ImagePyramidType::New();

  if(method->m_FixedSchedule.size() > 0 && method->m_MovingSchedule.size() > 0)
  {
    fixedImagePyramid->SetSchedule(method->m_FixedSchedule);
    movingImagePyramid->SetSchedule(method->m_MovingSchedule);
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

  if(method->m_UseMask)
  {
    itk::ImageMaskSpatialObject< VImageDimension>* mask =
        dynamic_cast< itk::ImageMaskSpatialObject< VImageDimension>* > ( method->m_BrainMask.GetPointer() );
    registration->GetMetric()->SetFixedImageMask(mask);
  }

  // registering command observer with the optimizer
  if (method->m_Observer.IsNotNull())
  {
    method->m_Observer->AddStepsToDo(20);
    optimizer->AddObserver(itk::AnyEvent(), method->m_Observer);
    registration->AddObserver(itk::AnyEvent(), method->m_Observer);
    transform->AddObserver(itk::AnyEvent(), method->m_Observer);
  }

  typename CommandType::Pointer command = CommandType::New();
  command->observer = method->m_Observer;
  command->m_Presets = method->m_Presets;
  command->m_UseMask = method->m_UseMask;
  command->m_BrainMask = method->m_BrainMask;

  registration->AddObserver( itk::IterationEvent(), command );
  registration->SetSchedules(method->m_FixedSchedule, method->m_MovingSchedule);

  // Start the registration process
  try
  {
    registration->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
  }
  if (method->m_Observer.IsNotNull())
  {
    optimizer->RemoveAllObservers();
    registration->RemoveAllObservers();
    transform->RemoveAllObservers();
    method->m_Observer->SetRemainingProgress(15);
  }
  if (method->m_Observer.IsNotNull())
  {
    method->m_Observer->SetRemainingProgress(5);
  }

}

} // end namespace
