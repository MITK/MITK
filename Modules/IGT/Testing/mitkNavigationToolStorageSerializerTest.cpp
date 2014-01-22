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

//headers of IGT classes releated to the tested class
#include <mitkNavigationToolStorageSerializer.h>

class mitkNavigationToolStorageSerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationToolStorageSerializerTestSuite);
  MITK_TEST(TestInstantiationSerializer);
  MITK_TEST(TestWriteSimpleToolStorage);
  MITK_TEST(TestWriteComplexToolStorage);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  std::string m_FileName1;

public:

  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp()
  {
    m_FileName1 = mitk::IOUtil::CreateTemporaryFile();
  }

  void tearDown()
  {
    try
    {
    std::remove(m_FileName1.c_str());
    }
    catch(...)
    {
    MITK_INFO << "Warning: Error occured when deleting test file!";
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

  //create Serializer
  mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();

  //test serialization
  bool success = mySerializer->Serialize(m_FileName1,myStorage);
  CPPUNIT_ASSERT_MESSAGE("Testing serialization of simple tool storage",success);
  }

  void TestWriteComplexToolStorage()
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

    //create Serializer
    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();


    //test serialization
    bool success = mySerializer->Serialize(m_FileName1,myStorage);
    CPPUNIT_ASSERT_MESSAGE("Testing serialization of complex tool storage",success);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageSerializer)
