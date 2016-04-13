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
    virtual ~UnspecifiedTrackingTypeInformation();

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer,
      mitk::NavigationToolStorage::Pointer,
      std::string*,
      std::vector<int>*);

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataUnspecified();
    static TrackingDeviceData GetDeviceDataInvalid();
  };
} // namespace mitk

#endif //mitkUnspecifiedTrackingTypeInformation_h
