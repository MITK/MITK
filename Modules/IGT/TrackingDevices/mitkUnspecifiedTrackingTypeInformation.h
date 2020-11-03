/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnspecifiedTrackingTypeInformation_h
#define mitkUnspecifiedTrackingTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for unspecified or invalid tracking devices. This is often used as default or for testing.
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT UnspecifiedTrackingTypeInformation : public TrackingDeviceTypeInformation
  {
  public:

    UnspecifiedTrackingTypeInformation();
    ~UnspecifiedTrackingTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer,
      mitk::NavigationToolStorage::Pointer,
      std::string*,
      std::vector<int>*) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataUnspecified();
    static TrackingDeviceData GetDeviceDataInvalid();
  };
} // namespace mitk

#endif //mitkUnspecifiedTrackingTypeInformation_h
