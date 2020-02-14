/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    mitk::TrackingDevice::Pointer,
    mitk::NavigationToolStorage::Pointer,
    std::string*,
    std::vector<int>*)
  {
    return nullptr;
  }
}
