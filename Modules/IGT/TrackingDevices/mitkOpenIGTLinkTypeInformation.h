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
    virtual ~OpenIGTLinkTypeInformation();

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage, std::vector<int>*) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataOpenIGTLinkTrackingDeviceConnection();
  };
} // namespace mitk

#endif //mitkOpenIGTLinkTypeInformation_h
