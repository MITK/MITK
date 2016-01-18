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

#include "mitkVirtualTrackerTypeInformation.h"

#include "mitkVirtualTrackingDevice.h"

namespace mitk
{
  std::string VirtualTrackerTypeInformation::GetTrackingDeviceName()
  {
    return "Virtual Tracker";
  }

  TrackingDeviceData VirtualTrackerTypeInformation::GetDeviceDataVirtualTracker()
  {
    TrackingDeviceData data = { VirtualTrackerTypeInformation::GetTrackingDeviceName(), "Virtual Tracker", "cube", "X" };
    return data;
  }

  VirtualTrackerTypeInformation::VirtualTrackerTypeInformation()
  {
    m_DeviceName = VirtualTrackerTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataVirtualTracker());
  }

  VirtualTrackerTypeInformation::~VirtualTrackerTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer VirtualTrackerTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::VirtualTrackingDevice::Pointer thisDevice = dynamic_cast<mitk::VirtualTrackingDevice*>(trackingDevice.GetPointer());
    *toolCorrespondencesInToolStorage = std::vector<int>();

    //add the tools to the tracking device
    for (int i = 0; i < navigationTools->GetToolCount(); i++)
    {
      mitk::NavigationTool::Pointer thisNavigationTool = navigationTools->GetTool(i);
      toolCorrespondencesInToolStorage->push_back(i);
      bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str());
      if (!toolAddSuccess)
      {
        //todo error handling
        errorMessage->append("Can't add tool, is the toolfile valid?");
        return NULL;
      }
    }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}