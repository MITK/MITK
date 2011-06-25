/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <mitkImageRegistrationMethodAccessFunctor.txx>
#include <mitkInstantiateAccessFunctions.h>

#define InstantiateAccessFunction_RegistrationAccessItkImage(pixelType, dim) \
  template void mitk::ImageRegistrationMethodAccessFunctor::AccessItkImage(itk::Image<pixelType, dim>* itkImage1, mitk::ImageRegistrationMethod* method);

InstantiateAccessFunctionForFixedPixelType(RegistrationAccessItkImage, (@TYPE@))
