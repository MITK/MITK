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
  tracker->SetRefreshRate(10);


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

  //test getOutput()
  mitk::NavigationData* nd0 = mySource->GetOutput();
  MITK_TEST_CONDITION(nd0!=NULL,"Testing GetOutput() [1]");
  nd0 = mySource->GetOutput(nd0->GetName());
  MITK_TEST_CONDITION(nd0!=NULL,"Testing GetOutput() [2]");

  //test getOutputIndex()
  MITK_TEST_CONDITION(mySource->GetOutputIndex(nd0->GetName())==0,"Testing GetOutputIndex()");

  //test GraftNthOutput()
  mitk::NavigationData::Pointer ndCopy = mitk::NavigationData::New();
  mySource->GraftNthOutput(1,nd0);
  ndCopy = mySource->GetOutput(1);
  MITK_TEST_CONDITION(std::string(ndCopy->GetName())==std::string(nd0->GetName()),"Testing GraftNthOutput()");

  //test GetParameters()
  mitk::PropertyList::ConstPointer p = mySource->GetParameters();
  MITK_TEST_CONDITION(p.IsNotNull(),"Testing GetParameters()");

  nd0->Update();
  mitk::NavigationData::PositionType pos = nd0->GetPosition();
  unsigned long tmpMTime0 = nd0->GetMTime();
  itksys::SystemTools::Delay(500); // allow the tracking thread to advance the tool position
  nd0->Update();
  mitk::NavigationData::PositionType newPos = nd0->GetPosition();
  if(nd0->GetMTime() == tmpMTime0) //tool not modified yet
  {
    MITK_TEST_CONDITION(mitk::Equal(newPos, pos) == true, "Testing if output changes on each update");
  }
  else
  {
    MITK_TEST_CONDITION(mitk::Equal(newPos, pos) == false, "Testing if output changes on each update");
  }

  mySource->StopTracking();
  mySource->Disconnect();

  tracker = mitk::VirtualTrackingDevice::New();
  mySource->SetTrackingDevice(tracker);
  MITK_TEST_CONDITION(watch->GetReferenceCount() == 0, "Testing if reference to previous tracker object is released");
  watch = NULL;

  MITK_TEST_FOR_EXCEPTION(std::runtime_error, mySource->StartTracking()); // new tracker, needs Connect() before StartTracking()

  mySource->Connect();
  mySource->StartTracking();
 // itksys::SystemTools::Delay(800); // wait for tracking thread to start properly //DEBUG ONLY  --> race condition. will the thread start before the object is destroyed? --> maybe hold a smartpointer?
  try
  {
    mySource = NULL;  // delete source
    tracker = NULL;   // delete tracker --> both should not result in any exceptions or deadlocks
  }
  catch (...)
  {
    MITK_TEST_FAILED_MSG(<< "exception during destruction of source or tracker!");
  }

  // always end with this!
  MITK_TEST_END();
}
