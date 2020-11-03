/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "vector"
#include "mitkTrackingTypes.h"
#include "mitkTestingMacros.h"

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

#include "mitkVirtualTrackerTypeInformation.h"

/**Documentation
* ClaronTool has a protected constructor and a protected itkFactorylessNewMacro
* so that only it's friend class ClaronTrackingDevice is able to instantiate
* tool objects. Therefore, we derive from ClaronTool and add a
* public itkFactorylessNewMacro, so that we can instantiate and test the class
*/

/**
 * This function tests the ClaronTool class.
 */
int mitkTrackingTypesTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("TrackingTypes");
  mitk::TrackingDeviceTypeCollection* deviceTypeCollection = new mitk::TrackingDeviceTypeCollection;
  deviceTypeCollection->RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
  deviceTypeCollection->RegisterTrackingDeviceType(new mitk::UnspecifiedTrackingTypeInformation());
  deviceTypeCollection->RegisterAsMicroservice();

  std::vector<mitk::TrackingDeviceData> ResultSet = deviceTypeCollection->GetDeviceDataForLine(mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(ResultSet.size() == 2,"Test correct retrieval of DeviceData: Number of results");
  MITK_TEST_CONDITION(ResultSet[0].Model == "Unspecified System", "Test correct retrieval of DeviceData: Correct device");
  MITK_TEST_CONDITION(ResultSet[1].Model == "Invalid Tracking System", "Test correct retrieval of DeviceData: Correct device");
  MITK_TEST_CONDITION(ResultSet[0].Line == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Test correct retrieval of DeviceData: Correct device");
  MITK_TEST_CONDITION(ResultSet[1].Line == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Test correct retrieval of DeviceData: Correct device");


  std::vector<mitk::TrackingDeviceData> ResultSet2 = deviceTypeCollection->GetDeviceDataForLine(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(ResultSet2.size() == 1, "Test correct retrieval of DeviceData: Number of results");
  MITK_TEST_CONDITION(ResultSet2[0].Line == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Test correct retrieval of DeviceData: Correct device");
  MITK_TEST_CONDITION(ResultSet2[0].Model == mitk::VirtualTrackerTypeInformation::GetDeviceDataVirtualTracker().Model, "Test correct retrieval of DeviceData: Correct device");

  mitk::TrackingDeviceData data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(data.Line == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Test correct retrieval of first compatible model");
  data = deviceTypeCollection->GetDeviceDataByName("Invalid Tracking System");
  MITK_TEST_CONDITION(data.Line == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Test GetDeviceDataByName");
  data = deviceTypeCollection->GetDeviceDataByName("Unspecified System");
  MITK_TEST_CONDITION(data.Line == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Test GetDeviceDataByName");

  data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName());
  MITK_TEST_CONDITION(data.Line == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Test correct retrieval of first compatible model");
  data = deviceTypeCollection->GetDeviceDataByName("Virtual Tracker");
  MITK_TEST_CONDITION(data.Line == mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Test GetDeviceDataByName");

  MITK_TEST_END();

}
