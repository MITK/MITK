/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPolhemusTrackerTypeInformation_h
#define mitkPolhemusTrackerTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for Polhemus Tracking Devices.
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT PolhemusTrackerTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    PolhemusTrackerTypeInformation();
    virtual ~PolhemusTrackerTypeInformation();

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataPolhemusTrackerLiberty();
  };
} // namespace mitk

#endif //mitkPolhemusTrackerTypeInformation_h
