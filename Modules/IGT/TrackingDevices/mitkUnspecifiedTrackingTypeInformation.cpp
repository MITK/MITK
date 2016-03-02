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

#include "mitkUnspecifiedTrackingTypeInformation.h"

#include "mitkIGTHardwareException.h"

namespace mitk
{
  std::string UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()
  {
    return "Tracking System Not Specified";
  }

  TrackingDeviceData UnspecifiedTrackingTypeInformation::GetDeviceDataUnspecified()
  {
    TrackingDeviceData data = { UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Unspecified System", "cube", "X" };
    return data;
  }
  // Careful when changing the "invalid" device: The mitkTrackingTypeTest is using it's data.
  TrackingDeviceData UnspecifiedTrackingTypeInformation::GetDeviceDataInvalid()
  {
    TrackingDeviceData data = { UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Invalid Tracking System", "", "X" };
    return data;
  }

  UnspecifiedTrackingTypeInformation::UnspecifiedTrackingTypeInformation()
  {
    m_DeviceName = UnspecifiedTrackingTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataUnspecified());
    m_TrackingDeviceData.push_back(GetDeviceDataInvalid());
  }

  UnspecifiedTrackingTypeInformation::~UnspecifiedTrackingTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer UnspecifiedTrackingTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    return nullptr;
  }
}
