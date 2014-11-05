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
#include <mitkTestFixture.h>
#include <mitkIPersistenceService.h>
#include <mitkPersistenceService.h>
#include <mitkSceneIO.h>
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>

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

class mitkPersistenceTestSuite : public mitk::TestFixture
{
   CPPUNIT_TEST_SUITE(mitkPersistenceTestSuite);
   MITK_TEST(PersistenceTest);
   CPPUNIT_TEST_SUITE_END();

private:
  // private test members that are initialized by setUp()
  std::string testClassId;
  int param1;
  double param2;
  bool param3;

public:

  void setUp()
  {
    testClassId = "testClass";
    param1 = 100;
    param2 = 201.56;
    param3 = true;
  }

  void PersistenceTest()
  {
    // dummy load of SceneIO, otherwise PersistenceService won't be available
    mitk::PersistenceService::LoadModule();

    PERSISTENCE_GET_SERVICE_MACRO
      CPPUNIT_ASSERT_MESSAGE("Testing availability of the PersistenceService.", persistenceService != NULL);

    // Initialize testable parameter values
    std::string defaultPersistenceFile = persistenceService->GetDefaultPersistenceFile();
    PersistenceTestClass autoLoadTestClass;
    autoLoadTestClass.id = testClassId;
    if( itksys::SystemTools::FileExists(defaultPersistenceFile.c_str(), true) && persistenceService->GetAutoLoadAndSave() )
    {
      /// Test auto load/save of the PersistenceService.
      itksys::SystemTools::RemoveFile(defaultPersistenceFile.c_str());
      autoLoadTestClass.FromPropertyList();
      testParams( autoLoadTestClass, "autoLoadTestClass" );
    }

    std::string testTempFile = mitk::IOUtil::CreateTemporaryFile("XXXXXX.mitk");
    std::string testXmlTempFile = mitk::IOUtil::CreateTemporaryFile("PersistenceTestFileXXXXXX.xml");

    MITK_INFO << "Testing standard write to scene file/xml file.";
    PersistenceTestClass testClass;
    testClass.id = testClassId;
    testClass.param1 = param1;
    testClass.param2 = param2;
    testClass.param3 = param3;
    CPPUNIT_ASSERT_MESSAGE( "Testing to save a scene file", testClass.Save(testTempFile));
    CPPUNIT_ASSERT_MESSAGE( "testing to save an xml file", testClass.Save(testXmlTempFile));

    CPPUNIT_ASSERT_MESSAGE( "Testing read from scene file: persistenceService->RemovePropertyList(testClassId)", persistenceService->RemovePropertyList(testClassId));
    PersistenceTestClass testClass2;
    testClass2.id = testClassId;
    CPPUNIT_ASSERT_MESSAGE( "Testing read from scene file: testClass2.Load(testTempFile.path())", testClass2.Load(testTempFile));

    testParams( testClass2, "testClass2" );

    CPPUNIT_ASSERT_MESSAGE( "Testing read from xml file: persistenceService->RemovePropertyList(testClassId)", persistenceService->RemovePropertyList(testClassId));
    PersistenceTestClass testClass3;
    testClass3.id = testClassId;
    CPPUNIT_ASSERT_MESSAGE( "Testing read from xml file: testClass3.Load(testXmlTempFile.path())", testClass3.Load(testXmlTempFile));

    testParams( testClass3, "testClass3" );

    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with scene load/write: persistenceService->RemovePropertyList(testClassId)", persistenceService->RemovePropertyList(testClassId));
    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with scene load/write: persistenceService->Save(testTempFile.path())",persistenceService->Save(testTempFile, true));
    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with scene load/write: persistenceService->Load(testTempFile.path())", persistenceService->Load(testTempFile));

    PersistenceTestClass testClass4;
    testClass4.id = testClassId;
    testClass4.FromPropertyList();
    testParams( testClass4, "testClass4" );

    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with xml load/write: persistenceService->RemovePropertyList(testClassId)", persistenceService->RemovePropertyList(testClassId));
    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with xml load/write: persistenceService->Save(testXmlTempFile.path())", persistenceService->Save(testXmlTempFile, true));
    CPPUNIT_ASSERT_MESSAGE( "Testing appendChanges functionality with xml load/write: persistenceService->Load(testXmlTempFile.path())", persistenceService->Load(testXmlTempFile));

    PersistenceTestClass testClass5;
    testClass5.id = testClassId;
    testClass5.FromPropertyList();
    testParams( testClass5, "testClass5" );

    // Test Observer Functionality
    TestPropertyListReplacedObserver testObserver;
    testObserver.m_Id = testClassId;
    persistenceService->AddPropertyListReplacedObserver( &testObserver );
    persistenceService->Load(testTempFile);
    CPPUNIT_ASSERT_MESSAGE( "Testing observer functionality: testObserver.counter == 2, testObserver.counter is " + testObserver.counter, testObserver.counter == 2 );

    autoLoadTestClass.param1 = param1;
    autoLoadTestClass.param2 = param2;
    autoLoadTestClass.param3 = param3;
    autoLoadTestClass.ToPropertyList();
  }

   /**
  * Helper Method that compares the returned class to its base values
  */
  void testParams( const PersistenceTestClass& testClass, const std::string& testClassName )
  {
    CPPUNIT_ASSERT_MESSAGE( "Parameter of TestClass not equal to reference value: testClass.id", testClass.id == testClassId );
    CPPUNIT_ASSERT_MESSAGE( "Parameter of TestClass not equal to reference value: testClass.param1" , testClass.param1 == param1);
    CPPUNIT_ASSERT_MESSAGE( "Parameter of TestClass not equal to reference value: testClass.param2" , testClass.param2 == param2);
    CPPUNIT_ASSERT_MESSAGE( "Parameter of TestClass not equal to reference value: testClass.param3" , testClass.param3 == param3);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkPersistence)