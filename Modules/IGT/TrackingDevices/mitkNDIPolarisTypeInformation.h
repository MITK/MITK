/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNDIPolarisTypeInformation_h
#define mitkNDIPolarisTypeInformation_h

#include "mitkTrackingDeviceTypeInformation.h"

namespace mitk
{
  /** Documentation:
  *   \brief Type information for NDI Polaris
  *
  *   \ingroup IGTUI
  */
  class MITKIGT_EXPORT NDIPolarisTypeInformation : public TrackingDeviceTypeInformation
  {
  public:
    NDIPolarisTypeInformation();
    ~NDIPolarisTypeInformation() override;

    TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice,
      mitk::NavigationToolStorage::Pointer navigationTools,
      std::string* errorMessage,
      std::vector<int>* toolCorrespondencesInToolStorage) override;

    static std::string GetTrackingDeviceName();
    static TrackingDeviceData GetDeviceDataPolarisOldModel();
    static TrackingDeviceData GetDeviceDataSpectraExtendedPyramid();
    static TrackingDeviceData GetDeviceDataPolarisSpectra();
    static TrackingDeviceData GetDeviceDataPolarisVicra();
  };
} // namespace mitk

#endif
