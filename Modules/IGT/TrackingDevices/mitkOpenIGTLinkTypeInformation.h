/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOpenIGTLinkTypeInformation_h
#define mitkOpenIGTLinkTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for tracking devices using OpenIGTLink
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT OpenIGTLinkTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    OpenIGTLinkTypeInformation();
    ~OpenIGTLinkTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage, std::vector<int>*) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataOpenIGTLinkTrackingDeviceConnection();
  };
} // namespace mitk

#endif //mitkOpenIGTLinkTypeInformation_h
