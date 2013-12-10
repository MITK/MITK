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
#include <mitkCommon.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <mitkTestingMacros.h>
#include <mitkPersistenceService.h>

// redefine get module context macro
#define PERSISTENCE_GET_MODULE_CONTEXT\
    us::ModuleContext* context = mitk::PersistenceService::GetModuleContext();


struct PersistenceTestClass
{
    PersistenceTestClass()
        : id(""), param1(1), param2(2), param3(false)
    {
    }
    std::string id;
    int param1;
    double param2;
    bool param3;

    PERSISTENCE_CREATE_SAVE3(id, param1, param2, param3)
    PERSISTENCE_CREATE_LOAD3(id, param1, param2, param3)
};


int mitkPersistenceTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PersistenceTest")

  PersistenceTestClass testClass;
  testClass.id = "testClass";
  testClass.param1 = 100;
  testClass.param2 = 200.56;
  testClass.param3 = true;
  MITK_TEST_CONDITION_REQUIRED( testClass.Save(), "testClass.Save()");

  PersistenceTestClass testClass2;
  testClass.id = "testClass";
  MITK_TEST_CONDITION_REQUIRED( testClass2.Load(), "testClass.Save()");


  MITK_TEST_CONDITION_REQUIRED( testClass.param1 == testClass2.param1, "testClass.param1 == testClass2.param1" );
  MITK_TEST_CONDITION_REQUIRED( testClass.param2 == testClass2.param2, "testClass.param2 == testClass2.param2" );
  MITK_TEST_CONDITION_REQUIRED( testClass.param3 == testClass2.param3, "testClass.param3 == testClass2.param3" );

  MITK_TEST_END()
}
