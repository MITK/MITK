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

#ifndef VIRTUALTRACKERTYPEINFORMATION_H_INCLUDED
#define VIRTUALTRACKERTYPEINFORMATION_H_INCLUDED

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
    virtual ~VirtualTrackerTypeInformation();

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataVirtualTracker();
  };
} // namespace mitk

#endif //VIRTUALTRACKERTYPEINFORMATION_H_INCLUDED
