/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAutoCropImageFilter.h"

int mitkAutoCropImageFilterTest(int /*argc*/, char * /*argv*/ [])
{
  mitk::AutoCropImageFilter::Pointer filter;
  std::cout << "Testing mitk::AutoCropImageFilter::New(): ";
  filter = mitk::AutoCropImageFilter::New();
  if (filter.IsNull())
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  // std::cout << "Testing mitk::SurfaceToImageFilter::*TESTED_METHOD_DESCRIPTION: ";
  //// METHOD_TEST_CODE
  // if (filter.IsNull()) {
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  // else {
  // std::cout<<"[PASSED]"<<std::endl;
  //}

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
