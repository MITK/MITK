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
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "usGetModuleContext.h"
#include <memory>

namespace mitk
{
  const std::string AbstractAnnotationRenderer::US_INTERFACE_NAME = "org.mitk.services.AbstractAnnotationRenderer";
  const std::string AbstractAnnotationRenderer::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
  const std::string AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";

  AbstractAnnotationRenderer::AbstractAnnotationRenderer(const std::string &rendererID, const std::string &arID)
    : Superclass(us::GetModuleContext(),
                 us::LDAPFilter("(&(" + Annotation::US_PROPKEY_RENDERER_ID + "=" + rendererID + ")(" +
                                Annotation::US_PROPKEY_AR_ID + "=" + arID + "))")),
      m_RendererID(rendererID),
      m_ID(arID)
  {
    this->Open();
  }

  AbstractAnnotationRenderer::~AbstractAnnotationRenderer() {}
  const std::string AbstractAnnotationRenderer::GetRendererID() const { return m_RendererID; }
  void AbstractAnnotationRenderer::CurrentBaseRendererChanged()
  {
    BaseRenderer *renderer = GetCurrentBaseRenderer();
    if (renderer)
    {
      for (Annotation *o : this->GetServices())
      {
        o->AddToBaseRenderer(renderer);
      }
    }
  }

  void AbstractAnnotationRenderer::RemoveAllAnnotation()
  {
    BaseRenderer *renderer = GetCurrentBaseRenderer();
    if (renderer)
    {
      for (Annotation *o : this->GetServices())
      {
        o->RemoveFromBaseRenderer(renderer);
      }
    }
  }

  void AbstractAnnotationRenderer::Update()
  {
    BaseRenderer *renderer = GetCurrentBaseRenderer();
    if (renderer)
    {
      for (Annotation *o : this->GetServices())
      {
        o->Update(renderer);
      }
    }
  }
  const std::string AbstractAnnotationRenderer::GetID() const { return m_ID; }
  AbstractAnnotationRenderer::TrackedType AbstractAnnotationRenderer::AddingService(
    const AbstractAnnotationRenderer::ServiceReferenceType &reference)
  {
    Annotation *Annotation = Superclass::AddingService(reference);
    BaseRenderer *renderer = GetCurrentBaseRenderer();
    if (Annotation && renderer)
    {
      Annotation->AddToBaseRenderer(renderer);
    }
    //    OnAnnotationRenderersChanged();
    return Annotation;
  }

  void AbstractAnnotationRenderer::ModifiedService(const AbstractAnnotationRenderer::ServiceReferenceType &,
                                                   AbstractAnnotationRenderer::TrackedType tracked)
  {
    BaseRenderer *renderer = GetCurrentBaseRenderer();
    if (tracked && renderer)
    {
      tracked->Update(renderer);
    }
    OnAnnotationRenderersChanged();
  }

  void AbstractAnnotationRenderer::RemovedService(const AbstractAnnotationRenderer::ServiceReferenceType &,
                                                  AbstractAnnotationRenderer::TrackedType /*tracked*/)
  {
    OnAnnotationRenderersChanged();
  }

  BaseRenderer *AbstractAnnotationRenderer::GetCurrentBaseRenderer()
  {
    return BaseRenderer::GetByName(this->GetRendererID());
  }
}
