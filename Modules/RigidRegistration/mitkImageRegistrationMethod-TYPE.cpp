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

#include <mitkImageRegistrationMethodAccessFunctor.txx>
#include <mitkInstantiateAccessFunctions.h>

#define InstantiateAccessFunction_RegistrationAccessItkImage(pixelType, dim) \
  template void mitk::ImageRegistrationMethodAccessFunctor::AccessItkImage(const itk::Image<pixelType, dim>* itkImage1, mitk::ImageRegistrationMethod* method);

InstantiateAccessFunctionForFixedPixelType(RegistrationAccessItkImage, (@TYPE@))
