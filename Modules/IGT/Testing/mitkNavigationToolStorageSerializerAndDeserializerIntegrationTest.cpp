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
#include <mitkTestingConfig.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

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
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/

public:
  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp()
  {
  }

  void tearDown()
  {
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

};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationToolStorageSerializerAndDeserializerIntegration)



//
//static void TestWriteAndReadSimpleToolStorageWithToolLandmarks()
//{
//  //create Tool Storage
//  mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
//
//  //first tool
//  mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
//  myTool1->SetIdentifier("001");
//  mitk::PointSet::Pointer CalLandmarks1 = mitk::PointSet::New();
//  mitk::Point3D testPt1;
//  mitk::FillVector3D(testPt1,1,2,3);
//  CalLandmarks1->SetPoint(0,testPt1);
//  mitk::PointSet::Pointer RegLandmarks1 = mitk::PointSet::New();
//  mitk::Point3D testPt2;
//  mitk::FillVector3D(testPt2,4,5,6);
//  RegLandmarks1->SetPoint(5,testPt2);
//  myTool1->SetToolCalibrationLandmarks(CalLandmarks1);
//  myTool1->SetToolRegistrationLandmarks(RegLandmarks1);
//  mitk::Point3D toolTipPos;
//  mitk::FillVector3D(toolTipPos,1.3423,2.323,4.332);
//  mitk::Quaternion toolTipRot = mitk::Quaternion(0.1,0.2,0.3,0.4);
//  myTool1->SetToolTipPosition(toolTipPos);
//  myTool1->SetToolTipOrientation(toolTipRot);
//  myStorage->AddTool(myTool1);
//
//  //create Serializer
//  mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();
//
//  //create filename
//  std::string filename = std::string( MITK_TEST_OUTPUT_DIR )+Poco::Path::separator()+"TestStorageToolReg.storage";
//
//  //test serialization
//  bool success = mySerializer->Serialize(filename,myStorage);
//  MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of tool storage with tool registrations");
//
//  mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
//  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
//  mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(std::string( MITK_TEST_OUTPUT_DIR )+Poco::Path::separator()+"TestStorageToolReg.storage");
//  MITK_TEST_CONDITION_REQUIRED(readStorage.IsNotNull(),"Testing deserialization of tool storage with tool registrations");
//  MITK_TEST_CONDITION_REQUIRED(readStorage->GetToolCount()==1," ..Testing number of tools in storage");
//
//  mitk::PointSet::Pointer readRegLandmarks = readStorage->GetTool(0)->GetToolRegistrationLandmarks();
//  mitk::PointSet::Pointer readCalLandmarks = readStorage->GetTool(0)->GetToolCalibrationLandmarks();
//
//  MITK_TEST_CONDITION_REQUIRED(((readRegLandmarks->GetPoint(5)[0] == 4)&&(readRegLandmarks->GetPoint(5)[1] == 5)&&(readRegLandmarks->GetPoint(5)[2] == 6)),"..Testing if tool registration landmarks have been stored and loaded correctly.");
//  MITK_TEST_CONDITION_REQUIRED(((readCalLandmarks->GetPoint(0)[0] == 1)&&(readCalLandmarks->GetPoint(0)[1] == 2)&&(readCalLandmarks->GetPoint(0)[2] == 3)),"..Testing if tool calibration landmarks have been stored and loaded correctly.");
//
//  mitk::Point3D readToolTipPos = readStorage->GetTool(0)->GetToolTipPosition();
//  mitk::Quaternion readToolTipRot = readStorage->GetTool(0)->GetToolTipOrientation();
//
//  MITK_TEST_CONDITION_REQUIRED(((float(readToolTipPos[0]) == float(1.3423))&&
//    (float(readToolTipPos[1]) == float(2.323))&&
//    (float(readToolTipPos[2]) == float(4.332))),
//    "..Testing if tool tip position has been stored and loaded correctly.");
//
//  MITK_TEST_CONDITION_REQUIRED(((float(readToolTipRot.x()) == float(0.1))&&
//    (float(readToolTipRot.y()) == float(0.2))&&
//    (float(readToolTipRot.z()) == float(0.3))&&
//    (float(readToolTipRot.r()) == float(0.4))),
//    "..Testing if tool tip orientation has been stored and loaded correctly.");
//}
//
//

///** This function is testing the TrackingVolume class. */
//int mitkNavigationToolStorageSerializerAndDeserializerTest(int /* argc */, char* /*argv*/[])
//{
//  MITK_TEST_BEGIN("NavigationToolStorageSerializerAndDeserializer");
//  try{
//    TestInstantiationSerializer();
//    TestInstantiationDeserializer();
//    TestWriteSimpleToolStorage();
//    TestWriteAndReadSimpleToolStorageWithToolLandmarks();
//    TestReadSimpleToolStorage();
//    TestWriteComplexToolStorage();
//    TestReadComplexToolStorage();
//    TestReadNotExistingStorage();
//    TestReadStorageWithUnknownFiletype();
//    TestReadZipFileWithNoToolstorage();
//    TestWriteStorageToInvalidFile();
//    TestWriteEmptyToolStorage();
//    TestSerializerForExceptions();
//    TestDeserializerForExceptions();
//  }
//  catch (std::exception& e) {
//    MITK_ERROR << "exception:" << e.what();
//    MITK_TEST_FAILED_MSG(<<"Exception occured, test failed!");
//  }
//  catch (...) {
//    MITK_ERROR << "Unknown Exception?";
//    MITK_TEST_FAILED_MSG(<<"Exception occured, test failed!");
//  }
//
//  CleanUp();
//
//  MITK_TEST_END();
//}
