/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingDeviceTypeCollection.h"

#include "mitkUnspecifiedTrackingTypeInformation.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>


mitk::TrackingDeviceTypeCollection::TrackingDeviceTypeCollection()
  : m_ServiceRegistration()
  , m_TrackingDeviceTypeInformations()
{
}

mitk::TrackingDeviceTypeCollection::~TrackingDeviceTypeCollection()
{
  std::vector<TrackingDeviceTypeInformation*>::iterator iter = m_TrackingDeviceTypeInformations.begin();

  for (; iter != m_TrackingDeviceTypeInformations.end(); iter++)
  {
    delete (*iter);
  }

  m_TrackingDeviceTypeInformations.clear();
}

void mitk::TrackingDeviceTypeCollection::RegisterAsMicroservice()
{
  us::ModuleContext* context = us::GetModuleContext();

  m_ServiceRegistration = context->RegisterService(this);
}

void mitk::TrackingDeviceTypeCollection::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != nullptr) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
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
    if (deviceType->GetTrackingDeviceName() == type)
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
    if (deviceType->GetTrackingDeviceName() == type)
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
    return mitk::UnspecifiedTrackingTypeInformation::GetDeviceDataInvalid();
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

  return mitk::UnspecifiedTrackingTypeInformation::GetDeviceDataInvalid();
}

std::vector<std::string> mitk::TrackingDeviceTypeCollection::GetTrackingDeviceTypeNames()
{
  std::vector<std::string> names;
  for (auto deviceType : m_TrackingDeviceTypeInformations)
  {
    names.push_back(deviceType->GetTrackingDeviceName());
  }
  return names;
}
