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

#include "mitkOverlayManager.h"
#include "usGetModuleContext.h"

const std::string mitk::OverlayManager::PROP_ID = "org.mitk.services.OverlayManager.ID";

mitk::OverlayManager::OverlayManager()
{
  m_id = this->RegisterMicroservice();
}

mitk::OverlayManager::~OverlayManager()
{
}

void mitk::OverlayManager::AddOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.push_back(overlay);
}

void mitk::OverlayManager::UpdateOverlays(mitk::BaseRenderer* baseRenderer)
{
  for(int i=0 ; i<m_OverlayList.size() ; i++)
  {
    mitk::Overlay::Pointer overlay = m_OverlayList[i];
    overlay->UpdateOverlay(baseRenderer);
  }
}

mitk::OverlayManager::LocalStorage::~LocalStorage()
{
}

mitk::OverlayManager::LocalStorage::LocalStorage()
{

}

std::string mitk::OverlayManager::RegisterMicroservice()
{
  mitk::ServiceProperties properties;
  static int ID = 0;
  std::string id_str = static_cast<std::ostringstream*>( &(std::ostringstream() << ID) )->str();
  properties[mitk::OverlayManager::PROP_ID] = id_str;
  mitk::ModuleContext* moduleContext = mitk::GetModuleContext();
  m_Registration = moduleContext->RegisterService<mitk::OverlayManager>(this,properties);
  ID++;
  return id_str;
}

void mitk::OverlayManager::UnregisterMicroservice()
{
  if(m_Registration =! NULL)
  {
    m_Registration.Unregister();
  }
}

mitk::OverlayManager::Pointer mitk::OverlayManager::GetServiceInstance(int ID)
{
  std::string id_str = static_cast<std::ostringstream*>( &(std::ostringstream() << ID) )->str();
  mitk::ModuleContext* moduleContext = mitk::GetModuleContext();
  std::string filter = "("+PROP_ID + "="+id_str+")";
  std::list<mitk::ServiceReference> serref = moduleContext->GetServiceReferences("org.mitk.services.OverlayManager",filter);
  if(serref.size()==0)
  {
    return mitk::OverlayManager::New();
  }
  else
  {
  mitk::OverlayManager::Pointer om = moduleContext->GetService<mitk::OverlayManager>(serref.front());
  return om;
  }
}
