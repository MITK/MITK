/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTrackingDeviceTypeInformation_h
#define mitkTrackingDeviceTypeInformation_h

#include <MitkIGTExports.h>

#include <string>

#include "mitkTrackingDevice.h"
#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationToolStorage.h"

namespace mitk
{
  /** Documentation:
  *   \brief Abstract class containing
  *          - The name of your tracking device (which is used as unique identifier in many plugins etc.)
  *          - Information about how to create the tracking device source for your device.
  *
  *   More information on how to implement your own tracking device can be found here: \ref IGTHowToImplementATrackingDevice
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT TrackingDeviceTypeInformation
  {
  public:

    virtual ~TrackingDeviceTypeInformation() { }

    virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(
      mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) = 0;

    std::string GetTrackingDeviceName(){ return m_DeviceName; }

    // In this vector, all TrackingDeviceData which belong to this type are stored.
    std::vector<TrackingDeviceData> m_TrackingDeviceData;

  protected:
    std::string m_DeviceName;
  };
} // namespace mitk

#endif
