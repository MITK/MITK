/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 14:52:01 +0200 (Mi, 13. Mai 2009) $
Version:   $Revision: 17230 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTrackingDeviceSource.h"
#include "mitkVirtualTrackingDevice.h"

#include "mitkTestingMacros.h"
#include <mitkReferenceCountWatcher.h>
#include "itksys/SystemTools.hxx"

/**Documentation
 *  test for the class "NavigationDataSource".
 */
int mitkTrackingDeviceSourceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingDeviceSource");

  // let's create an object of our class  
  mitk::TrackingDeviceSource::Pointer mySource = mitk::TrackingDeviceSource::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(mySource.IsNotNull(), "Testing instantiation");

  mySource->SetTrackingDevice(NULL);
  MITK_TEST_CONDITION(mySource->GetTrackingDevice() == NULL, "Testing Set/GetTrackingDevice(NULL)");
  MITK_TEST_CONDITION(mySource->GetNumberOfOutputs() == 0, "Testing GetNumberOfOutputs with NULL td");
  MITK_TEST_FOR_EXCEPTION(std::invalid_argument, mySource->Connect());
  MITK_TEST_FOR_EXCEPTION(std::invalid_argument, mySource->StartTracking());
  
  mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
  

  mySource->SetTrackingDevice(tracker);
  MITK_TEST_CONDITION(mySource->GetTrackingDevice() == tracker.GetPointer(), "Testing Set/GetTrackingDevice(tracker)");
  MITK_TEST_CONDITION(mySource->GetNumberOfOutputs() == 0, "Testing GetNumberOfOutputs with no tool tracker");
  tracker = mitk::VirtualTrackingDevice::New();
  mitk::ReferenceCountWatcher::Pointer watch = new mitk::ReferenceCountWatcher(tracker);

  tracker->AddTool("T0");
  tracker->AddTool("T1");
  mySource->SetTrackingDevice(tracker);
  MITK_TEST_CONDITION(mySource->GetTrackingDevice() == tracker.GetPointer(), "Testing Set/GetTrackingDevice(tracker2)");
  MITK_TEST_CONDITION(mySource->GetNumberOfOutputs() == 2, "Testing GetNumberOfOutputs with 2 tools tracker");
  MITK_TEST_CONDITION(mySource->IsConnected() == false, "Testing IsConnected()");
  mySource->Connect();
  MITK_TEST_CONDITION(mySource->IsConnected() == true, "Testing Connect()/IsConnected()");
  MITK_TEST_CONDITION(tracker->GetState() == mitk::TrackingDevice::Ready, "Testing Connect()/IsConnected() 2");
  mySource->Disconnect();
  MITK_TEST_CONDITION(mySource->IsConnected() == false, "Testing Disconnect()/IsConnected()");
  MITK_TEST_CONDITION(tracker->GetState() == mitk::TrackingDevice::Setup, "Testing Disconnect()/IsConnected() 2");
  
  mySource->Connect();
  mySource->StartTracking();
  MITK_TEST_CONDITION(mySource->IsConnected() == true, "Testing StartTracking()/IsConnected()");
  MITK_TEST_CONDITION(mySource->IsTracking() == true, "Testing StartTracking()/IsTracking()");
  MITK_TEST_CONDITION(tracker->GetState() == mitk::TrackingDevice::Tracking, "Testing StartTracking()/IsTracking() 2");

  unsigned long modTime = mySource->GetMTime();
  mySource->UpdateOutputInformation();
  MITK_TEST_CONDITION(mySource->GetMTime() != modTime, "Testing if UpdateOutputInformation() modifies the object");

  mitk::NavigationData* nd0 = mySource->GetOutput(0);
  
  nd0->Update();
  mitk::NavigationData::PositionType pos = nd0->GetPosition();
  itksys::SystemTools::Delay(500); // allow the tracking thread to advance the tool position
  nd0->Update();
  MITK_TEST_CONDITION(mitk::Equal(nd0->GetPosition(), pos) == false, "Testing if output changes on each update");
  mitk::NavigationData* nd1 = mySource->GetOutput(1);
  MITK_TEST_CONDITION(nd0 != nd1, "Testing if outputs are different");
  MITK_TEST_CONDITION(mitk::Equal(nd0->GetPosition(), nd1->GetPosition()) == false, "Testing if output contents are different");

  mySource->StopTracking();
  mySource->Disconnect();
  
  tracker = mitk::VirtualTrackingDevice::New();
  mySource->SetTrackingDevice(tracker);
  mySource->StartTracking();
  MITK_TEST_CONDITION(watch->GetReferenceCount() == 0, "Testing if reference to previous tracker object is released");

  mySource = NULL;
  tracker = NULL;
  // always end with this!
  MITK_TEST_END();
}
