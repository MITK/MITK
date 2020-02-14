/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingDevice.h"
#include "mitkTestingMacros.h"
#include "mitkTrackingTool.h"
#include "mitkTrackingTypes.h"

#include "mitkCommon.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

//All Tracking devices, which should be avaiable by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"
/**
* Create new class and derive it from TrackingDevice
*/
class TrackingDeviceTestClass : public mitk::TrackingDevice
{
public:

  mitkClassMacro(TrackingDeviceTestClass, mitk::TrackingDevice);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  bool OpenConnection() override{return true;};
  bool CloseConnection() override{return true;};
  bool StartTracking() override{this->SetState(Tracking); this->m_TrackingFinishedMutex->Unlock(); return true;};
  mitk::TrackingTool* GetTool(unsigned int /*toolNumber*/) const override {return nullptr;};
  unsigned int GetToolCount() const override {return 1;};
};

/**
* This function is testing the Class TrackingDevice. For most tests we would need the MicronTracker hardware, so only a few
* simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
* concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
*/
int mitkTrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingDevice");

  mitk::TrackingDeviceTypeCollection deviceTypeCollection;
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIAuroraTypeInformation());
  deviceTypeCollection.RegisterAsMicroservice();

  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIPolarisTypeInformation());
  deviceTypeCollection.RegisterTrackingDeviceType(new mitk::MicronTrackerTypeInformation());

  // Test instantiation of TrackingDevice
  TrackingDeviceTestClass::Pointer trackingDeviceTestClass = TrackingDeviceTestClass::New();
  MITK_TEST_CONDITION(trackingDeviceTestClass.IsNotNull(),"Test instatiation");

  // Test method GetState()
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetState()==mitk::TrackingDevice::Setup,"Mode should be initialized to SETUP");

  // Test method SetType()
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType()==mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(),"Type should be initialized to 'not specified'");
  trackingDeviceTestClass->SetType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType() == mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(), "Type should be NDIAurora, as it has just been set");
  trackingDeviceTestClass->SetType(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType() == mitk::NDIPolarisTypeInformation::GetTrackingDeviceName(), "Type should be NDIPolaris, as it has just been set");
  trackingDeviceTestClass->SetType(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType() == mitk::MicronTrackerTypeInformation::GetTrackingDeviceName(), "Type should be ClaronMicron, as it has just been set");
  trackingDeviceTestClass->SetType(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetType() == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Type should be VirtualTracker, as it has just been set");

  // Test method StopTracking()
  trackingDeviceTestClass->StartTracking();
  trackingDeviceTestClass->StopTracking();
  MITK_TEST_CONDITION(trackingDeviceTestClass->GetState()== mitk::TrackingDevice::Ready,"Type should be NDIAurora, as it has just been set");

  MITK_TEST_CONDITION(deviceTypeCollection.GetTrackingDeviceTypeInformation(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName())->GetTrackingDeviceName()
    == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Test GetTrackingDeviceTypeInformation");

  MITK_TEST_CONDITION(deviceTypeCollection.GetTrackingDeviceTypeInformation(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName())->m_TrackingDeviceData[0].Model
    == "Virtual Tracker", "Test GetTrackingDeviceTypeInformation");

  std::vector<std::string> names = deviceTypeCollection.GetTrackingDeviceTypeNames();
  MITK_TEST_CONDITION(names[0] == mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(), "Test collection name list");
  MITK_TEST_CONDITION(names[1] == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Test collection name list");
  MITK_TEST_CONDITION(names[2] == mitk::NDIPolarisTypeInformation::GetTrackingDeviceName(), "Test collection name list");
  MITK_TEST_CONDITION(names[3] == mitk::MicronTrackerTypeInformation::GetTrackingDeviceName(), "Test collection name list");



  MITK_TEST_END();
}
