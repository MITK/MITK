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

#ifndef mitkUnspecifiedTrackingTypeInformation_h
#define mitkUnspecifiedTrackingTypeInformation_h

#include <MitkIGTExports.h>

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  static TrackingDeviceData DeviceDataUnspecified = {mitk::TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED, "Unspecified System", "cube","X"};
  // Careful when changing the "invalid" device: The mitkTrackingTypeTest is using it's data.
  static TrackingDeviceData DeviceDataInvalid = {mitk::TRACKING_DEVICE_IDENTIFIER_INVALID, "Invalid Tracking System", "", "X"};

  class MITKIGT_EXPORT UnspecifiedTrackingTypeInformation : public TrackingDeviceTypeInformation
  {
    public:

      UnspecifiedTrackingTypeInformation()
      {
        m_DeviceName = mitk::TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED;
        m_TrackingDeviceData.push_back(DeviceDataUnspecified);
        m_TrackingDeviceData.push_back(DeviceDataInvalid);
      };
      virtual ~UnspecifiedTrackingTypeInformation(){};

      virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer,
                                                                       mitk::NavigationToolStorage::Pointer,
                                                                       std::string*,
                                                                       std::vector<int>*) { return nullptr; }
  };
} // namespace mitk

#endif //mitkUnspecifiedTrackingTypeInformation_h
