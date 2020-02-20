/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//testing headers
#include <mitkTestingConfig.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkNavigationToolStorageTestHelper.h>

//some general mitk headers
#include <mitkCommon.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>

//headers of IGT classes releated to the tested class
#include <mitkNavigationToolStorageSerializer.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkNavigationToolStorage.h>
#include <mitkIGTException.h>
#include <mitkIGTIOException.h>
#include <mitkIGTConfig.h>

//POCO headers for file handling
#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <Poco/File.h>


class mitkNavigationToolStorageSerializerAndDeserializerIntegrationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationToolStorageSerializerAndDeserializerIntegrationTestSuite);
  MITK_TEST(TestInstantiationSerializer);
  MITK_TEST(TestInstantiationDeserializer);
  MITK_TEST(TestWriteAndReadSimpleToolStorageWithToolLandmarks);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::NavigationToolStorageSerializer::Pointer m_Serializer;
  mitk::NavigationToolStorageDeserializer::Pointer m_Deserializer;
  std::string m_FileName1;

public:
  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp() override
  {
  try {
      m_FileName1 = mitk::IOUtil::CreateTemporaryFile();
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
  mitk::DataStorage::Pointer DataStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
  m_Deserializer = mitk::NavigationToolStorageDeserializer::New(DataStorage);
  }

  void tearDown() override
  {
  m_Serializer = nullptr;
  m_Deserializer = nullptr;
  try
    {
      std::remove(m_FileName1.c_str());
    }
    catch(...)
    {
      MITK_ERROR << "Warning: Error occured when deleting test file!";
    }
  }

  void TestInstantiationSerializer()
  {
  // let's create objects of our classes
  mitk::NavigationToolStorageSerializer::Pointer testSerializer = mitk::NavigationToolStorageSerializer::New();
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of NavigationToolStorageSerializer",testSerializer.IsNotNull());
  }

  void TestInstantiationDeserializer()
  {
  mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
  mitk::NavigationToolStorageDeserializer::Pointer testDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of NavigationToolStorageDeserializer",testDeserializer.IsNotNull());
  }

  void TestWriteAndReadSimpleToolStorageWithToolLandmarks()
  {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer storage = mitk::NavigationToolStorageTestHelper::CreateTestData_StorageWithOneTool();

    //test serialization
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Testing serialization of tool storage with tool registrations", m_Serializer->Serialize(m_FileName1,storage));

    //test deserialization of the same file
    mitk::NavigationToolStorage::Pointer readStorage = m_Deserializer->Deserialize(m_FileName1);
    CPPUNIT_ASSERT_MESSAGE("Testing deserialization of tool storage with tool registrations",readStorage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE(" ..Testing number of tools in storage",readStorage->GetToolCount()==1);

    mitk::PointSet::Pointer readRegLandmarks = readStorage->GetTool(0)->GetToolLandmarks();
    mitk::PointSet::Pointer readCalLandmarks = readStorage->GetTool(0)->GetToolControlPoints();

    CPPUNIT_ASSERT_MESSAGE("..Testing if tool registration landmarks have been stored and loaded correctly.",((readRegLandmarks->GetPoint(5)[0] == 4)&&(readRegLandmarks->GetPoint(5)[1] == 5)&&(readRegLandmarks->GetPoint(5)[2] == 6)));
    CPPUNIT_ASSERT_MESSAGE("..Testing if tool calibration landmarks have been stored and loaded correctly.",((readCalLandmarks->GetPoint(0)[0] == 1)&&(readCalLandmarks->GetPoint(0)[1] == 2)&&(readCalLandmarks->GetPoint(0)[2] == 3)));

    mitk::Point3D readToolTipPos = readStorage->GetTool(0)->GetToolTipPosition();
    mitk::Quaternion readToolTipRot = readStorage->GetTool(0)->GetToolAxisOrientation();

    CPPUNIT_ASSERT_MESSAGE("..Testing if tool tip position has been stored and loaded correctly.",
      ((float(readToolTipPos[0]) == float(1.3423))&&
      (float(readToolTipPos[1]) == float(2.323))&&
      (float(readToolTipPos[2]) == float(4.332))));

    CPPUNIT_ASSERT_MESSAGE("..Testing if tool tip orientation has been stored and loaded correctly.",
      ((float(readToolTipRot.x()) == float(0.1))&&
      (float(readToolTipRot.y()) == float(0.2))&&
      (float(readToolTipRot.z()) == float(0.3))&&
      (float(readToolTipRot.r()) == float(0.4))));
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageSerializerAndDeserializerIntegration)
