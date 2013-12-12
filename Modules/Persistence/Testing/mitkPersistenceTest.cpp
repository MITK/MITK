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
#include <mitkIPersistenceService.h>
#include <mitkSceneIO.h>
#include "Poco\File.h"

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

    PERSISTENCE_CREATE3(PersistenceTestClass, id, param1, param2, param3)
};


struct PersistenceTestClass2
{
};

struct TestPropertyListReplacedObserver: public mitk::PropertyListReplacedObserver
{
    TestPropertyListReplacedObserver(): counter(0) {}
    virtual void BeforePropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList )
    {
        counter++;
    }

    virtual void AfterPropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList )
    {
        counter++;
    }

    int counter;
};

int mitkPersistenceTest(int /*argc*/, char* /*argv*/[])
{
    // dummy load of SceneIO, otherwise PersistenceService won't be available
    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    Poco::File testTempFile("Test.mitk");
    if( testTempFile.exists() )
        testTempFile.remove(false);

    MITK_TEST_BEGIN("PersistenceTest")
    PersistenceTestClass testClass;
    testClass.id = "testClass";
    testClass.param1 = 100;
    testClass.param2 = 200.56;
    testClass.param3 = true;
    MITK_TEST_CONDITION_REQUIRED( testClass.Save(), "testClass.Save()");
    MITK_TEST_CONDITION_REQUIRED( testClass.Save(testTempFile.path()), "testClass.Save(testTempFile.path())");

    PersistenceTestClass testClass2;
    testClass2.id = "testClass";
    MITK_TEST_CONDITION_REQUIRED( testClass2.Load(), "testClass2.Load()");

    MITK_TEST_CONDITION_REQUIRED( testClass.param1 == testClass2.param1, "testClass.param1 == testClass2.param1" );
    MITK_TEST_CONDITION_REQUIRED( testClass.param2 == testClass2.param2, "testClass.param2 == testClass2.param2" );
    MITK_TEST_CONDITION_REQUIRED( testClass.param3 == testClass2.param3, "testClass.param3 == testClass2.param3" );

    PersistenceTestClass testClass3;
    testClass3.id = "testClass";
    MITK_TEST_CONDITION_REQUIRED( testClass3.Load(testTempFile.path()), "testClass3.Load(testTempFile.path())");

    MITK_TEST_CONDITION_REQUIRED( testClass.param1 == testClass3.param1, "testClass.param1 == testClass3.param1" );
    MITK_TEST_CONDITION_REQUIRED( testClass.param2 == testClass3.param2, "testClass.param2 == testClass3.param2" );
    MITK_TEST_CONDITION_REQUIRED( testClass.param3 == testClass3.param3, "testClass.param3 == testClass3.param3" );

    TestPropertyListReplacedObserver testObserver;

    PERSISTENCE_GET_SERVICE_MACRO

    MITK_TEST_CONDITION_REQUIRED(persistenceService, "IPersistenceService available")

    persistenceService->AddPropertyListReplacedObserver( &testObserver );
    persistenceService->Load();

    MITK_TEST_CONDITION_REQUIRED( testObserver.counter == 4, "testObserver.counter == 4" );

    if( testTempFile.exists() )
      testTempFile.remove(false);

    MITK_TEST_END()
}
