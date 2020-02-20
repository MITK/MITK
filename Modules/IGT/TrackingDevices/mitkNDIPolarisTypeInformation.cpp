/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNDIPolarisTypeInformation.h"

#include "mitkNDITrackingDevice.h"

namespace mitk
{
  std::string NDIPolarisTypeInformation::GetTrackingDeviceName()
  {
    return "NDI Polaris"; ///< Polaris: optical Tracker from NDI;
  }
  TrackingDeviceData NDIPolarisTypeInformation::GetDeviceDataPolarisOldModel()
  {
    TrackingDeviceData data = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris (Old Model)", "NDIPolarisOldModel.stl", "0" };
    return data;
  }
  TrackingDeviceData NDIPolarisTypeInformation::GetDeviceDataPolarisSpectra()
  {
    //full hardware code of polaris spectra: 5-240000-153200-095000+057200+039800+056946+024303+029773+999999+99999924
    TrackingDeviceData data = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Spectra", "NDIPolarisSpectra.stl", "5-2" };
    return data;
  }

  TrackingDeviceData NDIPolarisTypeInformation::GetDeviceDataSpectraExtendedPyramid()
  {
    //full hardware code of polaris spectra (extended pyramid): 5-300000-153200-095000+057200+039800+056946+024303+029773+999999+07350024
    TrackingDeviceData data = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Spectra (Extended Pyramid)", "NDIPolarisSpectraExtendedPyramid.stl", "5-3" };
    return data;
  }
  TrackingDeviceData NDIPolarisTypeInformation::GetDeviceDataPolarisVicra()
  {
    TrackingDeviceData data = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Vicra", "NDIPolarisVicra.stl", "7" };
    return data;
  }

  NDIPolarisTypeInformation::NDIPolarisTypeInformation()
  {
    m_DeviceName = NDIPolarisTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataPolarisOldModel());
    m_TrackingDeviceData.push_back(GetDeviceDataSpectraExtendedPyramid());
    m_TrackingDeviceData.push_back(GetDeviceDataPolarisSpectra());
    m_TrackingDeviceData.push_back(GetDeviceDataPolarisVicra());
  }

  NDIPolarisTypeInformation::~NDIPolarisTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer NDIPolarisTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::NDITrackingDevice::Pointer thisDevice = dynamic_cast<mitk::NDITrackingDevice*>(trackingDevice.GetPointer());
    *toolCorrespondencesInToolStorage = std::vector<int>();
    //add the tools to the tracking device
    for (unsigned int i = 0; i < navigationTools->GetToolCount(); i++)
    {
      mitk::NavigationTool::Pointer thisNavigationTool = navigationTools->GetTool(i);
      toolCorrespondencesInToolStorage->push_back(i);
      bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(), thisNavigationTool->GetCalibrationFile().c_str());
      if (!toolAddSuccess)
      {
        //todo: error handling
        errorMessage->append("Can't add tool, is the SROM-file valid?");
        return nullptr;
      }
      thisDevice->GetTool(i)->SetToolTipPosition(thisNavigationTool->GetToolTipPosition(), thisNavigationTool->GetToolAxisOrientation());
    }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}
