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

#include <mitkTestingMacros.h>
#include <mitkOclUtils.h>
#include <mitkOclResourceService.h>
#include <mitkException.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleInitialization.h>

using namespace mitk;

/**
  This function is testing the class mitk::OclResourceService.
  */
int mitkOclResourceServiceTest( int /*argc*/, char* /*argv*/[] )
{
  MITK_TEST_BEGIN("mitkOclResourceServiceTest");

  us::ServiceReference<OclResourceService> ref = us::GetModuleContext()->GetServiceReference<OclResourceService>();
  MITK_TEST_CONDITION_REQUIRED( ref != NULL, "Resource service available." );

  OclResourceService* resources = us::GetModuleContext()->GetService<OclResourceService>(ref);
  MITK_TEST_CONDITION_REQUIRED( resources != NULL, "Resource service available." );

  cl_context first = resources->GetContext();
  MITK_TEST_CONDITION_REQUIRED(first != NULL, "Got not-null OpenCL context.");

  OclResourceService* resources_2 = us::GetModuleContext()->GetService<OclResourceService>(ref);
  MITK_TEST_CONDITION_REQUIRED( resources == resources_2, "Same resource reference the second time." );

  cl_context second = resources_2->GetContext();
  MITK_TEST_CONDITION_REQUIRED( first == second, "Both return same context");

  // further tests requires for valid context
  if( first )
  {
    cl_image_format testFmt;
    testFmt.image_channel_data_type = CL_FLOAT;
    testFmt.image_channel_order = CL_RGBA;

    MITK_TEST_CONDITION( resources->GetIsFormatSupported( &testFmt ), "Checking if format CL_FLOAT / CL_RGBA supported." );
  }

  // create test program
  const std::string testProgramSource =
        "__kernel void testKernel( __global uchar* buffer ){ \
         const unsigned int globalPosX = get_global_id(0); \
         buffer[globalPosX] = buffer[globalPosX] + 1;}";

  cl_int err = 0;
  size_t progSize = testProgramSource.length();
  const char* progSource = testProgramSource.c_str();
  cl_program testProgram = clCreateProgramWithSource( first, 1, &progSource, &progSize, &err );

  MITK_TEST_CONDITION_REQUIRED( err == CL_SUCCESS, "Test program loaded succesfully.");

  err = clBuildProgram(testProgram, 0, NULL, NULL, NULL, NULL);
  MITK_TEST_CONDITION_REQUIRED( err == CL_SUCCESS, "Test program built succesfully.");

  resources->InsertProgram( testProgram, "test_program", true);
  MITK_TEST_CONDITION( resources->GetProgram("test_program") == testProgram, "Program correctly stored by ResourceService");

  // the manger throws exception when accessing non-existant programs
  MITK_TEST_FOR_EXCEPTION( mitk::Exception, resources->GetProgram("blah"); );

  // another test source, this one does not compile
  const std::string testProgramSource_notCompiling =
        "__kernel void testKernel( __global uchar* buffer ){ \
         const unsigned intt globalPosX = get_global_id(0); }";

  progSize = testProgramSource_notCompiling.length();
  const char* progSource2 = testProgramSource_notCompiling.c_str();
  cl_program notComp_testProgram = clCreateProgramWithSource( first, 1, &progSource2, &progSize, &err );

  // the error in the source code has no influence on loading the program
  MITK_TEST_CONDITION_REQUIRED( err == CL_SUCCESS, "Test program 2 loaded succesfully.");

  err = clBuildProgram(notComp_testProgram, 0, NULL, NULL, NULL, NULL);
  MITK_TEST_CONDITION_REQUIRED( err == CL_BUILD_PROGRAM_FAILURE, "Test program 2 failed to build.");
  std::cout << " --> The (expected) OpenCL Build Error occured : ";// << GetOclErrorString(err);

  resources->InsertProgram( notComp_testProgram, "test_program_failed", true);
  MITK_TEST_CONDITION( resources->GetProgram("test_program_failed") == notComp_testProgram, "Program correctly stored by ResourceService");

  // calling the InvalidateStorage() will result in removing the _failed test program inserted above
  resources->InvalidateStorage();

  // the second test program should no more exist in the storage, hence we await an exception
  MITK_TEST_FOR_EXCEPTION( mitk::Exception, resources->GetProgram("test_program_failed"); );

  MITK_TEST_END();
}
US_INITIALIZE_MODULE
