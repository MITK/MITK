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

#include "mitkAnnotationUtils.h"
#include "mitkAnnotation.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include <mitkAbstractAnnotationRenderer.h>

namespace mitk
{
  AnnotationUtils::AnnotationUtils() {}
  AnnotationUtils::~AnnotationUtils() {}
  AbstractAnnotationRenderer *AnnotationUtils::GetAnnotationRenderer(const std::string &arTypeID,
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

  void AnnotationUtils::RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer)
  {
    static AnnotationRendererServices AnnotationRendererServices;
    // Define ServiceProps
    us::ServiceProperties props;
    props[AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID] = annotationRenderer->GetRendererID();
    props[AbstractAnnotationRenderer::US_PROPKEY_ID] = annotationRenderer->GetID();

    us::GetModuleContext()->RegisterService(annotationRenderer, props);
    AnnotationRendererServices.push_back(std::unique_ptr<AbstractAnnotationRenderer>(annotationRenderer));
  }

  void AnnotationUtils::UpdateAnnotationRenderer(const std::string &rendererID)
  {
    for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(rendererID))
    {
      annotationRenderer->Update();
    }
  }

  void AnnotationUtils::BaseRendererChanged(BaseRenderer *renderer)
  {
    for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(renderer->GetName()))
    {
      annotationRenderer->CurrentBaseRendererChanged();
    }
    vtkCallbackCommand *renderCallbackCommand = vtkCallbackCommand::New();
    renderCallbackCommand->SetCallback(AnnotationUtils::RenderWindowCallback);
    renderer->GetRenderWindow()->AddObserver(vtkCommand::ModifiedEvent, renderCallbackCommand);
    renderCallbackCommand->Delete();
  }

  void AnnotationUtils::RenderWindowCallback(vtkObject *caller, unsigned long, void *, void *)
  {
    auto *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);
    if (!renderWindow)
      return;
    BaseRenderer *renderer = BaseRenderer::GetInstance(renderWindow);

    if (nullptr != renderer)
    {
      for (AbstractAnnotationRenderer *annotationRenderer : GetAnnotationRenderer(renderer->GetName()))
        annotationRenderer->OnRenderWindowModified();
    }
  }

  Annotation *AnnotationUtils::GetAnnotation(const std::string &AnnotationID)
  {
    std::string ldapFilter = "(" + Annotation::US_PROPKEY_ID + "=" + AnnotationID + ")";
    us::ModuleContext *context = us::GetModuleContext();
    std::vector<us::ServiceReference<mitk::Annotation>> annotations =
      context->GetServiceReferences<mitk::Annotation>(ldapFilter);
    Annotation *annotation = nullptr;
    if (!annotations.empty())
    {
      annotation = us::GetModuleContext()->GetService<mitk::Annotation>(annotations.front());
    }
    return annotation;
  }

  std::vector<AbstractAnnotationRenderer *> AnnotationUtils::GetAnnotationRenderer(const std::string &rendererID)
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
