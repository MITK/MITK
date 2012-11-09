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

#include "mitkImageRegistrationMethod.h"
#include "mitkImageRegistrationMethodAccessFunctor.h"

namespace mitk {

  ImageRegistrationMethod::ImageRegistrationMethod()
  : m_Interpolator(0)
  {
    m_ReferenceImage = Image::New();
    m_OptimizerScales.clear();
  }

  ImageRegistrationMethod::~ImageRegistrationMethod()
  {

  }

  void ImageRegistrationMethod::GenerateData()
  {
    if (this->GetInput())
    {
      ImageRegistrationMethodAccessFunctor()(this->GetInput(), this);
    }
  }

  void ImageRegistrationMethod::SetObserver(RigidRegistrationObserver::Pointer observer)
  {
    m_Observer = observer;
  }

  void ImageRegistrationMethod::SetInterpolator(int interpolator)
  {
    m_Interpolator = interpolator;
  }

  void ImageRegistrationMethod::SetReferenceImage(Image::Pointer fixedImage)
  {
    m_ReferenceImage = fixedImage;
    SetNthInput(1, m_ReferenceImage);
    Modified();
  }

  void ImageRegistrationMethod::SetMovingMask(Image::Pointer movingMask)
  {
    m_MovingMask = movingMask;
    SetNthInput(3, m_MovingMask);
    Modified();
  }

  void ImageRegistrationMethod::SetFixedMask(Image::Pointer FixedMask)
  {
    m_FixedMask = FixedMask;
    SetNthInput(4, m_FixedMask);
    Modified();
  }

  void ImageRegistrationMethod::SetTransform(itk::Object::Pointer transform)
  {
    m_Transform = transform;
  }

  void ImageRegistrationMethod::SetMetric(itk::Object::Pointer metric)
  {
    m_Metric = metric;
  }

  void ImageRegistrationMethod::SetOptimizer(itk::Object::Pointer optimizer)
  {
    m_Optimizer = optimizer;
  }

  void ImageRegistrationMethod::SetOptimizerScales(itk::Array<double> scales)
  {
    m_OptimizerScales = scales;
  }

} // end namespace
