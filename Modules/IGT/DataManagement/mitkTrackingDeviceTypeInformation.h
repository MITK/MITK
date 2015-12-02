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

#endif //TRACKINGDEVICETYPEINFORMATION_H_INCLUDED
