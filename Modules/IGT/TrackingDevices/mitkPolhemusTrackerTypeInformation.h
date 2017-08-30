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
