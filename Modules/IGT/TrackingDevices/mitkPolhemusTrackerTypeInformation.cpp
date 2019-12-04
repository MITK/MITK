/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPolhemusTrackerTypeInformation.h"

#include "mitkPolhemusTrackingDevice.h"

namespace mitk
{
  std::string PolhemusTrackerTypeInformation::GetTrackingDeviceName()
  {
    return "Polhemus tracker";
  }

  TrackingDeviceData PolhemusTrackerTypeInformation::GetDeviceDataPolhemusTrackerLiberty()
  {
    TrackingDeviceData data = { PolhemusTrackerTypeInformation::GetTrackingDeviceName(), "Polhemus Liberty Tracker", "cube", "X" };
    return data;
  }

  PolhemusTrackerTypeInformation::PolhemusTrackerTypeInformation()
  {
    m_DeviceName = PolhemusTrackerTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataPolhemusTrackerLiberty());
  }

  PolhemusTrackerTypeInformation::~PolhemusTrackerTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer PolhemusTrackerTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::PolhemusTrackingDevice::Pointer thisDevice = dynamic_cast<mitk::PolhemusTrackingDevice*>(trackingDevice.GetPointer());
    *toolCorrespondencesInToolStorage = std::vector<int>();
    //add the tools to the tracking device
    for (unsigned int i = 0; i < navigationTools->GetToolCount(); i++)
    {
      mitk::NavigationTool::Pointer thisNavigationTool = navigationTools->GetTool(i);
      toolCorrespondencesInToolStorage->push_back(i);
      bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(), std::stoi(thisNavigationTool->GetIdentifier()));
      if (!toolAddSuccess)
      {
        //todo error handling
        errorMessage->append("Can't add tool, is the toolfile valid?");
        return NULL;
      }
      thisDevice->GetTool(i)->SetToolTipPosition(thisNavigationTool->GetToolTipPosition(), thisNavigationTool->GetToolAxisOrientation());
    }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}
