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

#include "mitkNDIPolarisTypeInformation.h"

#include "mitkNDITrackingDevice.h"

namespace mitk
{
  std::string NDIPolarisTypeInformation::GetTrackingDeviceName()
  {
    return "NDI Polaris"; ///< Polaris: optical Tracker from NDI;
  }


  std::vector<TrackingDeviceData> NDIPolarisTypeInformation::GetTrackingDeviceData()
  {

    //############## NDI Polaris device data ############
    TrackingDeviceData DeviceDataPolarisOldModel = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris (Old Model)", "NDIPolarisOldModel.stl", "0" };
    //full hardware code of polaris spectra: 5-240000-153200-095000+057200+039800+056946+024303+029773+999999+99999924
    TrackingDeviceData DeviceDataPolarisSpectra = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Spectra", "NDIPolarisSpectra.stl", "5-2" };
    //full hardware code of polaris spectra (extended pyramid): 5-300000-153200-095000+057200+039800+056946+024303+029773+999999+07350024
    TrackingDeviceData DeviceDataSpectraExtendedPyramid = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Spectra (Extended Pyramid)", "NDIPolarisSpectraExtendedPyramid.stl", "5-3" };
    TrackingDeviceData DeviceDataPolarisVicra = { NDIPolarisTypeInformation::GetTrackingDeviceName(), "Polaris Vicra", "NDIPolarisVicra.stl", "7" };


    std::vector<TrackingDeviceData> _TrackingDeviceData;
    _TrackingDeviceData.push_back(DeviceDataPolarisOldModel);
    _TrackingDeviceData.push_back(DeviceDataSpectraExtendedPyramid);
    _TrackingDeviceData.push_back(DeviceDataPolarisSpectra);
    _TrackingDeviceData.push_back(DeviceDataPolarisVicra);

    return _TrackingDeviceData;
  }

  TrackingDeviceData NDIPolarisTypeInformation::GetTrackingDeviceData(std::string model)
  {
    for (auto data : GetTrackingDeviceData())
    {
      if (data.Model == model)
        return data;
    }
    return TrackingDeviceData();
  }


  NDIPolarisTypeInformation::NDIPolarisTypeInformation()
    //: m_DeviceName("Polaris")
    //, m_Widget(nullptr)
  {
    m_DeviceName = NDIPolarisTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData = NDIPolarisTypeInformation::GetTrackingDeviceData();
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
    for (int i=0; i<navigationTools->GetToolCount(); i++)
        {
          mitk::NavigationTool::Pointer thisNavigationTool = navigationTools->GetTool(i);
          toolCorrespondencesInToolStorage->push_back(i);
          bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(),thisNavigationTool->GetCalibrationFile().c_str());
          if (!toolAddSuccess)
            {
            //todo: error handling
            errorMessage->append("Can't add tool, is the SROM-file valid?");
            return NULL;
            }
          thisDevice->GetTool(i)->SetToolTip(thisNavigationTool->GetToolTipPosition(),thisNavigationTool->GetToolTipOrientation());
        }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}

