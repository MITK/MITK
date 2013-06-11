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
  BaseRendererLSMap::iterator it;
  for ( it=m_BaseRendererMap.begin() ; it != m_BaseRendererMap.end(); it++ )
  {
    delete (*it).second;
  }
  m_BaseRendererMap.clear();
}

void mitk::OverlayManager::AddBaseRenderer(mitk::BaseRenderer* renderer)
{
  LocalStorage *l = m_BaseRendererMap[ renderer ];
  if(!l)
  {
    l = new LocalStorage;
    m_BaseRendererMap[ renderer ] = l;
  }
}

void mitk::OverlayManager::AddOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.push_back(overlay);
  BaseRendererLSMap::iterator it;
  for ( it=m_BaseRendererMap.begin() ; it != m_BaseRendererMap.end(); it++ )
  {
    overlay->AddOverlay((*it).first);
  }
}

void mitk::OverlayManager::RemoveOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.remove(overlay);
  BaseRendererLSMap::iterator it;
  for ( it=m_BaseRendererMap.begin() ; it != m_BaseRendererMap.end(); it++ )
  {
    overlay->RemoveOverlay((*it).first);
  }
}

void mitk::OverlayManager::RemoveAllOverlays()
{
  std::list<Overlay::Pointer>::iterator it;
  for ( it=m_OverlayList.begin() ; it != m_OverlayList.end(); it++ )
  {
    RemoveOverlay(*it);
  }
  m_OverlayList.clear();
}

void mitk::OverlayManager::UpdateOverlays(mitk::BaseRenderer* baseRenderer)
{
  std::list<Overlay::Pointer>::iterator it;
  for ( it=m_OverlayList.begin() ; it != m_OverlayList.end(); it++ )
  {
    (*it)->UpdateOverlay(baseRenderer);
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
