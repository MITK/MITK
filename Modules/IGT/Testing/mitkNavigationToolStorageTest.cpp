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

#include "mitkNavigationToolStorage.h"
#include "mitkNavigationTool.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"

class mitkNavigationToolStorageTestClass
  {
  public:

    static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    MITK_TEST_CONDITION_REQUIRED(myStorage.IsNotNull(),"Testing instantiation with constructor 1.")

    mitk::DataStorage::Pointer myDataStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer());
    mitk::NavigationToolStorage::Pointer myStorage2 = mitk::NavigationToolStorage::New(myDataStorage);
    MITK_TEST_CONDITION_REQUIRED(myStorage2.IsNotNull(),"Testing instantiation with constructor 2.")

    }

    static void TestAddAndDelete()
    {
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    MITK_TEST_CONDITION_REQUIRED(myStorage->AddTool(myTool1),"Testing: Add 1st tool.");
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==1,"Testing: Is first tool in storage?");

    //second tool
    mitk::NavigationTool::Pointer myTool2 = mitk::NavigationTool::New();
    myTool2->SetIdentifier("002");
    MITK_TEST_CONDITION_REQUIRED(myStorage->AddTool(myTool2),"Testing: Add 2nd tool.");
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==2,"Testing: Is second tool in storage?");

    //third tool (same identifier => not valid!)
    mitk::NavigationTool::Pointer myTool3 = mitk::NavigationTool::New();
    myTool3->SetIdentifier("002");
    MITK_TEST_CONDITION_REQUIRED(!myStorage->AddTool(myTool3),"Testing: Add 3rd tool, which is not valid.");
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==2,"Testing: 3rd tool should NOT be in the storage!");

    //delete second tool
    myStorage->DeleteTool(1);
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==1,"Testing: Delete 2nd tool.");

    //delete first tool
    myStorage->DeleteTool(0);
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==0,"Testing: Delete 1st tool.");

    //delete tool with wrong tool number
    MITK_TEST_CONDITION_REQUIRED(!myStorage->DeleteTool(412),"Testing: Delete non-existing tool. No error should occur!");
    }

    static void TestAddAndDelete100Tools()
    {
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    for(int i=0; i<100; i++)
      {
      mitk::NavigationTool::Pointer myTool = mitk::NavigationTool::New();
      std::stringstream str;
      str << i;
      myTool->SetIdentifier(str.str());
      myStorage->AddTool(myTool);
      }
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==100,"Testing: Adding 100 tools.");
    for(int i=99; i>-1; i--)
      {
      myStorage->DeleteTool(i);
      }
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==0,"Testing: Delete 100 tools.");
    }

    static void TestAddAndDelete100ToolsAtOnce()
    {
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    for(int i=0; i<100; i++)
      {
      mitk::NavigationTool::Pointer myTool = mitk::NavigationTool::New();
      std::stringstream str;
      str << i;
      myTool->SetIdentifier(str.str());
      myStorage->AddTool(myTool);
      }
    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==100,"Testing: Adding 100 tools.");
    MITK_TEST_CONDITION_REQUIRED(!myStorage->isEmpty(),"Testing: method isEmpty() with non empty storage.");
    MITK_TEST_CONDITION_REQUIRED(myStorage->DeleteAllTools(),"Testing: Delete method.");
    MITK_TEST_CONDITION_REQUIRED(myStorage->isEmpty(),"Testing: method isEmpty() with empty storage.");

    MITK_TEST_CONDITION_REQUIRED(myStorage->GetToolCount()==0,"Testing: Delete 100 tools at once.");
    }

    static void TestGetTool()
    {
    //let's create an object of our class
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();

    //let's add two different tools
    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetSerialNumber("0815");
    myTool1->SetIdentifier("001");
    mitk::DataNode::Pointer toolNode = mitk::DataNode::New();
    toolNode->SetName("Tool1");
    myTool1->SetDataNode(toolNode);
    myStorage->AddTool(myTool1);

    //second tool
    mitk::NavigationTool::Pointer myTool2 = mitk::NavigationTool::New();
    myTool2->SetSerialNumber("0816");
    myTool2->SetIdentifier("002");
    myStorage->AddTool(myTool2);

    //let's try to get the first tool in different ways.
    mitk::NavigationTool::Pointer myToolGet = myStorage->GetTool(0);
    MITK_TEST_CONDITION_REQUIRED(myToolGet==myTool1,"Testing GetTool() by number.");
    myToolGet = myStorage->GetTool("001");
    MITK_TEST_CONDITION_REQUIRED(myToolGet==myTool1,"Testing GetTool() by identifier.");
    myToolGet = myStorage->GetToolByName("Tool1");
    MITK_TEST_CONDITION_REQUIRED(myToolGet==myTool1,"Testing GetTool() by name.");

    //let's try to get a tool which doesn't exist
    myToolGet = myStorage->GetToolByName("quatsch");
    MITK_TEST_CONDITION_REQUIRED(myToolGet.IsNull(),"Testing GetTool() with wrong name.");
    }

    static void TestStorageHandling()
    {
    mitk::DataStorage::Pointer myDataStorage = dynamic_cast<mitk::DataStorage*>(mitk::StandaloneDataStorage::New().GetPointer());
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New(myDataStorage);

    //define first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetIdentifier("001");
    mitk::DataNode::Pointer node1 = mitk::DataNode::New();
    node1->SetName("Tool1");
    myTool1->SetDataNode(node1);

    //define second tool
    mitk::NavigationTool::Pointer myTool2 = mitk::NavigationTool::New();
    myTool2->SetIdentifier("002");
    mitk::DataNode::Pointer node2 = mitk::DataNode::New();
    node2->SetName("Tool2");
    myTool2->SetDataNode(node2);

    //execute tests
    MITK_TEST_CONDITION_REQUIRED(myStorage->AddTool(myTool1),"Testing: Add one tool.");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool1")==node1,"Testing: Is data node in data storage?");
    MITK_TEST_CONDITION_REQUIRED(myStorage->DeleteAllTools(),"Deleting all tools.");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool1")==NULL,"Testing: Was data node removed from storage?");
    MITK_TEST_CONDITION_REQUIRED(myStorage->AddTool(myTool1),"Testing: Add two tools (1).");
    MITK_TEST_CONDITION_REQUIRED(myStorage->AddTool(myTool2),"Testing: Add two tools (2).");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool1")==node1,"Testing: Is data node in data storage (1)?");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool2")==node2,"Testing: Is data node in data storage (2)?");
    MITK_TEST_CONDITION_REQUIRED(myStorage->DeleteTool(0),"Deleting tool 1.");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool1")==NULL,"Testing: Was data node 1 deleted?");
    MITK_TEST_CONDITION_REQUIRED(myDataStorage->GetNamedNode("Tool2")==node2,"Testing: Is data node 2 still in data storage?");

    }
  };

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorage")

  mitkNavigationToolStorageTestClass::TestInstantiation();
  mitkNavigationToolStorageTestClass::TestAddAndDelete();
  mitkNavigationToolStorageTestClass::TestAddAndDelete100Tools();
  mitkNavigationToolStorageTestClass::TestAddAndDelete100ToolsAtOnce();
  mitkNavigationToolStorageTestClass::TestGetTool();
  mitkNavigationToolStorageTestClass::TestStorageHandling();

  MITK_TEST_END()
}


