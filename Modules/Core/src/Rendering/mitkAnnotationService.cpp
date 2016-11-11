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

#include "mitkAnnotationService.h"
#include "mitkOverlay.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include <mitkAbstractAnnotationRenderer.h>

namespace mitk
{
  AnnotationService::AnnotationService() {}
  AnnotationService::~AnnotationService() {}
  AbstractAnnotationRenderer *AnnotationService::GetAnnotationRenderer(const std::string &arTypeID,
                                                                       const std::string &rendererID)
  {
    // get the context
    us::ModuleContext *context = us::GetModuleContext();

    // specify a filter that defines the requested type
    std::string filter = "(&(" + AbstractAnnotationRenderer::US_PROPKEY_ID + "=" + arTypeID + ")(" +
                         AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID + "=" + rendererID + "))";
    // find the fitting service
    std::vector<us::ServiceReferenceU> serviceReferences =
      context->GetServiceReferences(AbstractAnnotationRenderer::US_INTERFACE_NAME, filter);

    // check if a service reference was found. It is also possible that several
    // services were found. This is not checked here, just the first one is taken.
    AbstractAnnotationRenderer *ar = nullptr;
    if (serviceReferences.size())
    {
      ar = context->GetService<AbstractAnnotationRenderer>(serviceReferences.front());
    }
    // no service reference was found or found service reference has no valid source
    return ar;
  }

  void AnnotationService::RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer)
  {
    static AnnotationRendererServices AnnotationRendererServices;
    // Define ServiceProps
    us::ServiceProperties props;
    props[AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID] = annotationRenderer->GetRendererID();
    props[AbstractAnnotationRenderer::US_PROPKEY_ID] = annotationRenderer->GetID();

    us::GetModuleContext()->RegisterService(annotationRenderer, props);
    AnnotationRendererServices.push_back(std::unique_ptr<AbstractAnnotationRenderer>(annotationRenderer));
  }

  void AnnotationService::UpdateAnnotationRenderer(const std::string &rendererID)
  {
    for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(rendererID))
    {
      annotationRenderer->Update();
    }
  }

  void AnnotationService::BaseRendererChanged(BaseRenderer *renderer)
  {
    for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(renderer->GetName()))
    {
      annotationRenderer->CurrentBaseRendererChanged();
    }
    vtkCallbackCommand *renderCallbackCommand = vtkCallbackCommand::New();
    renderCallbackCommand->SetCallback(AnnotationService::RenderWindowCallback);
    renderer->GetRenderWindow()->AddObserver(vtkCommand::ModifiedEvent, renderCallbackCommand);
    renderCallbackCommand->Delete();
  }

  void AnnotationService::RenderWindowCallback(vtkObject *caller, unsigned long, void *, void *)
  {
    vtkRenderWindow *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);
    if (!renderWindow)
      return;
    BaseRenderer *renderer = BaseRenderer::GetInstance(renderWindow);
    for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(renderer->GetName()))
    {
      annotationRenderer->OnRenderWindowModified();
    }
  }

  Overlay *AnnotationService::GetOverlay(const std::string &overlayID)
  {
    std::string ldapFilter = "(" + Overlay::US_PROPKEY_ID + "=" + overlayID + ")";
    us::ModuleContext *context = us::GetModuleContext();
    std::vector<us::ServiceReference<mitk::Overlay>> overlays =
      context->GetServiceReferences<mitk::Overlay>(ldapFilter);
    Overlay *overlay = nullptr;
    if (!overlays.empty())
    {
      overlay = us::GetModuleContext()->GetService<mitk::Overlay>(overlays.front());
    }
    return overlay;
  }

  std::vector<AbstractAnnotationRenderer *> AnnotationService::GetAnnotationRenderer(const std::string &rendererID)
  {
    us::ModuleContext *context = us::GetModuleContext();

    // specify a filter that defines the requested type
    std::string filter = "(&(" + AbstractAnnotationRenderer::US_PROPKEY_ID + "=*)(" +
                         AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID + "=" + rendererID + "))";
    // find the fitting service
    std::vector<us::ServiceReferenceU> serviceReferences =
      context->GetServiceReferences(AbstractAnnotationRenderer::US_INTERFACE_NAME, filter);
    std::vector<AbstractAnnotationRenderer *> arList;
    for (us::ServiceReferenceU service : serviceReferences)
    {
      arList.push_back(context->GetService<AbstractAnnotationRenderer>(service));
    }
    return arList;
  }
}
