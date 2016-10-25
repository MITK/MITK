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
}
