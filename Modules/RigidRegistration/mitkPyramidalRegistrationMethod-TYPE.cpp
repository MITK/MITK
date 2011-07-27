/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkPyramidalRegistrationMethodAccessFunctor.txx>
#include <mitkInstantiateAccessFunctions.h>

#define InstantiateAccessFunction_PyramidalAccessItkImage(pixelType, dim) \
  template void mitk::PyramidalRegistrationMethodAccessFunctor::AccessItkImage(itk::Image<pixelType, dim>* itkImage1, mitk::PyramidalRegistrationMethod* method);

InstantiateAccessFunctionForFixedPixelType(PyramidalAccessItkImage, (@TYPE@))

