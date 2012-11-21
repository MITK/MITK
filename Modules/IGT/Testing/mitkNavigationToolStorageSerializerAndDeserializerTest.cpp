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

//Poco headers
#include "Poco/Path.h"

#include <mitkNavigationToolStorageSerializer.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkCommon.h>
#include <mitkTestingMacros.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStandardFileLocations.h>
#include <mitkSTLFileReader.h>

#include "mitkNavigationToolStorage.h"

//POCO
#include <Poco/Exception.h>

#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class NavigationToolStorageSerializerAndDeserializerTestClass
  {
  public:

    static void TestInstantiationSerializer()
    {
    // let's create objects of our classes
    mitk::NavigationToolStorageSerializer::Pointer testSerializer = mitk::NavigationToolStorageSerializer::New();
    MITK_TEST_CONDITION_REQUIRED(testSerializer.IsNotNull(),"Testing instantiation of NavigationToolStorageSerializer");
    }

    static void TestInstantiationDeserializer()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer testDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    MITK_TEST_CONDITION_REQUIRED(testDeserializer.IsNotNull(),"Testing instantiation of NavigationToolStorageDeserializer")
    }

    static void TestWriteSimpleToolStorage()
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

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage";

    //test serialization
    bool success = mySerializer->Serialize(filename,myStorage);
    MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of simple tool storage");
    }

    static void TestWriteAndReadSimpleToolStorageWithToolLandmarks()
    {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    mitk::PointSet::Pointer CalLandmarks1 = mitk::PointSet::New();
    mitk::Point3D testPt1;
    mitk::FillVector3D(testPt1,1,2,3);
    CalLandmarks1->SetPoint(0,testPt1);
    mitk::PointSet::Pointer RegLandmarks1 = mitk::PointSet::New();
    mitk::Point3D testPt2;
    mitk::FillVector3D(testPt2,4,5,6);
    RegLandmarks1->SetPoint(5,testPt2);
    myTool1->SetToolCalibrationLandmarks(CalLandmarks1);
    myTool1->SetToolRegistrationLandmarks(RegLandmarks1);
    mitk::Point3D toolTipPos;
    mitk::FillVector3D(toolTipPos,1.3423,2.323,4.332);
    mitk::Quaternion toolTipRot = mitk::Quaternion(0.1,0.2,0.3,0.4);
    myTool1->SetToolTipPosition(toolTipPos);
    myTool1->SetToolTipOrientation(toolTipRot);
    myStorage->AddTool(myTool1);

    //create Serializer
    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorageToolReg.storage";

    //test serialization
    bool success = mySerializer->Serialize(filename,myStorage);
    MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of tool storage with tool registrations");

    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorageToolReg.storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage.IsNotNull(),"Testing deserialization of tool storage with tool registrations");
    MITK_TEST_CONDITION_REQUIRED(readStorage->GetToolCount()==1," ..Testing number of tools in storage");

    mitk::PointSet::Pointer readRegLandmarks = readStorage->GetTool(0)->GetToolRegistrationLandmarks();
    mitk::PointSet::Pointer readCalLandmarks = readStorage->GetTool(0)->GetToolCalibrationLandmarks();

    MITK_TEST_CONDITION_REQUIRED(((readRegLandmarks->GetPoint(5)[0] == 4)&&(readRegLandmarks->GetPoint(5)[1] == 5)&&(readRegLandmarks->GetPoint(5)[2] == 6)),"..Testing if tool registration landmarks have been stored and loaded correctly.");
    MITK_TEST_CONDITION_REQUIRED(((readCalLandmarks->GetPoint(0)[0] == 1)&&(readCalLandmarks->GetPoint(0)[1] == 2)&&(readCalLandmarks->GetPoint(0)[2] == 3)),"..Testing if tool calibration landmarks have been stored and loaded correctly.");

    mitk::Point3D readToolTipPos = readStorage->GetTool(0)->GetToolTipPosition();
    mitk::Quaternion readToolTipRot = readStorage->GetTool(0)->GetToolTipOrientation();

    MITK_TEST_CONDITION_REQUIRED(((float(readToolTipPos[0]) == float(1.3423))&&
                                  (float(readToolTipPos[1]) == float(2.323))&&
                                  (float(readToolTipPos[2]) == float(4.332))),
                                  "..Testing if tool tip position has been stored and loaded correctly.");

    MITK_TEST_CONDITION_REQUIRED(((float(readToolTipRot.x()) == float(0.1))&&
                                  (float(readToolTipRot.y()) == float(0.2))&&
                                  (float(readToolTipRot.z()) == float(0.3))&&
                                  (float(readToolTipRot.r()) == float(0.4))),
                                  "..Testing if tool tip orientation has been stored and loaded correctly.");

    }

    static void TestReadSimpleToolStorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage.IsNotNull(),"Testing deserialization of simple tool storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage->GetToolCount()==3," ..Testing number of tools in storage");
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
    MITK_TEST_CONDITION_REQUIRED(foundtool1&&foundtool2&&foundtool3," ..Testing if identifiers of tools where saved / loaded successfully");
    }

    static void CleanUp()
    {
    try
      {
      std::remove((mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage").c_str());
      std::remove((mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorageToolReg.storage").c_str());
      std::remove((mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage2.storage").c_str());
      }
    catch(...)
      {
      MITK_INFO << "Warning: Error occured when deleting test file!";
      }
    }

    static void TestWriteComplexToolStorage()
    {

    //create first tool
    mitk::Surface::Pointer testSurface;
    std::string toolFileName = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool", "Modules/IGT/Testing/Data");
    MITK_TEST_CONDITION(toolFileName.empty() == false, "Check if tool calibration of claron tool file exists");
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetCalibrationFile(toolFileName);

    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("ClaronTool");

    //load an stl File
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
      {
      stlReader->SetFileName( mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool.stl", "Testing/Data/").c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }

    if ( stlReader->GetOutput() == NULL )
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }
    else
      {
      testSurface = stlReader->GetOutput();
      myNode->SetData(testSurface);
      }

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
    try
      {
      stlReader->SetFileName( mitk::StandardFileLocations::GetInstance()->FindFile("EMTool.stl", "Testing/Data/").c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }

    if ( stlReader->GetOutput() == NULL )
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }
    else
      {
      testSurface2 = stlReader->GetOutput();
      myNode2->SetData(testSurface2);
      }

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

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage2.storage";

    //test serialization
    bool success = mySerializer->Serialize(filename,myStorage);
    MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of complex tool storage");

    }

    static void TestReadComplexToolStorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage2.storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage.IsNotNull(),"Testing deserialization of complex tool storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage->GetToolCount()==2," ..Testing number of tools in storage");
    }

    static void TestReadNotExistingStorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);

    bool exceptionThrown = false;
    try
      {
      mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize("noStorage.tfl");
      }
    catch (mitk::IGTException e)
      {
      exceptionThrown = true;
      }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing if exception is thrown if a non existing storage is given for deserialization.");
    }

    static void TestReadStorageWithUnknownFiletype()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!

    std::string toolFileName = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool.stl", "Modules/IGT/Testing/Data");
    MITK_TEST_CONDITION(toolFileName.empty() == false, "Check if tool calibration of claron tool file exists");
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);

    bool exceptionThrown = false;
    try
      {
      mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(toolFileName);
      }
    catch (mitk::IGTException e)
      {
      exceptionThrown = true;
      }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing if exception is thrown if a wrong file type is given for deserialization.");
    }

    static void TestReadZipFileWithNoToolstorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!

    std::string toolFileName = mitk::StandardFileLocations::GetInstance()->FindFile("Empty.zip", "Modules/IGT/Testing/Data");
    MITK_TEST_CONDITION(toolFileName.empty() == false, "Check if tool calibration of claron tool file exists");
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);

    bool exceptionThrown = false;
    try
      {
      mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(toolFileName);
      }
    catch (mitk::IGTException e)
      {
      exceptionThrown = true;
      }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing if exception is thrown if a empty zip file is given for deserialization.");
    }


    static void TestWriteStorageToInvalidFile()
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

    //create filename
    #ifdef WIN32
      std::string filename = "C:\342INVALIDFILE<>.storage"; //invalid filename for windows
    #else
      std::string filename = "/dsfdsf:$§$342INVALIDFILE.storage"; //invalid filename for linux
    #endif


    //test serialization
    bool exceptionThrown = false;
    try
      {
      mySerializer->Serialize(filename,myStorage);
      }
    catch(mitk::IGTException e)
      {
      exceptionThrown = true;
      }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing if an exception is thrown if an invalid file is used.");
    }


    static void TestWriteEmptyToolStorage()
    {
    //create Tool Storage
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //create Serializer
    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();

    std::string filename;
    std::string optionDirectory;
    std::string separator;
    try
    {
        //create filename
        separator = Poco::Path::separator();
        optionDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory();
        filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage";
    }
    catch (std::exception& e) {
        MITK_ERROR << "File access Exception: " << e.what();
        MITK_INFO << "separator: " << separator;
        MITK_INFO << "optionDirectory: " << optionDirectory;
        MITK_TEST_FAILED_MSG(<<"Could not create filename for Exceptiontest");
    }

    //test serialization
    bool success = mySerializer->Serialize(filename,myStorage);
    MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of simple tool storage");
    }
    //new tests for exception throwing of NavigationToolStorageSerializer
    static void TestSerializerForExceptions()
    {
    mitk::NavigationToolStorageSerializer::Pointer testSerializer = mitk::NavigationToolStorageSerializer::New();
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //create an invalid filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"";

    //now try to serialize an check if an exception is thrown
    bool ExceptionThrown = false;
    try
      {
      testSerializer->Serialize(filename,myStorage);
      }
    catch(mitk::IGTException)
      {
      ExceptionThrown = true;
      }
    MITK_TEST_CONDITION_REQUIRED(ExceptionThrown, "Testing serializer with invalid filename.");
    }

    //new tests for exception throwing of NavigationToolStorageDeserializer
    static void TestDeserializerForExceptions()
    {


    // Desearializing file with invalid name
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer());
    mitk::NavigationToolStorageDeserializer::Pointer testDeseralizer= mitk::NavigationToolStorageDeserializer::New(tempStorage);
    bool ExceptionThrown1 = false;
    try
      {
      mitk::NavigationToolStorage::Pointer readStorage = testDeseralizer->Deserialize("InvalidName");
      }
    catch(mitk::IGTException)
      {
      ExceptionThrown1 = true;
      }
    MITK_TEST_CONDITION_REQUIRED(ExceptionThrown1, "Testing deserializer with invalid filename.");

    bool ExceptionThrown2 = false;

    // Deserializing of empty zip file
    mitk::NavigationToolStorageDeserializer::Pointer testDeseralizer2= mitk::NavigationToolStorageDeserializer::New(tempStorage);
    try
      {
      std::string filename = mitk::StandardFileLocations::GetInstance()->FindFile("EmptyZipFile.zip", "Modules/IGT/Testing/Data");
      mitk::NavigationToolStorage::Pointer readStorage = testDeseralizer2->Deserialize(filename);
      }
    catch(mitk::IGTException)
      {
      ExceptionThrown2 = true;
      }
    MITK_TEST_CONDITION_REQUIRED(ExceptionThrown2, "Testing deserializer method with empty zip file.");

    }

  };






/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageSerializerAndDeserializerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorageSerializerAndDeserializer");
  try{
  NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiationSerializer();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiationDeserializer();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteAndReadSimpleToolStorageWithToolLandmarks();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteComplexToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadComplexToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadNotExistingStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadStorageWithUnknownFiletype();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadZipFileWithNoToolstorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteStorageToInvalidFile();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteEmptyToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestSerializerForExceptions();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestDeserializerForExceptions();
  }
  catch (std::exception& e) {
      MITK_ERROR << "exception:" << e.what();
      MITK_TEST_FAILED_MSG(<<"Exception occured, test failed!");
  }
  catch (...) {
      MITK_ERROR << "Unknown Exception?";
      MITK_TEST_FAILED_MSG(<<"Exception occured, test failed!");
  }

  NavigationToolStorageSerializerAndDeserializerTestClass::CleanUp();

  MITK_TEST_END();
}
