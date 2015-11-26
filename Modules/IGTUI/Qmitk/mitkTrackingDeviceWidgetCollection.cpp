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

#include "mitkTrackingDeviceWidgetCollection.h"
#include "mitkUIDGenerator.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

const std::string mitk::TrackingDeviceWidgetCollection::US_INTERFACE_NAME = "org.mitk.services.TrackingDeviceWidgetCollection";
const std::string mitk::TrackingDeviceWidgetCollection::US_PROPKEY_DEVICENAME = US_INTERFACE_NAME + ".devicename";
const std::string mitk::TrackingDeviceWidgetCollection::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::TrackingDeviceWidgetCollection::US_PROPKEY_ISACTIVE = US_INTERFACE_NAME + ".isActive";

mitk::TrackingDeviceWidgetCollection::TrackingDeviceWidgetCollection()
  : m_TrackingDeviceWidgets()
{
}

mitk::TrackingDeviceWidgetCollection::~TrackingDeviceWidgetCollection()
{
}

void mitk::TrackingDeviceWidgetCollection::RegisterAsMicroservice()
{
  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  us::ServiceProperties props;
  mitk::UIDGenerator uidGen = mitk::UIDGenerator("org.mitk.services.TrackingDeviceWidgetCollection.id_", 16);
  props[US_PROPKEY_ID] = uidGen.GetUID();
  props[US_PROPKEY_DEVICENAME] = m_Name;
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::TrackingDeviceWidgetCollection::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != NULL) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

std::string mitk::TrackingDeviceWidgetCollection::GetMicroserviceID()
{
  return this->m_ServiceRegistration.GetReference().GetProperty(US_PROPKEY_ID).ToString();
}

void mitk::TrackingDeviceWidgetCollection::RegisterTrackingDeviceWidget(TrackingDeviceType type, QmitkAbstractTrackingDeviceWidget* widget)
{
  if (widget != nullptr)
  {
    m_TrackingDeviceWidgets.push_back(std::make_pair(type, widget));
  }
}

QmitkAbstractTrackingDeviceWidget* mitk::TrackingDeviceWidgetCollection::GetTrackingDeviceWidget(TrackingDeviceType type)
{
  for (int i = 0; i < m_TrackingDeviceWidgets.size(); i++)
  {
    if (m_TrackingDeviceWidgets.at(i).first == type)
      return m_TrackingDeviceWidgets.at(i).second;
  }
  return nullptr;
}