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
  NDIPolarisTypeInformation::NDIPolarisTypeInformation()
    //: m_DeviceName("Polaris")
    //, m_Widget(nullptr)
  {
    m_DeviceName = "Polaris";
    m_TrackingDeviceData.push_back(DeviceDataPolarisOldModel);
    m_TrackingDeviceData.push_back(DeviceDataPolarisSpectra);
    m_TrackingDeviceData.push_back(DeviceDataSpectraExtendedPyramid);
    m_TrackingDeviceData.push_back(DeviceDataPolarisVicra);
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

