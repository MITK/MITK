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

#include "mitkAbstractAnnotationRenderer.h"
#include "usGetModuleContext.h"
#include <memory>

namespace mitk
{
  const std::string AbstractAnnotationRenderer::US_INTERFACE_NAME = "org.mitk.services.AbstractAnnotationRenderer";
  const std::string AbstractAnnotationRenderer::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
  const std::string AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";

  AbstractAnnotationRenderer::AbstractAnnotationRenderer(const std::string &rendererID, const std::string &arID)
    : Superclass(us::GetModuleContext(),
                 us::LDAPFilter("(&(" + Overlay::US_PROPKEY_RENDERER_ID + "=" + rendererID + ")(" +
                                Overlay::US_PROPKEY_AR_ID + "=" + arID + "))")),
      m_RendererID(rendererID),
      m_ID(arID)
  {
  }

  AbstractAnnotationRenderer::~AbstractAnnotationRenderer() {}
  const std::string AbstractAnnotationRenderer::GetRendererID() const { return m_RendererID; }
  const std::string AbstractAnnotationRenderer::GetID() const { return m_ID; }
  AbstractAnnotationRenderer::TrackedType AbstractAnnotationRenderer::AddingService(
    const AbstractAnnotationRenderer::ServiceReferenceType &reference)
  {
    Overlay *overlay = Superclass::AddingService(reference);
    if (overlay)
    {
      m_OverlayServices.push_back(overlay);
    }
    return overlay;
  }

  void AbstractAnnotationRenderer::ModifiedService(const AbstractAnnotationRenderer::ServiceReferenceType &,
                                                   AbstractAnnotationRenderer::TrackedType /*tracked*/)
  {
  }

  void AbstractAnnotationRenderer::RemovedService(const AbstractAnnotationRenderer::ServiceReferenceType &,
                                                  AbstractAnnotationRenderer::TrackedType tracked)
  {
    //  tracked->RemoveFromBaseRenderer() TODO19786
    m_OverlayServices.erase(std::remove(m_OverlayServices.begin(), m_OverlayServices.end(), tracked),
                            m_OverlayServices.end());
  }
}
