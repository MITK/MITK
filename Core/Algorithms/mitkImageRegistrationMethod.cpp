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

  ImageRegistrationMethod::ImageRegistrationMethod()
  {
    m_Observer = NULL;
    m_Interpolator = 0;
    m_ReferenceImage = Image::New();
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

} // end namespace
