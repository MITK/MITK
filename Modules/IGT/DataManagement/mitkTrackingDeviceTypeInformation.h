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

#ifndef TRACKINGDEVICETYPEINFORMATION_H_INCLUDED
#define TRACKINGDEVICETYPEINFORMATION_H_INCLUDED

#include <MitkIGTExports.h>

#include <string>

#include "mitkTrackingDevice.h"
#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationToolStorage.h"

namespace mitk
{

//############## other device data ##################
// The following TrackingDeviceData instances don't have TrackingDeviceTypeInformation implementations yet
// static TrackingDeviceData DeviceDataDaVinci = {IntuitiveDaVinci, "Intuitive DaVinci", "IntuitiveDaVinci.stl","X"};
// static TrackingDeviceData DeviceDataMicroBird = {AscensionMicroBird, "Ascension MicroBird", "", "X"};

  class MITKIGT_EXPORT TrackingDeviceTypeInformation
  {
    public:

      //TrackingDeviceTypeInformation();
      virtual ~TrackingDeviceTypeInformation() { }

      virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(
          mitk::TrackingDevice::Pointer trackingDevice,
          mitk::NavigationToolStorage::Pointer navigationTools,
          std::string* errorMessage,
          std::vector<int>* toolCorrespondencesInToolStorage) = 0;

      std::string m_DeviceName;

      // In this vector, all TrackingDeviceData which belong to this type are stored.
      std::vector<TrackingDeviceData> m_TrackingDeviceData;

  };
} // namespace mitk

#endif //TRACKINGDEVICETYPEINFORMATION_H_INCLUDED
