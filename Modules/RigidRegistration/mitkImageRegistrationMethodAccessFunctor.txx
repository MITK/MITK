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

#include "mitkImageRegistrationMethodAccessFunctor.h"
#include "mitkImageRegistrationMethod.h"

#include <mitkImageCast.h>

#include <itkLinearInterpolateImageFunction.h>

namespace mitk {

template<typename TPixel, unsigned int VImageDimension>
void ImageRegistrationMethodAccessFunctor::AccessItkImage(itk::Image<TPixel, VImageDimension>* itkImage1,
                                                          ImageRegistrationMethod* method)
{
  //convert mitk masks to itk masks
  typedef typename itk::Image<TPixel, VImageDimension> FixedImageType;
  typedef typename itk::Image<TPixel, VImageDimension> MovingImageType;
  typedef typename itk::Image< unsigned char, VImageDimension >  MaskImageType;
  typedef typename itk::ImageMaskSpatialObject< VImageDimension > ImageMaskType;
  typename ImageMaskType::Pointer movingImageMask;
  if(method->m_MovingMask.IsNotNull())
  {
    typename MovingImageType::Pointer movingMask = MovingImageType::New();
    mitk::CastToItkImage(method->m_MovingMask, movingMask);
    typename itk::CastImageFilter<MovingImageType, MaskImageType>::Pointer maskImageCaster = itk::CastImageFilter<MovingImageType, MaskImageType>::New();
    maskImageCaster->SetInput(movingMask);
    maskImageCaster->UpdateLargestPossibleRegion();
    movingImageMask = ImageMaskType::New();
    movingImageMask->SetImage(maskImageCaster->GetOutput());
  }
  typename ImageMaskType::Pointer fixedImageMask;
  if(method->m_FixedMask.IsNotNull())
  {
    typename FixedImageType::Pointer fixedMask = FixedImageType::New();
    mitk::CastToItkImage(method->m_FixedMask, fixedMask);
    typename itk::CastImageFilter<FixedImageType, MaskImageType>::Pointer maskImageCaster = itk::CastImageFilter<FixedImageType, MaskImageType>::New();
    maskImageCaster->SetInput(fixedMask);
    maskImageCaster->UpdateLargestPossibleRegion();
    fixedImageMask = ImageMaskType::New();
    fixedImageMask->SetImage(maskImageCaster->GetOutput());
  }
  // typedefs
  typedef typename itk::Image<TPixel, VImageDimension> FixedImageType;
  typedef typename itk::Image<TPixel, VImageDimension> MovingImageType;
  typedef typename itk::LinearInterpolateImageFunction<MovingImageType, double> InterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<MovingImageType, double> InterpolatorType2;
  typedef typename itk::ImageRegistrationMethod<FixedImageType, MovingImageType> RegistrationType;
  typedef typename itk::Transform< double, VImageDimension, VImageDimension >    TransformType;
  typedef typename TransformType::Pointer                TransformPointer;
  typedef typename itk::ImageToImageMetric<FixedImageType, MovingImageType>    MetricType;
  typedef typename MetricType::Pointer                MetricPointer;
  typedef typename itk::SingleValuedNonLinearOptimizer OptimizerType;
  // the fixed and the moving image
  typename FixedImageType::Pointer fixedImage = FixedImageType::New();
  typename MovingImageType::Pointer movingImage = itkImage1;
  mitk::CastToItkImage(method->m_ReferenceImage, fixedImage);
  // the metric
  MetricPointer metric = dynamic_cast<MetricType*>(method->m_Metric.GetPointer());
  if(movingImageMask.IsNotNull())
    metric->SetMovingImageMask(movingImageMask);
  if(fixedImageMask.IsNotNull())
    metric->SetFixedImageMask(fixedImageMask);
  // the transform
  TransformPointer transform = dynamic_cast<TransformType*>(method->m_Transform.GetPointer());
  // the optimizer
  typename OptimizerType::Pointer optimizer = dynamic_cast<OptimizerType*>(method->m_Optimizer.GetPointer());
  // optimizer scales
  if (method->m_OptimizerScales.Size() != 0)
  {
    typename OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );
    for (unsigned int i = 0; i < scales.Size(); i++)
    {
      scales[i] = method->m_OptimizerScales[i];
    }
    optimizer->SetScales( scales );
  }
  // the registration method
  typename RegistrationType::Pointer registration = RegistrationType::New();
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetTransform(transform);
  registration->SetFixedImage(fixedImage);
  registration->SetMovingImage(movingImage);
  registration->SetFixedImageRegion(fixedImage->GetBufferedRegion());
//     if(transFac->GetTransformParameters()->GetInitialParameters().size())
//     {
//       registration->SetInitialTransformParameters( transFac->GetTransformParameters()->GetInitialParameters() );
//     }
//     else
//     {
    itk::Array<double> zeroInitial;
    zeroInitial.set_size(transform->GetNumberOfParameters());
    zeroInitial.fill(0.0);
    if (zeroInitial.size() >= 1)
    {
      zeroInitial[0] = 1.0;
    }
    if (zeroInitial.size() >= 5)
    {
      zeroInitial[4] = 1.0;
    }
    if (zeroInitial.size() >= 9)
    {
      zeroInitial[8] = 1.0;
    }
    registration->SetInitialTransformParameters( zeroInitial );
    optimizer->SetInitialPosition( zeroInitial );
//    }
  if (method->m_Interpolator == ImageRegistrationMethod::LINEARINTERPOLATOR)
  {
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    registration->SetInterpolator(interpolator);
  }
  else if (method->m_Interpolator == ImageRegistrationMethod::NEARESTNEIGHBORINTERPOLATOR)
  {
    typename InterpolatorType2::Pointer interpolator = InterpolatorType2::New();
    registration->SetInterpolator(interpolator);
  }
  // registering command observer with the optimizer
  if (method->m_Observer.IsNotNull())
  {
    method->m_Observer->AddStepsToDo(20);
    optimizer->AddObserver(itk::AnyEvent(), method->m_Observer);
    registration->AddObserver(itk::AnyEvent(), method->m_Observer);
    transform->AddObserver(itk::AnyEvent(), method->m_Observer);
  }
  registration->Update();
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
