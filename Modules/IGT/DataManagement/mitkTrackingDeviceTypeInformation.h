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

//#include <QWidget>
#include "mitkTrackingDevice.h"
#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationToolStorage.h"

namespace mitk
{
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
      //QWidget* m_Widget = nullptr;

  };
} // namespace mitk

#endif //TRACKINGDEVICETYPEINFORMATION_H_INCLUDED
