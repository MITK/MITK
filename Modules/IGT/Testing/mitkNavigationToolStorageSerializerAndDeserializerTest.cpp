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

class NavigationToolStorageSerializerAndDeserializerTestClass
  {
  public:

    static void TestInstantiation()
    {
    // let's create objects of our classes
    mitk::NavigationToolStorageSerializer::Pointer testSerializer = mitk::NavigationToolStorageSerializer::New();
    MITK_TEST_CONDITION_REQUIRED(testSerializer.IsNotNull(),"Testing instantiation of NavigationToolStorageSerializer")

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
    MITK_TEST_CONDITION_REQUIRED(success,"Testing serialization of tool storage");
    }
 
    static void TestReadSimpleToolStorage()
    {
    mitk::DataStorage::Pointer tempStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer()); //needed for deserializer!
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(tempStorage);
    mitk::NavigationToolStorage::Pointer readStorage = myDeserializer->Deserialize(mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage.IsNotNull(),"Testing deserialization of tool storage");
    MITK_TEST_CONDITION_REQUIRED(readStorage->GetToolCount()==3,"Testing number of tools in storage");
    bool allToolsCorrect = true; //TODO: why is the order of tools changed is save/load process??
    if (!(readStorage->GetTool(0)->GetIdentifier()=="001")) allToolsCorrect = false;
    if (!(readStorage->GetTool(1)->GetIdentifier()=="003")) allToolsCorrect = false;
    if (!(readStorage->GetTool(2)->GetIdentifier()=="002")) allToolsCorrect = false;
    MITK_TEST_CONDITION_REQUIRED(allToolsCorrect,"Testing if identifiers of tools where saved / loaded successfully");
    }

    static void CleanUp()
    {
    std::remove((mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestStorage.storage").c_str());
    }
  };

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageSerializerAndDeserializerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorageSerializerAndDeserializer")

  NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiation();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestWriteSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::TestReadSimpleToolStorage();
  NavigationToolStorageSerializerAndDeserializerTestClass::CleanUp();


  MITK_TEST_END()
}

