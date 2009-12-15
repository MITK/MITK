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

#include "mitkNavigationToolStorage.h"
#include "mitkNavigationTool.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"

class mitkNavigationToolStorageTestClass
  {
  public:

    static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    MITK_TEST_CONDITION_REQUIRED(myStorage.IsNotNull(),"Testing instantiation")
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

    static void TestGetTool()
    {
    //let's create an object of our class
    mitk::NavigationToolStorage::Pointer myStorage = mitk::NavigationToolStorage::New();
    
    //let's add two different tools
    //first tool
    mitk::NavigationTool::Pointer myTool1 = mitk::NavigationTool::New();
    myTool1->SetSerialNumber("0815");
    myTool1->SetIdentifier("001");
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
    }
  };

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorage")

  mitkNavigationToolStorageTestClass::TestInstantiation();
  mitkNavigationToolStorageTestClass::TestAddAndDelete();
  mitkNavigationToolStorageTestClass::TestAddAndDelete100Tools();
  mitkNavigationToolStorageTestClass::TestGetTool();

  MITK_TEST_END()
}


