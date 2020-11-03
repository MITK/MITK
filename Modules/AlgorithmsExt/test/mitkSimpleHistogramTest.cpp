/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSimpleHistogram.h>
#include <mitkSurface.h>
#include <mitkTestingMacros.h>

int mitkSimpleHistogramTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("mitkSimpleHistogram");

  auto myTestSimpleImageHistogram = new mitk::SimpleImageHistogram();

  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram != nullptr, "Testing instanciation.");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetMax() == 1, "Testing GetMax().");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetMin() == 0, "Testing GetMin().");
  MITK_TEST_CONDITION_REQUIRED(myTestSimpleImageHistogram->GetRelativeBin(1.0, 5.0) == 0, "Testing GetRelativeBin().");
  bool success = true;
  try
  {
    myTestSimpleImageHistogram->ComputeFromBaseData(nullptr);
    myTestSimpleImageHistogram->ComputeFromBaseData(mitk::Image::New());   // an empty image
    myTestSimpleImageHistogram->ComputeFromBaseData(mitk::Surface::New()); // an invalid value
  }
  catch (...)
  {
    success = false;
  }
  MITK_TEST_CONDITION_REQUIRED(success, "Testing ComputeFromBaseData() with invalid input values.");
  MITK_TEST_CONDITION_REQUIRED(!myTestSimpleImageHistogram->GetValid(),
                               "Testing if histogram is invalid after invalid input.");

  MITK_TEST_END();
}
