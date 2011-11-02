/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

    static void TestReadInvalidStorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize("noStorage.tfl");
    MITK_TEST_CONDITION_REQUIRED(readStorage->isEmpty(),"Testing deserialization of invalid data storage.");
    MITK_TEST_CONDITION_REQUIRED(myDeserializer->GetErrorMessage() == "Cannot open 'noStorage.tfl' for reading", "Checking Error Message");
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
    bool success = true;
    success = mySerializer->Serialize(filename,myStorage);
	
    MITK_TEST_CONDITION_REQUIRED(!success,"Testing serialization into invalid file.");
    }

  };

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageSerializerAndDeserializerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorageSerializerAndDeserializer");

  /** TESTS DEACTIVATED BECAUSE OF DART-CLIENT PROBLEMS
  NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiationSerializer();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiationDeserializer();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteComplexToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadComplexToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadInvalidStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteStorageToInvalidFile();
  NavigationToolStorageSerializerAndDeserializerTestClass::CleanUp();
  */

  MITK_TEST_END();
}