/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingDeviceWidgetCollection.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>

mitk::TrackingDeviceWidgetCollection::TrackingDeviceWidgetCollection()
  : m_ServiceRegistration()
  , m_TrackingDeviceWidgets()
{
}

mitk::TrackingDeviceWidgetCollection::~TrackingDeviceWidgetCollection()
{
  for (auto& item : m_TrackingDeviceWidgets)
  {
    delete item.second;
  }
}

void mitk::TrackingDeviceWidgetCollection::RegisterAsMicroservice()
{
  us::ModuleContext* context = us::GetModuleContext();

  m_ServiceRegistration = context->RegisterService(this);
}

void mitk::TrackingDeviceWidgetCollection::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != nullptr) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

void mitk::TrackingDeviceWidgetCollection::RegisterTrackingDeviceWidget(TrackingDeviceType type, QmitkAbstractTrackingDeviceWidget* widget)
{
  if (widget != nullptr)
  {
    //Don't add widget, if it is already included
    for (unsigned int i = 0; i < m_TrackingDeviceWidgets.size(); i++)
    {
      if (m_TrackingDeviceWidgets.at(i).first == type)
        return;
    }
    m_TrackingDeviceWidgets.push_back(std::make_pair(type, widget));
  }
}

QmitkAbstractTrackingDeviceWidget* mitk::TrackingDeviceWidgetCollection::GetTrackingDeviceWidgetClone(TrackingDeviceType type)
{
  for (unsigned int i = 0; i < m_TrackingDeviceWidgets.size(); i++)
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
