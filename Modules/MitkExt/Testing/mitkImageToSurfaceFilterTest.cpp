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

#include "mitkTestingMacros.h"
#include "mitkImageToSurfaceFilter.h"

int mitkImageToSurfaceFilterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ImageToSurfaceFilterTest");
  mitk::ImageToSurfaceFilter::Pointer testObject = mitk::ImageToSurfaceFilter::New();
  MITK_TEST_CONDITION_REQUIRED(testObject.IsNotNull(), "Testing instantiation of test object");


  // thats it folks
  MITK_TEST_END();
}
