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

#include <mitkVtkLayerController.h>

mitk::OverlayManager::OverlayManager()
{
}

mitk::OverlayManager::~OverlayManager()
{
  RemoveAllOverlays();
  RemoveAllBaseRenderers();
}

void mitk::OverlayManager::AddBaseRenderer(mitk::BaseRenderer* renderer)
{
  if(!m_ForegroundRenderer[renderer])
  {
    m_ForegroundRenderer[renderer] = vtkSmartPointer<vtkRenderer>::New();
    vtkRenderer* rendererVtk = m_ForegroundRenderer[renderer];
    rendererVtk->SetActiveCamera(renderer->GetVtkRenderer()->GetActiveCamera());
    mitk::VtkLayerController::GetInstance(renderer->GetRenderWindow())->InsertForegroundRenderer(rendererVtk,false);
    rendererVtk->SetInteractive(false);
  }
  std::pair<BaseRendererSet::iterator,bool> inSet;
  inSet = m_BaseRendererSet.insert(renderer);
  if(inSet.second)
  {
    OverlaySet::iterator it;
    for ( it=m_OverlaySet.begin() ; it != m_OverlaySet.end(); it++ )
    {
      if((*it)->IsForceInForeground())
        (*it)->AddToRenderer(renderer,m_ForegroundRenderer[renderer]);
        else
        (*it)->AddToBaseRenderer(renderer);
    }
  }
}

void mitk::OverlayManager::RemoveBaseRenderer(mitk::BaseRenderer* renderer)
{
  if(!renderer)
    return;

  vtkRenderer* forgroundRenderer = m_ForegroundRenderer[renderer];
  OverlaySet::iterator it;
  for ( it=m_OverlaySet.begin() ; it != m_OverlaySet.end(); it++ )
  {
    (*it)->RemoveFromBaseRenderer(renderer);
    if(forgroundRenderer)
      (*it)->RemoveFromRenderer(renderer,forgroundRenderer);
  }

  BaseRendererSet::iterator i = m_BaseRendererSet.find(renderer);
  if( i == m_BaseRendererSet.end() )
    return;

  m_BaseRendererSet.erase(i);

  m_ForegroundRenderer[renderer] = NULL;
}

void mitk::OverlayManager::RemoveAllBaseRenderers()
{
  BaseRendererSet::iterator it;
  for ( it=m_BaseRendererSet.begin() ; it != m_BaseRendererSet.end(); it++)
  {
    this->RemoveBaseRenderer(*it);
  }
}

void mitk::OverlayManager::AddOverlay(const Overlay::Pointer& overlay, bool ForceInForeground)
{
  std::pair<OverlaySet::iterator,bool> inSet;
  inSet = m_OverlaySet.insert(overlay);
  if(inSet.second)
  {
    BaseRendererSet::iterator it;
    for ( it=m_BaseRendererSet.begin() ; it != m_BaseRendererSet.end(); it++ )
    {
      if(ForceInForeground)
      {
        vtkRenderer* forgroundRenderer = m_ForegroundRenderer[*it];
        overlay->AddToRenderer(*it,forgroundRenderer);
      }
      else
        overlay->AddToBaseRenderer(*it);
    }
  }
}

void mitk::OverlayManager::AddOverlay(const Overlay::Pointer& overlay, BaseRenderer* renderer, bool ForceInForeground)
{
  std::pair<OverlaySet::iterator,bool> inSet;
  inSet = m_OverlaySet.insert(overlay);
  if(inSet.second)
  {
    if(ForceInForeground)
    {
      vtkRenderer* forgroundRenderer = m_ForegroundRenderer[renderer];
      overlay->AddToRenderer(renderer,forgroundRenderer);
    }
    else
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
    vtkRenderer* forgroundRenderer = m_ForegroundRenderer[*it];
    if(forgroundRenderer)
      overlay->RemoveFromRenderer(*it,forgroundRenderer);
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
