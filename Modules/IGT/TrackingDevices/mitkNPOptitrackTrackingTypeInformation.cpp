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

#include "mitkNPOptitrackTrackingTypeInformation.h"

#include "mitkOptitrackTrackingDevice.h"

namespace mitk
{
  std::string NPOptitrackTrackingTypeInformation::GetTrackingDeviceName()
  {
    return "NP Optitrack";
  }

  TrackingDeviceData NPOptitrackTrackingTypeInformation::GetDeviceDataNPOptitrack()
  {
    TrackingDeviceData data = { NPOptitrackTrackingTypeInformation::GetTrackingDeviceName(), "Optitrack", "cube", "X" };
    return data;
  }

  NPOptitrackTrackingTypeInformation::NPOptitrackTrackingTypeInformation()
  {
    m_DeviceName = NPOptitrackTrackingTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataNPOptitrack());
  }

  NPOptitrackTrackingTypeInformation::~NPOptitrackTrackingTypeInformation()
  {
  }

  TrackingDeviceSource::Pointer NPOptitrackTrackingTypeInformation::CreateTrackingDeviceSource(
    TrackingDevice::Pointer trackingDevice,
    NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::OptitrackTrackingDevice::Pointer thisDevice = dynamic_cast<mitk::OptitrackTrackingDevice*>(trackingDevice.GetPointer());
    *toolCorrespondencesInToolStorage = std::vector<int>();

    //OpenConnection with Optitrack
    thisDevice->OpenConnection();

    //add the tools to the tracking device
    for (int i = 0; i < navigationTools->GetToolCount(); i++)
    {
      mitk::NavigationTool::Pointer thisNavigationTool = navigationTools->GetTool(i);
      toolCorrespondencesInToolStorage->push_back(i);
      bool toolAddSuccess = thisDevice->AddToolByDefinitionFile(thisNavigationTool->GetCalibrationFile());
      thisDevice->GetOptitrackTool(i)->SetToolName(thisNavigationTool->GetToolName().c_str());
      if (!toolAddSuccess)
      {
        //todo error handling
        errorMessage->append("Can't add tool, is the toolfile valid?");
        return NULL;
      }
      //thisDevice->GetTool(i)->SetToolTip(thisNavigationTool->GetToolTipPosition(),thisNavigationTool->GetToolTipOrientation());
    }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}