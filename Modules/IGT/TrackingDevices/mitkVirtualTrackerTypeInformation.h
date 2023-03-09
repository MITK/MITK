/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVirtualTrackerTypeInformation_h
#define mitkVirtualTrackerTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for a virtual tracker
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT VirtualTrackerTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    VirtualTrackerTypeInformation();
    ~VirtualTrackerTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataVirtualTracker();
  };
} // namespace mitk

#endif
