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

  m_ServiceRegistration = context->RegisterService(this);
}

void mitk::TrackingDeviceWidgetCollection::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != NULL) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

void mitk::TrackingDeviceWidgetCollection::RegisterTrackingDeviceWidget(TrackingDeviceType type, QmitkAbstractTrackingDeviceWidget* widget)
{
  if (widget != nullptr)
  {
    //Don't add widget, if it is already included
    for (int i = 0; i < m_TrackingDeviceWidgets.size(); i++)
    {
      if (m_TrackingDeviceWidgets.at(i).first == type)
        return;
    }
    m_TrackingDeviceWidgets.push_back(std::make_pair(type, widget));
  }
}

QmitkAbstractTrackingDeviceWidget* mitk::TrackingDeviceWidgetCollection::GetTrackingDeviceWidgetClone(TrackingDeviceType type)
{
  for (int i = 0; i < m_TrackingDeviceWidgets.size(); i++)
  {
    if (m_TrackingDeviceWidgets.at(i).first == type)
    {
      if (!(m_TrackingDeviceWidgets.at(i).second->IsInitialized()))
      {
        m_TrackingDeviceWidgets.at(i).second->Initialize();
        if (!(m_TrackingDeviceWidgets.at(i).second->IsInitialized())) //still not initialized?
          MITK_ERROR << "Something went wrong with initialization of your tracking device widget!";
      }
      return (m_TrackingDeviceWidgets.at(i).second->CloneForQt());
    }
  }
  return nullptr;
}