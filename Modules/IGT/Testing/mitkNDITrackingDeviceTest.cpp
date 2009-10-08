/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNDITrackingDevice.h"
#include "mitkNDIPassiveTool.h"
//#include "mitkSerialCommunication.h"
//#include "mitkNDIPassiveToolTest.cpp"
#include "mitkTestingMacros.h"
#include "mitkTrackingTypes.h"
#include "mitkTrackingTool.h"
#include "mitkStandardFileLocations.h"

/**Documentation
* NDIPassiveTool has a protected constructor and a protected itkNewMacro
* so that only it's friend class NDITrackingDevice is able to instantiate
* tool objects. Therefore, we derive from NDIPassiveTool and add a 
* public itkNewMacro, so that we can instantiate and test the class
*/
class NDIPassiveToolTestClass : public mitk::NDIPassiveTool
{
public:
  mitkClassMacro(NDIPassiveToolTestClass, NDIPassiveTool);

  /** make a public constructor, so that the test is able
  *   to instantiate NDIPassiveTool
  */
  itkNewMacro(Self);

protected:
  NDIPassiveToolTestClass() : mitk::NDIPassiveTool()  
  {
  }
};

int mitkNDITrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NDITrackingDevice ");

  // let's create an object of our class  
  mitk::NDITrackingDevice::Pointer myNDITrackingDevice = mitk::NDITrackingDevice::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myNDITrackingDevice.IsNotNull(),"Testing instantiation\n");


  MITK_TEST_CONDITION_REQUIRED(myNDITrackingDevice->GetMode() == mitk::TrackingDevice::Setup ,"Checking tracking device state == setup.\n");

  //OpenConnection
  //MITK_TEST_CONDITION( (!myNDITrackingDevice->OpenConnection()), "Testing behavior of method OpenConnection() (Errors should occur because Tracking Device is not activated).");
  // <-- this test is dangerous. It implies that no tracking device is connected to the dartclient pc, which could be wrong.

  //CloseConnection
  MITK_TEST_CONDITION( (myNDITrackingDevice->CloseConnection()), "Testing behavior of method CloseConnection().");

  //StartTracking
  MITK_TEST_CONDITION( (!myNDITrackingDevice->StartTracking()), "Testing behavior of method StartTracking().");

  //Beep(unsigned char count)
  MITK_TEST_CONDITION( (myNDITrackingDevice->Beep(3)== false), "Testing behavior of method Beep(). No Tracking device initialized!");

  //AddTool( const char* toolName, const char* fileName, TrackingPriority p /*= NDIPassiveTool::Dynamic*/ )
  std::string file = mitk::StandardFileLocations::GetInstance()->FindFile("SROMFile.rom", "Modules/IGT/Testing/Data");
  const char *name = file.c_str();

  MITK_TEST_CONDITION( (myNDITrackingDevice->AddTool("Tool0", name))!=NULL, "Testing AddTool() for tool 0.");

  //GetToolCount()
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetToolCount())==1, "Testing GetToolCount() for one tool.");

  //GetTool(unsigned int toolNumber)
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetTool(0))!=NULL, "Testing GetTool() for tool 0.");

  mitk::TrackingTool::Pointer testtool = myNDITrackingDevice->GetTool(0);

  //UpdateTool(mitk::TrackingTool* tool)
  MITK_TEST_CONDITION( (!myNDITrackingDevice->UpdateTool(testtool)), "Testing behavior of method UpdateTool().\n");

  //RemoveTool(mitk::TrackingTool* tool)
  MITK_TEST_CONDITION( (myNDITrackingDevice->RemoveTool(testtool)), "Testing RemoveTool()for tool 0.");

  //SetOperationMode(OperationMode mode)
  MITK_TEST_CONDITION( (myNDITrackingDevice->SetOperationMode( mitk::MarkerTracking3D )== true ), "Testing behavior of method SetOperationMode().\n");

  //GetOperationMode()
  myNDITrackingDevice->SetOperationMode(mitk::MarkerTracking3D);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==2),"" );

  myNDITrackingDevice->SetOperationMode(mitk::ToolTracking5D);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==1),"" );

  myNDITrackingDevice->SetOperationMode(mitk::HybridTracking);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==3), "Testing behavior of method GetOperationMode().\n");

  //GetMarkerPositions(MarkerPointContainerType* markerpositions)
  mitk::MarkerPointContainerType* markerpositions = new mitk::MarkerPointContainerType();
  MITK_TEST_CONDITION( (!myNDITrackingDevice->GetMarkerPositions(markerpositions)), "Testing behavior of method GetMarkerPositions().\n");
  delete markerpositions;

  // always end with this!
  MITK_TEST_END();
}