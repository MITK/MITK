/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>
#include <mitkIGTException.h>
#include <mitkNavigationToolStorageTestHelper.h>

//headers of IGT classes releated to the tested class
#include <mitkNavigationToolStorageSerializer.h>

#include <Poco/Path.h>

class mitkNavigationToolStorageSerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationToolStorageSerializerTestSuite);
  MITK_TEST(TestInstantiationSerializer);
  MITK_TEST(TestWriteSimpleToolStorage);
  MITK_TEST(TestWriteComplexToolStorage);
  MITK_TEST(TestWriteStorageToInvalidFile);
  MITK_TEST(TestWriteEmptyToolStorage);
  MITK_TEST(TestSerializerForExceptions);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  std::string m_FileName1;
  mitk::NavigationToolStorageSerializer::Pointer m_Serializer;

public:

  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp() override
  {
    try {
      m_FileName1 = mitk::IOUtil::CreateTemporaryFile("NavigationToolStorageSerializerTestTmp_XXXXXX.IGTToolStorage",mitk::IOUtil::GetProgramPath());
      std::ofstream file;
      file.open(m_FileName1.c_str());
      if (!file.good()) {MITK_ERROR <<"Could not create a valid file during setUp() method.";}
      file.close();
    }
    catch (std::exception& e) {
      MITK_ERROR << "File access Exception: " << e.what();
      MITK_ERROR <<"Could not create filename during setUp() method.";
    }
    m_Serializer = mitk::NavigationToolStorageSerializer::New();
  }

  void tearDown() override
  {
    m_Serializer = nullptr;
    try
    {
      std::remove(m_FileName1.c_str());
    }
    catch(...)
    {
      MITK_ERROR << "Warning: Error occured while deleting test file!";
    }
  }

  void TestInstantiationSerializer()
  {
  // let's create objects of our classes
  mitk::NavigationToolStorageSerializer::Pointer testSerializer = mitk::NavigationToolStorageSerializer::New();
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of NavigationToolStorageSerializer",testSerializer.IsNotNull());
  }

  void TestWriteSimpleToolStorage()
  {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorageTestHelper::CreateTestData_SimpleStorage();

    //test serialization
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Testing serialization of simple tool storage", m_Serializer->Serialize(m_FileName1, myStorage));
  }

  void TestWriteComplexToolStorage()
  {
    //create navigation tool storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorageTestHelper::CreateTestData_ComplexStorage(GetTestDataFilePath("ClaronTool"),GetTestDataFilePath("IGT-Data/ClaronTool.stl"),GetTestDataFilePath("IGT-Data/EMTool.stl"));

    //test serialization
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Testing serialization of complex tool storage", m_Serializer->Serialize(m_FileName1, myStorage));
  }

  void TestWriteStorageToInvalidFile()
  {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorageTestHelper::CreateTestData_SimpleStorage();

    //create invalid filename
  #ifdef WIN32
    std::string filename = "C:\342INVALIDFILE<>.storage"; //invalid filename for windows
  #else
    std::string filename = "/dsfdsf:$�$342INVALIDFILE.storage"; //invalid filename for linux
  #endif

    //test serialization (should throw exception)
    CPPUNIT_ASSERT_THROW_MESSAGE("Test serialization with simple storage and invalid filename, an exception is expected.",m_Serializer->Serialize(filename,myStorage),mitk::IGTException);
  }

  void TestWriteEmptyToolStorage()
  {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //test serialization
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Testing serialization of simple tool storage", m_Serializer->Serialize(m_FileName1, myStorage));
  }

  void TestSerializerForExceptions()
  {
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //create an invalid filename
    std::string filename = std::string( MITK_TEST_OUTPUT_DIR )+Poco::Path::separator()+"";

    //now try to serialize an check if an exception is thrown
    CPPUNIT_ASSERT_THROW_MESSAGE("Test serialization with empty storage and invalid filename, an exception is expected.",m_Serializer->Serialize(filename,myStorage),mitk::IGTException);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageSerializer)
