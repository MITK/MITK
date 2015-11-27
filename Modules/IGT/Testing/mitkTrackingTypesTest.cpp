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

  std::vector<mitk::TrackingDeviceData> ResultSet = deviceTypeCollection->GetDeviceDataForLine(mitk::TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED);
  MITK_TEST_CONDITION(ResultSet.size() == 2,"Test correct retrieval of DeviceData: Number of results");
  MITK_TEST_CONDITION(ResultSet[0].Line == mitk::TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED, "Test correct retrieval of DeviceData: Correct device");
  MITK_TEST_CONDITION(ResultSet[1].Line == mitk::TRACKING_DEVICE_IDENTIFIER_INVALID, "Test correct retrieval of DeviceData: Correct device");

  std::vector<mitk::TrackingDeviceData> ResultSet2 = deviceTypeCollection->GetDeviceDataForLine(mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL);
  MITK_TEST_CONDITION(ResultSet2.size() == 1, "Test correct retrieval of DeviceData: Number of results");
  MITK_TEST_CONDITION(ResultSet2[0].Line == mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL, "Test correct retrieval of DeviceData: Correct device");

  mitk::TrackingDeviceData data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(mitk::TRACKING_DEVICE_IDENTIFIER_INVALID);
  MITK_TEST_CONDITION(data.Line == mitk::TRACKING_DEVICE_IDENTIFIER_INVALID,"Test correct retrieval of first compatible model");
  data = deviceTypeCollection->GetDeviceDataByName("Invalid Tracking System");
  MITK_TEST_CONDITION(data.Line == mitk::TRACKING_DEVICE_IDENTIFIER_INVALID, "Test GetDeviceDataByName");
  data = deviceTypeCollection->GetDeviceDataByName("Unspecified System");
  MITK_TEST_CONDITION(data.Line == mitk::TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED, "Test GetDeviceDataByName");

  data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL);
  MITK_TEST_CONDITION(data.Line == mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL, "Test correct retrieval of first compatible model");
  data = deviceTypeCollection->GetDeviceDataByName("Virtual Tracker");
  MITK_TEST_CONDITION(data.Line == mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL, "Test GetDeviceDataByName");

  MITK_TEST_END();

}
