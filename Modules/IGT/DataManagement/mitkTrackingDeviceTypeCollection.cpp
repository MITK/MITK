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

#include "mitkTrackingDeviceTypeCollection.h"
#include "mitkUIDGenerator.h"

#include "mitkUnspecifiedTrackingTypeInformation.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

const std::string mitk::TrackingDeviceTypeCollection::US_INTERFACE_NAME = "org.mitk.services.TrackingDeviceTypeCollection";
const std::string mitk::TrackingDeviceTypeCollection::US_PROPKEY_DEVICENAME = US_INTERFACE_NAME + ".devicename";
const std::string mitk::TrackingDeviceTypeCollection::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::TrackingDeviceTypeCollection::US_PROPKEY_ISACTIVE = US_INTERFACE_NAME + ".isActive";

mitk::TrackingDeviceTypeCollection::TrackingDeviceTypeCollection()
  : m_TrackingDeviceTypeInformations()
{
}

mitk::TrackingDeviceTypeCollection::~TrackingDeviceTypeCollection()
{
}

void mitk::TrackingDeviceTypeCollection::RegisterAsMicroservice()
{
  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  us::ServiceProperties props;
  mitk::UIDGenerator uidGen = mitk::UIDGenerator ("org.mitk.services.TrackingDeviceTypeCollection.id_", 16);
  props[ US_PROPKEY_ID ] = uidGen.GetUID();
  props[ US_PROPKEY_DEVICENAME ] = m_Name;
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::TrackingDeviceTypeCollection::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != NULL) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

std::string mitk::TrackingDeviceTypeCollection::GetMicroserviceID()
{
  return this->m_ServiceRegistration.GetReference().GetProperty(US_PROPKEY_ID).ToString();
}

void mitk::TrackingDeviceTypeCollection::RegisterTrackingDeviceType(TrackingDeviceTypeInformation* typeInformation)
{
  if (typeInformation != nullptr)
  {
    m_TrackingDeviceTypeInformations.push_back(typeInformation);
  }
}

mitk::TrackingDeviceTypeInformation* mitk::TrackingDeviceTypeCollection::GetTrackingDeviceTypeInformation(mitk::TrackingDeviceType type)
{
  for (auto deviceType : m_TrackingDeviceTypeInformations)
  {
    if (deviceType->m_DeviceName == type)
    {
      return deviceType;
    }
  }

  return nullptr;
}

std::vector<mitk::TrackingDeviceData> mitk::TrackingDeviceTypeCollection::GetDeviceDataForLine(TrackingDeviceType type)
{
  for (auto deviceType : m_TrackingDeviceTypeInformations)
  {
    if (deviceType->m_DeviceName == type)
    {
      return deviceType->m_TrackingDeviceData;
    }
  }

  return std::vector<TrackingDeviceData>();
}

mitk::TrackingDeviceData mitk::TrackingDeviceTypeCollection::GetFirstCompatibleDeviceDataForLine(TrackingDeviceType type)
{
  if (GetDeviceDataForLine(type).empty())
  {
    return mitk::DeviceDataInvalid;
  }

  return GetDeviceDataForLine(type).front();
}

mitk::TrackingDeviceData mitk::TrackingDeviceTypeCollection::GetDeviceDataByName(const std::string& modelName)
{
  for (auto deviceType : m_TrackingDeviceTypeInformations)
  {
    for (auto deviceData : deviceType->m_TrackingDeviceData)
    {
      if (deviceData.Model == modelName)
      {
        return deviceData;
      }
    }
  }

  return mitk::DeviceDataInvalid;
}

std::vector<std::string> mitk::TrackingDeviceTypeCollection::GetTrackingDeviceTypeNames()
{
  std::vector<std::string> names;
  for (auto deviceType : m_TrackingDeviceTypeInformations)
  {
    names.push_back(deviceType->m_DeviceName);
  }
  return names;
}
