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
    virtual ~NPOptitrackTrackingTypeInformation();

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(TrackingDevice::Pointer trackingDevice,
      NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataNPOptitrack();
  };
} // namespace mitk

#endif //mitkNPOptitrackTrackingTypeInformation_h
