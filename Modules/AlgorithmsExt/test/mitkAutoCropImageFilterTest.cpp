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


#include "mitkAutoCropImageFilter.h"


int mitkAutoCropImageFilterTest(int /*argc*/, char* /*argv*/[])
{
  mitk::AutoCropImageFilter::Pointer filter;
  std::cout << "Testing mitk::AutoCropImageFilter::New(): ";
  filter = mitk::AutoCropImageFilter::New();
  if (filter.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  }

  //std::cout << "Testing mitk::SurfaceToImageFilter::*TESTED_METHOD_DESCRIPTION: ";
  //// METHOD_TEST_CODE
  //if (filter.IsNull()) {
  //  std::cout<<"[FAILED]"<<std::endl;
  //  return EXIT_FAILURE;
  //}
  //else {
  //std::cout<<"[PASSED]"<<std::endl;
  //}

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
