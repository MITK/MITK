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

//headers of IGT classes releated to the tested class
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIGTException.h>

class mitkNavigationToolStorageDeserializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationToolStorageDeserializerTestSuite);
  MITK_TEST(TestInstantiationDeserializer);
  MITK_TEST(TestReadSimpleToolStorage);
  MITK_TEST(TestReadComplexToolStorage);
  MITK_TEST(TestReadNotExistingStorage);
  MITK_TEST(TestReadStorageWithUnknownFiletype);
  MITK_TEST(TestReadZipFileWithNoToolstorage);
  MITK_TEST(TestDeserializerForExceptions);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::NavigationToolStorageDeserializer::Pointer m_Deserializer;
  mitk::DataStorage::Pointer m_DataStorage;

public:
  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp() override
  {
  m_DataStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
  m_Deserializer = mitk::NavigationToolStorageDeserializer::New(m_DataStorage);
  }

  void tearDown() override
  {
  m_DataStorage = nullptr;
  m_Deserializer = nullptr;
  }

  void TestInstantiationDeserializer()
  {
  mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
  mitk::NavigationToolStorageDeserializer::Pointer testDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of NavigationToolStorageDeserializer",testDeserializer.IsNotNull());
  }

  void TestReadSimpleToolStorage()
  {
    std::string testDataPath = GetTestDataFilePath("IGT-Data/SimpleIGTStorage.storage");
    mitk::NavigationToolStorage::Pointer readStorage = m_Deserializer->Deserialize(testDataPath);
    CPPUNIT_ASSERT_MESSAGE("Testing deserialization of simple tool storage",readStorage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE(" ..Testing number of tools in storage",readStorage->GetToolCount()==3);
    //TODO: why is the order of tools changed is save/load process??
    bool foundtool1 = false;
    bool foundtool2 = false;
    bool foundtool3 = false;
    for(int i=0; i<3; i++)
    {
      if ((readStorage->GetTool(i)->GetIdentifier()=="001")) foundtool1 = true;
      else if ((readStorage->GetTool(i)->GetIdentifier()=="002")) foundtool2 = true;
      else if ((readStorage->GetTool(i)->GetIdentifier()=="003")) foundtool3 = true;
    }
    CPPUNIT_ASSERT_MESSAGE(" ..Testing if tool 1 was loaded successfully",foundtool1);
    CPPUNIT_ASSERT_MESSAGE(" ..Testing if tool 2 was loaded successfully",foundtool2);
    CPPUNIT_ASSERT_MESSAGE(" ..Testing if tool 3 was loaded successfully",foundtool3);
  }

  void TestReadComplexToolStorage()
  {
    std::string testDataPath = GetTestDataFilePath("IGT-Data/ComplexIGTStorage.storage");
    mitk::NavigationToolStorage::Pointer readStorage = m_Deserializer->Deserialize(testDataPath);
    CPPUNIT_ASSERT_MESSAGE("Testing deserialization of complex tool storage",readStorage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE(" ..Testing number of tools in storage",readStorage->GetToolCount()==2);
  }

  void TestReadNotExistingStorage()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Testing if exception is thrown if a non existing storage is given for deserialization.",
                                 m_Deserializer->Deserialize("noStorage.tfl"),
                                 mitk::IGTException);
  }

  void TestReadStorageWithUnknownFiletype()
  {
    std::string testDataPath = GetTestDataFilePath("IGT-Data/ClaronTool.stl");

    CPPUNIT_ASSERT_THROW_MESSAGE("Testing if exception is thrown if a wrong file type is given for deserialization.",
                                 m_Deserializer->Deserialize(testDataPath),
                                 mitk::IGTException);
  }

  void TestReadZipFileWithNoToolstorage()
  {
    std::string testDataPath = GetTestDataFilePath("IGT-Data/Empty.zip");

    CPPUNIT_ASSERT_THROW_MESSAGE("Testing if exception is thrown if a empty zip file is given for deserialization.",
                                 m_Deserializer->Deserialize(testDataPath),
                                 mitk::IGTException);
  }

  //new tests for exception throwing of NavigationToolStorageDeserializer
  void TestDeserializerForExceptions()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Testing deserializer with invalid filename.",
                                 m_Deserializer->Deserialize("InvalidName"),
                                 mitk::IGTException);

    std::string testDataPath = GetTestDataFilePath("IGT-Data/Empty2.zip");
    CPPUNIT_ASSERT_THROW_MESSAGE("Testing deserializer with invalid filename.",
                                 m_Deserializer->Deserialize(testDataPath),
                                 mitk::IGTException);
  }


};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageDeserializer)
