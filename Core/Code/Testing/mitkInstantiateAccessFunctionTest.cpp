/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkTestingMacros.h"

#include <mitkInstantiateAccessFunctions.h>

// This is a "compile test", meaning that this file calls all InstantiateAccessFunction macros
// and must successfully compile.
class InstantiateAccessFunctionTest
{
public:

  template<typename TPixel, unsigned int VDimension>
  void AccessItkImage(itk::Image<TPixel, VDimension>*, int)
  { }

};

#define InstantiateAccessFunction_AccessItkImage(Pixel, Dim) \
  template void InstantiateAccessFunctionTest::AccessItkImage(itk::Image<Pixel, Dim>*, int);

InstantiateAccessFunctionForFixedPixelType(AccessItkImage, (float)(double))
InstantiateAccessFunctionForIntegralPixelTypes(AccessItkImage)

int mitkInstantiateAccessFunctionTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("InstantiateAccessFunction")

  MITK_TEST_OUTPUT(<< "Successfully compiled")

  MITK_TEST_END()

}
