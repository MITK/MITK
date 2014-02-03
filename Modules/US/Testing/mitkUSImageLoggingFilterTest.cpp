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

#include "mitkUSImageLoggingFilter.h"
#include "mitkTestingMacros.h"

/**
* This function is testing methods of the class USDevice.
*/
int mitkUSImageLoggingFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSDeviceTest");

  //create object of class
  mitk::USImageLoggingFilter::Pointer testFilter = mitk::USImageLoggingFilter::New();
  MITK_TEST_CONDITION_REQUIRED(testFilter.IsNotNull(),"Testing instantiation");

  //create empty test images
  mitk::Image::Pointer testImage = mitk::Image::New();
  mitk::Image::Pointer testImage2 = mitk::Image::New();

  //set both as input for the filter
  testFilter->SetInput(testImage);
  MITK_TEST_CONDITION_REQUIRED(testFilter->GetNumberOfInputs()==1,"Testing SetInput(...) for first input.");
  testFilter->SetInput("secondImage",testImage2);
  MITK_TEST_CONDITION_REQUIRED(testFilter->GetNumberOfInputs()==2,"Testing SetInput(...) for second input.");

  testFilter->Update();
  MITK_TEST_OUTPUT(<<"Tested method Update()");


  MITK_TEST_END();
}
