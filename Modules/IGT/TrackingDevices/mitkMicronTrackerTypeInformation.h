/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMicronTrackerTypeInformation_h
#define mitkMicronTrackerTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for Micron Tracking Devices.
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT MicronTrackerTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    MicronTrackerTypeInformation();
    ~MicronTrackerTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataMicronTrackerH40();
  };
} // namespace mitk

#endif
