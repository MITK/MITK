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

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>
#include <mitkIGTException.h>

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

  //help methods for test tool storages
  mitk::NavigationToolStorage::Pointer CreateTestData_SimpleStorage()
    {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    myStorage->AddTool(myTool1);
    //second tool
    mitk::NavigationTool::Pointer myTool2 = mitk::NavigationTool::New();
    myTool2->SetIdentifier("002");
    myStorage->AddTool(myTool2);
    //third tool
    mitk::NavigationTool::Pointer myTool3 = mitk::NavigationTool::New();
    myTool3->SetIdentifier("003");
    myStorage->AddTool(myTool3);

    return myStorage;
    }
  mitk::NavigationToolStorage::Pointer CreateTestData_ComplexStorage()
    {
    //create first tool
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetCalibrationFile(GetTestDataFilePath("ClaronTool"));
    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("ClaronTool");
    myNode->SetData(mitk::IOUtil::LoadSurface(GetTestDataFilePath("IGT-Data/ClaronTool.stl"))); //load an stl File
    myNavigationTool->SetDataNode(myNode);
    myNavigationTool->SetIdentifier("ClaronTool#1");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::ClaronMicron);
    myNavigationTool->SetType(mitk::NavigationTool::Fiducial);

    //create second tool
    mitk::NavigationTool::Pointer myNavigationTool2 = mitk::NavigationTool::New();
    mitk::Surface::Pointer testSurface2;

    mitk::DataNode::Pointer myNode2 = mitk::DataNode::New();
    myNode2->SetName("AuroraTool");

    //load an stl File
    testSurface2 = mitk::IOUtil::LoadSurface(GetTestDataFilePath("IGT-Data/EMTool.stl"));
    myNode2->SetData(testSurface2);

    myNavigationTool2->SetDataNode(myNode2);
    myNavigationTool2->SetIdentifier("AuroraTool#1");
    myNavigationTool2->SetSerialNumber("0816");
    myNavigationTool2->SetTrackingDeviceType(mitk::NDIAurora);
    myNavigationTool2->SetType(mitk::NavigationTool::Instrument);

    //create navigation tool storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    myStorage->AddTool(myNavigationTool);
    myStorage->AddTool(myNavigationTool2);

    return myStorage;
    }

public:

  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp()
  {
    try {
      m_FileName1 = mitk::IOUtil::CreateTemporaryFile();
    }
    catch (std::exception& e) {
      MITK_ERROR << "File access Exception: " << e.what();
      MITK_ERROR <<"Could not create filename during setUp() method.";
    }
    m_Serializer = mitk::NavigationToolStorageSerializer::New();
  }

  void tearDown()
  {
    m_Serializer = NULL;
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

  void TestWriteSimpleToolStorage()
  {
  //create Tool Storage
  mitk::NavigationToolStorage::Pointer myStorage = this->CreateTestData_SimpleStorage();

  //test serialization
  bool success = m_Serializer->Serialize(m_FileName1,myStorage);
  CPPUNIT_ASSERT_MESSAGE("Testing serialization of simple tool storage",success);
  }

  void TestWriteComplexToolStorage()
  {
    //create navigation tool storage
    mitk::NavigationToolStorage::Pointer myStorage = this->CreateTestData_ComplexStorage();

    //test serialization
    bool success = m_Serializer->Serialize(m_FileName1,myStorage);
    CPPUNIT_ASSERT_MESSAGE("Testing serialization of complex tool storage",success);
  }

  void TestWriteStorageToInvalidFile()
  {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = this->CreateTestData_SimpleStorage();

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
    bool success = m_Serializer->Serialize(m_FileName1,myStorage);
    CPPUNIT_ASSERT_MESSAGE("Testing serialization of simple tool storage",success);
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
