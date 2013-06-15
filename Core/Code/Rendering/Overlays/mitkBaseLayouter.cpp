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

#include "mitkBaseLayouter.h"

mitk::BaseLayouter::BaseLayouter()
{
}

mitk::BaseLayouter::~BaseLayouter()
{
}

void mitk::BaseLayouter::SetBaseRenderer(mitk::BaseRenderer::Pointer renderer)
{
  m_BaseRenderer = renderer;
}

mitk::BaseRenderer::Pointer mitk::BaseLayouter::GetBaseRenderer()
{
  return m_BaseRenderer;
}

void mitk::BaseLayouter::AddOverlay(mitk::Overlay::Pointer Overlay)
{
  if(Overlay->m_LayoutedBy && Overlay->m_LayoutedBy != this)
    Overlay->m_LayoutedBy->RemoveOverlay(Overlay);
  Overlay->m_LayoutedBy = this;
  m_ManagedOverlays.push_back(Overlay.GetPointer());
}

void mitk::BaseLayouter::RemoveOverlay(mitk::Overlay::Pointer Overlay)
{
  m_ManagedOverlays.remove(Overlay.GetPointer());
}

std::string mitk::BaseLayouter::GetIdentifier()
{
  return m_Identifier;
}
