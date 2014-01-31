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
//#include <mitkSceneIO.h>
#include <Poco/File.h>

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

struct TestPropertyListReplacedObserver: public mitk::PropertyListReplacedObserver
{
    TestPropertyListReplacedObserver(): counter(0) {}
    virtual void BeforePropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList )
    {
        if( id == m_Id )
            counter++;
    }

    virtual void AfterPropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList )
    {
        if( id == m_Id )
            counter++;
    }

    int counter;
    std::string m_Id;
};

std::string testClassId = "testClass";
int param1 = 100;
double param2 = 201.56;
bool param3 = true;

void testParams( const PersistenceTestClass& testClass, const std::string& testClassName )
{
    MITK_INFO << "Testing parameters of " << testClassName;
    MITK_TEST_CONDITION( testClass.id == testClassId, "testClass.id (" << testClass.id << ") != testClassId (" << testClassId << ")" );
    MITK_TEST_CONDITION( testClass.param1 == param1, "testClass.param1 (" << testClass.param1 << ") != param1 (" << param1 << ")" );
    MITK_TEST_CONDITION( testClass.param2 == param2, "testClass.param2 (" << testClass.param2 << ") != param2 (" << param2 << ")" );
    MITK_TEST_CONDITION( testClass.param3 == param3, "testClass.param3 (" << testClass.param3 << ") != param3 (" << param3 << ")" );
}

int mitkPersistenceTest(int /*argc*/, char* /*argv*/[])
{
    MITK_TEST_BEGIN("PersistenceTest")
    // dummy load of SceneIO, otherwise PersistenceService won't be available
    // mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    MITK_INFO << "Testing availability of the PersistenceService.";
    PERSISTENCE_GET_SERVICE_MACRO
    MITK_TEST_CONDITION_REQUIRED(persistenceService, "IPersistenceService available")

    MITK_INFO << "Initialize testable parameter values.";

    Poco::File defaultPersistenceFile(persistenceService->GetDefaultPersistenceFile());
    if( defaultPersistenceFile.exists() && persistenceService->GetAutoLoadAndSave() )
    {
        MITK_INFO << "Testing auto load/save of the PersistenceService.";

        PersistenceTestClass autoLoadTestClass;
        autoLoadTestClass.id = testClassId;
        autoLoadTestClass.FromPropertyList();

        testParams( autoLoadTestClass, "autoLoadTestClass" );
    }

    MITK_INFO << "Removing left-over test files.";
    Poco::File testTempFile("PersistenceTestFile.mitk");
    if( testTempFile.exists() )
        testTempFile.remove(false);

    Poco::File testXmlTempFile("PersistenceTestFile.xml");
    if( testXmlTempFile.exists() )
        testXmlTempFile.remove(false);

    MITK_INFO << "Testing standard write to scene file/xml file.";
    PersistenceTestClass testClass;
    testClass.id = testClassId;
    testClass.param1 = param1;
    testClass.param2 = param2;
    testClass.param3 = param3;
    MITK_TEST_CONDITION_REQUIRED( testClass.Save(testTempFile.path()), "testClass.Save(testTempFile.path())");
    MITK_TEST_CONDITION_REQUIRED( testClass.Save(testXmlTempFile.path()), "testClass.Save(testTempFile.path())");

    MITK_INFO << "Testing read from scene file.";
    MITK_TEST_CONDITION_REQUIRED( persistenceService->RemovePropertyList(testClassId), "persistenceService->RemovePropertyList(testClassId)");
    PersistenceTestClass testClass2;
    testClass2.id = testClassId;
    MITK_TEST_CONDITION_REQUIRED( testClass2.Load(testTempFile.path()), "testClass2.Load(testTempFile.path())");

    testParams( testClass2, "testClass2" );

    MITK_INFO << "Testing read from xml file.";
    MITK_TEST_CONDITION_REQUIRED( persistenceService->RemovePropertyList(testClassId), "persistenceService->RemovePropertyList(testClassId)");
    PersistenceTestClass testClass3;
    testClass3.id = testClassId;
    MITK_TEST_CONDITION_REQUIRED( testClass3.Load(testXmlTempFile.path()), "testClass3.Load(testXmlTempFile.path())");

    testParams( testClass3, "testClass3" );

    MITK_INFO << "Testing appendChanges functionality with scene load/write.";
    MITK_TEST_CONDITION_REQUIRED( persistenceService->RemovePropertyList(testClassId), "persistenceService->RemovePropertyList(testClassId)");
    MITK_TEST_CONDITION_REQUIRED( persistenceService->Save(testTempFile.path(), true), "persistenceService->Save(testTempFile.path())");
    MITK_TEST_CONDITION_REQUIRED( persistenceService->Load(testTempFile.path()), "persistenceService->Load(testTempFile.path())");

    PersistenceTestClass testClass4;
    testClass4.id = testClassId;
    testClass4.FromPropertyList();
    testParams( testClass4, "testClass4" );

    MITK_INFO << "Testing appendChanges functionality with xml load/write.";
    MITK_TEST_CONDITION_REQUIRED( persistenceService->RemovePropertyList(testClassId), "persistenceService->RemovePropertyList(testClassId)");
    MITK_TEST_CONDITION_REQUIRED( persistenceService->Save(testXmlTempFile.path(), true), "persistenceService->Save(testXmlTempFile.path())");
    MITK_TEST_CONDITION_REQUIRED( persistenceService->Load(testXmlTempFile.path()), "persistenceService->Load(testXmlTempFile.path())");

    PersistenceTestClass testClass5;
    testClass5.id = testClassId;
    testClass5.FromPropertyList();
    testParams( testClass5, "testClass5" );

    MITK_INFO << "Testing observer functionality.";
    TestPropertyListReplacedObserver testObserver;
    testObserver.m_Id = testClassId;
    persistenceService->AddPropertyListReplacedObserver( &testObserver );
    persistenceService->Load(testTempFile.path());
    MITK_TEST_CONDITION( testObserver.counter == 2, "testObserver.counter == 2, testObserver.counter is " << testObserver.counter );

    MITK_INFO << "Cleaning test files.";
    if( testXmlTempFile.exists() )
        testXmlTempFile.remove(false);
    if( testTempFile.exists() )
        testTempFile.remove(false);

    MITK_TEST_END()
}
