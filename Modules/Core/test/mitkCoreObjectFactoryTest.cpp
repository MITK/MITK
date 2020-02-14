/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCoreObjectFactory.h"
#include "mitkTestingMacros.h"

int mitkCoreObjectFactoryTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("CoreObjectFactory")

  mitk::CoreObjectFactory::Pointer instance = mitk::CoreObjectFactory::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(instance.IsNotNull(), "Testing instantiation");
  MITK_TEST_CONDITION(strcmp(instance->GetNameOfClass(), "CoreObjectFactory") == 0, "Is this a CoreObjectFactory?");

  MITK_TEST_END()
}
