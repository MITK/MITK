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

#include "mitkAbstractOverlayLayouter.h"
#include "mitkBaseRenderer.h"

mitk::AbstractOverlayLayouter::AbstractOverlayLayouter()
{
}

mitk::AbstractOverlayLayouter::~AbstractOverlayLayouter()
{
}

std::list<mitk::Overlay*> mitk::AbstractOverlayLayouter::GetManagedOverlays() const
{
  return m_ManagedOverlays;
}

void mitk::AbstractOverlayLayouter::SetBaseRenderer(BaseRenderer *renderer)
{
  m_BaseRenderer = renderer;
}

mitk::BaseRenderer* mitk::AbstractOverlayLayouter::GetBaseRenderer()
{
  return m_BaseRenderer;
}

void mitk::AbstractOverlayLayouter::AddOverlay(mitk::Overlay* Overlay)
{
  if(Overlay->m_LayoutedBy && Overlay->m_LayoutedBy != this)
    Overlay->m_LayoutedBy->RemoveOverlay(Overlay);
  Overlay->m_LayoutedBy = this;
  m_ManagedOverlays.push_back(Overlay);
  PrepareLayout();
}

void mitk::AbstractOverlayLayouter::RemoveOverlay(mitk::Overlay* Overlay)
{
  m_ManagedOverlays.remove(Overlay);
}

std::string mitk::AbstractOverlayLayouter::GetIdentifier() const
{
  return m_Identifier;
}
