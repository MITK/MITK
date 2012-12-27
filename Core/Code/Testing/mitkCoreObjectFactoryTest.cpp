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
#include "mitkCoreObjectFactory.h"


int mitkCoreObjectFactoryTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("CoreObjectFactory")

  mitk::CoreObjectFactory::Pointer instance = mitk::CoreObjectFactory::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(instance.IsNotNull(),"Testing instantiation");
  MITK_TEST_CONDITION(strcmp(instance->GetNameOfClass(),"CoreObjectFactory") == 0,"Is this a CoreObjectFactory?");




  MITK_TEST_END()
}
