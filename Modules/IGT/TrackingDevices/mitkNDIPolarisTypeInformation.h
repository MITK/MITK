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

#ifndef NDIPOLARISTYPEINFORMATION_H_INCLUDED
#define NDIPOLARISTYPEINFORMATION_H_INCLUDED

#include <MitkIGTExports.h>

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{

  //############## NDI Polaris device data ############
  static TrackingDeviceData DeviceDataPolarisOldModel = {TRACKING_DEVICE_IDENTIFIER_POLARIS, "Polaris (Old Model)", "NDIPolarisOldModel.stl", "0"};
  //full hardware code of polaris spectra: 5-240000-153200-095000+057200+039800+056946+024303+029773+999999+99999924
  static TrackingDeviceData DeviceDataPolarisSpectra = {TRACKING_DEVICE_IDENTIFIER_POLARIS, "Polaris Spectra", "NDIPolarisSpectra.stl", "5-2"};
  //full hardware code of polaris spectra (extended pyramid): 5-300000-153200-095000+057200+039800+056946+024303+029773+999999+07350024
  static TrackingDeviceData DeviceDataSpectraExtendedPyramid = {TRACKING_DEVICE_IDENTIFIER_POLARIS, "Polaris Spectra (Extended Pyramid)", "NDIPolarisSpectraExtendedPyramid.stl","5-3"};
  static TrackingDeviceData DeviceDataPolarisVicra = {TRACKING_DEVICE_IDENTIFIER_POLARIS, "Polaris Vicra", "NDIPolarisVicra.stl","7"};

  class MITKIGT_EXPORT NDIPolarisTypeInformation : public TrackingDeviceTypeInformation
  {
    public:
      NDIPolarisTypeInformation();
      virtual ~NDIPolarisTypeInformation();

      virtual TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
                                                                       mitk::NavigationToolStorage::Pointer navigationTools,
                                                                       std::string* errorMessage,
                                                                       std::vector<int>* toolCorrespondencesInToolStorage) override;
  };
} // namespace mitk

#endif //NDIPOLARISTYPEINFORMATION_H_INCLUDED
