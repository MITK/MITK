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

#ifndef mitkNDIAuroraTypeInformation_h
#define mitkNDIAuroraTypeInformation_h

#include <MitkIGTExports.h>

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  class MITKIGT_EXPORT NDIPAuroraTypeInformation : public TrackingDeviceTypeInformation
  //############## NDI Aurora device data #############
  static TrackingDeviceData DeviceDataAuroraCompact = {TRACKING_DEVICE_IDENTIFIER_AURORA, "Aurora Compact", "NDIAuroraCompactFG_Dome.stl", "A"};
  static TrackingDeviceData DeviceDataAuroraPlanarCube = {TRACKING_DEVICE_IDENTIFIER_AURORA, "Aurora Planar (Cube)", "NDIAurora.stl", "9"};
  static TrackingDeviceData DeviceDataAuroraPlanarDome = {TRACKING_DEVICE_IDENTIFIER_AURORA, "Aurora Planar (Dome)","NDIAuroraPlanarFG_Dome.stl", "A"};
  static TrackingDeviceData DeviceDataAuroraTabletop = {TRACKING_DEVICE_IDENTIFIER_AURORA, "Aurora Tabletop", "NDIAuroraTabletopFG_Dome.stl", "A"};
  // The following entry is for the tabletop prototype, which had an lower barrier of 8cm. The new version has a lower barrier of 12cm.
  //static TrackingDeviceData DeviceDataAuroraTabletopPrototype = {NDIAurora, "Aurora Tabletop Prototype", "NDIAuroraTabletopFG_Prototype_Dome.stl"};

  {
    public:
      NDIPAuroraTypeInformation();
      virtual ~NDIPAuroraTypeInformation();

      virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
                                                                       mitk::NavigationToolStorage::Pointer navigationTools,
                                                                       std::string* errorMessage,
                                                                       std::vector<int>* toolCorrespondencesInToolStorage) override;
  };
} // namespace mitk

#endif //mitkNDIAuroraTypeInformation_h
