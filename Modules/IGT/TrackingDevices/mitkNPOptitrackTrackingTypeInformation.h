/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNPOptitrackTrackingTypeInformation_h
#define mitkNPOptitrackTrackingTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for NP Optitrack
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT NPOptitrackTrackingTypeInformation : public TrackingDeviceTypeInformation
  {
  public:

    NPOptitrackTrackingTypeInformation();
    ~NPOptitrackTrackingTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(TrackingDevice::Pointer trackingDevice,
      NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataNPOptitrack();
  };
} // namespace mitk

#endif
