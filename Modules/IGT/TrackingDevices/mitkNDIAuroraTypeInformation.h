/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNDIAuroraTypeInformation_h
#define mitkNDIAuroraTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for NDI Aurora
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT NDIAuroraTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    NDIAuroraTypeInformation();
    ~NDIAuroraTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();

    static TrackingDeviceData GetDeviceDataAuroraCompact();
    static TrackingDeviceData GetDeviceDataAuroraPlanarCube();
    static TrackingDeviceData GetDeviceDataAuroraPlanarDome();
    static TrackingDeviceData GetDeviceDataAuroraTabletop();
  };
} // namespace mitk

#endif
