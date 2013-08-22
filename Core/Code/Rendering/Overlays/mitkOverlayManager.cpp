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
#include "mitkBaseRenderer.h"

mitk::OverlayManager::OverlayManager()
{
}

mitk::OverlayManager::~OverlayManager()
{
  m_BaseRendererList.clear();
  m_OverlayList.clear();
  LayouterRendererMap::iterator it;
  for ( it=m_LayouterMap.begin() ; it != m_LayouterMap.end(); it++ )
  {
    (it->second).clear();
  }
  m_LayouterMap.clear();
}

void mitk::OverlayManager::AddBaseRenderer(mitk::BaseRenderer* renderer)
{
  size_t nRenderers = m_BaseRendererList.size();
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

void mitk::OverlayManager::AddOverlay(Overlay *overlay)
{
  m_OverlayList.push_back(overlay);
  BaseRendererList::iterator it;
  for ( it=m_BaseRendererList.begin() ; it != m_BaseRendererList.end(); it++ )
  {
    overlay->AddOverlay(*it);
  }
}

void mitk::OverlayManager::RemoveOverlay(Overlay *overlay)
{
  m_OverlayList.remove(overlay);
  BaseRendererList::iterator it;
  for ( it=m_BaseRendererList.begin() ; it != m_BaseRendererList.end(); it++ )
  {
    overlay->RemoveOverlay(*it);
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
  UpdateLayouts(baseRenderer);
}


void mitk::OverlayManager::SetLayouter(Overlay *overlay, const char *identifier, mitk::BaseRenderer *renderer)
{
  if(renderer)
  {
    BaseLayouter::Pointer layouter = GetLayouter(renderer,identifier);
    if(layouter.IsNull())
    {
      MITK_WARN << "Layouter " << identifier << " cannot be found or created!";
      return;
    }
    else
    {
      layouter->AddOverlay(overlay);
    }
  }
}

void mitk::OverlayManager::UpdateLayouts(mitk::BaseRenderer *renderer)
{
  LayouterMap layouters = m_LayouterMap[renderer];
  LayouterMap::iterator it;
  for ( it=layouters.begin() ; it != layouters.end(); it++ )
  {
    (it->second)->PrepareLayout();
  }
}

mitk::BaseLayouter::Pointer mitk::OverlayManager::GetLayouter(mitk::BaseRenderer *renderer, const std::string identifier)
{
  BaseLayouter::Pointer layouter = m_LayouterMap[renderer][identifier];
  return layouter;
}

void mitk::OverlayManager::AddLayouter(BaseLayouter *layouter)
{
  if(layouter)
  {
    BaseLayouter::Pointer oldLayouter = m_LayouterMap[layouter->GetBaseRenderer()][layouter->GetIdentifier()];
    if(oldLayouter.IsNotNull() && oldLayouter.GetPointer() != layouter)
    {
      MITK_WARN << "Layouter " << layouter->GetIdentifier() << " does already exist!";
    }
    else if(oldLayouter.IsNull())
    {
      m_LayouterMap[layouter->GetBaseRenderer()][layouter->GetIdentifier()] = layouter;
    }
  }
}
