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
#include "mitkCoreExtObjectFactory.h"
#include "mitkCoreObjectFactory.h"

int mitkCoreExtObjectFactoryTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("CoreExtObjectFactory")

  mitk::CoreObjectFactory::Pointer instance = mitk::CoreObjectFactory::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(instance.IsNotNull(),"Testing instantiation");
  MITK_TEST_CONDITION(strcmp(instance->GetNameOfClass(),"CoreObjectFactory") == 0,"Is this a CoreObjectFactory?");

  instance->RegisterExtraFactory(mitk::CoreExtObjectFactory::New());

  MITK_TEST_END()
}
