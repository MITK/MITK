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
  RemoveAllOverlays();
}

void mitk::OverlayManager::AddBaseRenderer(mitk::BaseRenderer* renderer)
{
  std::pair<BaseRendererSet::iterator,bool> inSet;
  inSet = m_BaseRendererSet.insert(renderer);
  if(inSet.second)
  {
    OverlaySet::iterator it;
    for ( it=m_OverlaySet.begin() ; it != m_OverlaySet.end(); it++ )
    {
      (*it)->AddToBaseRenderer(renderer);
    }
  }
}

void mitk::OverlayManager::RemoveBaseRenderer(mitk::BaseRenderer* renderer)
{
  if(!renderer)
    return;

  OverlaySet::iterator it;
  for ( it=m_OverlaySet.begin() ; it != m_OverlaySet.end(); it++ )
  {
    (*it)->RemoveFromBaseRenderer(renderer);
  }

  BaseRendererSet::iterator i = m_BaseRendererSet.find(renderer);
  if( i == m_BaseRendererSet.end() )
    return;

  m_BaseRendererSet.erase(i);
}

void mitk::OverlayManager::AddOverlay(const Overlay::Pointer& overlay)
{
  std::pair<OverlaySet::iterator,bool> inSet;
  inSet = m_OverlaySet.insert(overlay);
  if(inSet.second)
  {
    BaseRendererSet::iterator it;
    for ( it=m_BaseRendererSet.begin() ; it != m_BaseRendererSet.end(); it++ )
    {
      overlay->AddToBaseRenderer(*it);
    }
  }
}

void mitk::OverlayManager::AddOverlay(const Overlay::Pointer& overlay, BaseRenderer* renderer)
{
  std::pair<OverlaySet::iterator,bool> inSet;
  inSet = m_OverlaySet.insert(overlay);
  if(inSet.second)
  {
    overlay->AddToBaseRenderer(renderer);
  }
}

void mitk::OverlayManager::RemoveOverlay(const Overlay::Pointer &overlay)
{
  OverlaySet::iterator overlayIt = m_OverlaySet.find(overlay);
  if( overlayIt == m_OverlaySet.end() )
    return;

  BaseRendererSet::iterator it;
  for ( it=m_BaseRendererSet.begin() ; it != m_BaseRendererSet.end(); it++)
  {
    overlay->RemoveFromBaseRenderer(*it);
  }

  m_OverlaySet.erase(overlayIt);
}

void mitk::OverlayManager::RemoveAllOverlays()
{
  while(!m_OverlaySet.empty())
    RemoveOverlay(*m_OverlaySet.begin());
}

void mitk::OverlayManager::UpdateOverlays(mitk::BaseRenderer* baseRenderer)
{
  OverlaySet::iterator it;
  for ( it=m_OverlaySet.begin() ; it != m_OverlaySet.end(); it++ )
  {
    (*it)->Update(baseRenderer);
  }
  UpdateLayouts(baseRenderer);
}

void mitk::OverlayManager::SetLayouter(Overlay *overlay, const std::string &identifier, mitk::BaseRenderer *renderer)
{
  if(renderer)
  {
    AbstractOverlayLayouter::Pointer layouter = GetLayouter(renderer,identifier);
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

mitk::AbstractOverlayLayouter::Pointer mitk::OverlayManager::GetLayouter(mitk::BaseRenderer *renderer, const std::string& identifier)
{
  AbstractOverlayLayouter::Pointer layouter = m_LayouterMap[renderer][identifier];
  return layouter;
}

void mitk::OverlayManager::AddLayouter(const AbstractOverlayLayouter::Pointer& layouter)
{
  if(layouter.IsNotNull())
  {
    AbstractOverlayLayouter::Pointer oldLayouter = m_LayouterMap[layouter->GetBaseRenderer()][layouter->GetIdentifier()];
    if(oldLayouter.IsNotNull() && oldLayouter.GetPointer() != layouter.GetPointer())
    {
      MITK_WARN << "Layouter " << layouter->GetIdentifier() << " does already exist!";
    }
    else if(oldLayouter.IsNull())
    {
      m_LayouterMap[layouter->GetBaseRenderer()][layouter->GetIdentifier()] = layouter;
    }
  }
}