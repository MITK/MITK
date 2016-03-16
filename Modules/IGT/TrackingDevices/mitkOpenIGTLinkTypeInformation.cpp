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

#include "mitkOpenIGTLinkTypeInformation.h"

#include "mitkOpenIGTLinkTrackingDevice.h"

namespace mitk
{
  std::string OpenIGTLinkTypeInformation::GetTrackingDeviceName()
  {
    return "Open IGT Link Connection";
  }

  TrackingDeviceData OpenIGTLinkTypeInformation::GetDeviceDataOpenIGTLinkTrackingDeviceConnection(){
    TrackingDeviceData data = { OpenIGTLinkTypeInformation::GetTrackingDeviceName(), "OpenIGTLink Tracking Device", "cube", "X" };
    return data;
  }

  OpenIGTLinkTypeInformation::OpenIGTLinkTypeInformation()
  {
    m_DeviceName = OpenIGTLinkTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataOpenIGTLinkTrackingDeviceConnection());
  }

  OpenIGTLinkTypeInformation::~OpenIGTLinkTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer OpenIGTLinkTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* /*toolCorrespondencesInToolStorage*/)
  {
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::OpenIGTLinkTrackingDevice::Pointer thisDevice = dynamic_cast<mitk::OpenIGTLinkTrackingDevice*>(trackingDevice.GetPointer());
    thisDevice->DiscoverTools();
    if (thisDevice->GetToolCount() != navigationTools->GetToolCount())
    {
      errorMessage->append("The number of tools in the connected device differs from the tool storage, cannot add tools.");
      return NULL;
    }
    returnValue->SetTrackingDevice(thisDevice);
    return returnValue;
  }
}