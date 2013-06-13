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

void mitk::OverlayManager::PrepareLayout(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = m_BaseRendererMap[renderer];
  if(ls)
  {
  }
}

void mitk::OverlayManager::AddBaseRenderer(mitk::BaseRenderer* renderer)
{
  int nRenderers = m_BaseRendererList.size();
  m_BaseRendererList.remove(renderer);
  m_BaseRendererList.push_back(renderer);

  if(nRenderers < m_BaseRendererList.size())
  {
    std::list<Overlay::Pointer>::iterator it;
    for ( it=m_OverlayList.begin() ; it != m_OverlayList.end(); it++ )
    {
      (*it)->AddOverlay(renderer);
    }
  }
}

void mitk::OverlayManager::AddOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.push_back(overlay);
  BaseRendererList::iterator it;
  for ( it=m_BaseRendererList.begin() ; it != m_BaseRendererList.end(); it++ )
  {
    overlay->AddOverlay((*it));
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
  m_OverlayLayouterMap.clear();
}

void mitk::OverlayManager::LocalStorage::SetLayouterToOverlay(mitk::Overlay::Pointer overlay, mitk::BaseLayouter::Pointer layouter)
{
  mitk::BaseLayouter::Pointer oldLayouter = m_OverlayLayouterMap[overlay.GetPointer()];
  if(oldLayouter.IsNotNull())
  {
    oldLayouter->RemoveOverlay(overlay);
  }
  m_OverlayLayouterMap[overlay.GetPointer()] = layouter;
  layouter->AddOverlay(overlay);
}

mitk::BaseLayouter::Pointer mitk::OverlayManager::LocalStorage::GetLayouter(mitk::Overlay::Pointer overlay)
{
  return m_OverlayLayouterMap[overlay.GetPointer()];
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

void mitk::OverlayManager::SetLayouter(mitk::Overlay::Pointer overlay, mitk::BaseLayouter::Pointer layouter)
{
  LocalStorage* ls = m_BaseRendererMap[layouter->GetBaseRenderer().GetPointer()];
  if(!ls){
    MITK_WARN << "The used renderer is not part of the OverlayManager. Nothing will happen.";
    return;
  }
  ls->SetLayouterToOverlay(overlay,layouter);
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
