/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImageRegistrationMethod.h"


namespace mitk {

  ImageRegistrationMethod::ImageRegistrationMethod() : m_Observer(NULL), m_Interpolator(0), m_MovingMask(NULL), m_FixedMask(NULL)
  {
    m_OptimizerParameters = OptimizerParameters::New();
    m_TransformParameters = TransformParameters::New();
    m_MetricParameters = MetricParameters::New();
    m_ReferenceImage = Image::New();

    m_Preset = new mitk::RigidRegistrationPreset();

    bool succeed = m_Preset->LoadPreset();
    if(!succeed)
    {
       std::cout << "RigidRegistrationParameters.xml is empty or does not exist. There are no presets to select." << std::endl;
       return;
    }
    
    m_UseMask = false;

  }

  ImageRegistrationMethod::~ImageRegistrationMethod()
  {
 
  }

  void ImageRegistrationMethod::GenerateData()
  {
    if (this->GetInput())
    {
      AccessByItk(this->GetInput(), GenerateData2);
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
} // end namespace
