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
#include "mitkNDITrackingDevice.h"

namespace mitk
{
  std::string UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()
  {
    return "Tracking System Not Specified";
  }

  std::vector<TrackingDeviceData> UnspecifiedTrackingTypeInformation::GetTrackingDeviceData()
  {
    TrackingDeviceData DeviceDataUnspecified = { UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(), "Unspecified System", "cube", "X" };
    // Careful when changing the "invalid" device: The mitkTrackingTypeTest is using it's data.
    TrackingDeviceData DeviceDataInvalid = { "Tracking System Invalid", "Invalid Tracking System", "", "X" };

    std::vector<TrackingDeviceData> _TrackingDeviceData;
    _TrackingDeviceData.push_back(DeviceDataUnspecified);
    _TrackingDeviceData.push_back(DeviceDataInvalid);

    return _TrackingDeviceData;
  }

  TrackingDeviceData UnspecifiedTrackingTypeInformation::GetTrackingDeviceData(std::string model)
  {
    for (auto data : GetTrackingDeviceData())
    {
      if (data.Model == model)
        return data;
    }
    return TrackingDeviceData();
  }

  UnspecifiedTrackingTypeInformation::UnspecifiedTrackingTypeInformation()
  {
    m_DeviceName = UnspecifiedTrackingTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData = UnspecifiedTrackingTypeInformation::GetTrackingDeviceData();
    //############## NDI Aurora device data #############
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