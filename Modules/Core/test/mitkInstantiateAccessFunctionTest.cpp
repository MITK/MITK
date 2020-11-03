/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"

#include <mitkInstantiateAccessFunctions.h>

// This is a "compile test", meaning that this file calls all InstantiateAccessFunction macros
// and must successfully compile.
class InstantiateAccessFunctionTest
{
public:
  template <typename TPixel, unsigned int VDimension>
  void AccessItkImage(itk::Image<TPixel, VDimension> *, int)
  {
  }
};

#define InstantiateAccessFunction_AccessItkImage(Pixel, Dim)                                                           \
  template void InstantiateAccessFunctionTest::AccessItkImage(itk::Image<Pixel, Dim> *, int);

InstantiateAccessFunctionForFixedPixelType(AccessItkImage, (float)(double))
  InstantiateAccessFunctionForIntegralPixelTypes(AccessItkImage)

    int mitkInstantiateAccessFunctionTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("InstantiateAccessFunction")

  MITK_TEST_OUTPUT(<< "Successfully compiled")

  MITK_TEST_END()
}
