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
#include "mitkTypes.h"

#include <iostream>

using namespace mitk;

Point3D a, b, c;

static void Setup(void)
{

}

static void TestAddition(void)
{
  Setup();

  // c = a + b;

  MITK_TEST_CONDITION_REQUIRED(false, "this test is passed if there is no compile error in this method when not commented out :)")
}

int mitkTypeOperationTest(int /*argc*/ , char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("TypeOperationTest")

  TestAddition();

  MITK_TEST_END()

}
