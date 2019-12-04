/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkManualPlacementAnnotationRenderer.h"
#include "mitkBaseRenderer.h"

#include "mitkAnnotationUtils.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string ManualPlacementAnnotationRenderer::ANNOTATIONRENDERER_ID = "ManualPlacementAnnotationRenderer";

  ManualPlacementAnnotationRenderer::ManualPlacementAnnotationRenderer(const std::string &rendererId)
    : AbstractAnnotationRenderer(rendererId, ManualPlacementAnnotationRenderer::ANNOTATIONRENDERER_ID)
  {
  }
  ManualPlacementAnnotationRenderer::~ManualPlacementAnnotationRenderer() {}
  ManualPlacementAnnotationRenderer *ManualPlacementAnnotationRenderer::GetAnnotationRenderer(const std::string &rendererID)
  {
    ManualPlacementAnnotationRenderer *result = nullptr;
    AbstractAnnotationRenderer *registeredService =
      AnnotationUtils::GetAnnotationRenderer(ANNOTATIONRENDERER_ID, rendererID);
    if (registeredService)
      result = dynamic_cast<ManualPlacementAnnotationRenderer *>(registeredService);
    if (!result)
    {
      result = new ManualPlacementAnnotationRenderer(rendererID);
      AnnotationUtils::RegisterAnnotationRenderer(result);
    }
    return result;
  }

  void ManualPlacementAnnotationRenderer::AddAnnotation(Annotation *Annotation, const std::string &rendererID)
  {
    GetAnnotationRenderer(rendererID); // rename ??
    us::ServiceProperties props;
    props[Annotation::US_PROPKEY_AR_ID] = ANNOTATIONRENDERER_ID;
    props[Annotation::US_PROPKEY_RENDERER_ID] = rendererID;
    Annotation->RegisterAsMicroservice(props);
  }

  void ManualPlacementAnnotationRenderer::AddAnnotation(Annotation *Annotation, BaseRenderer *renderer)
  {
    AddAnnotation(Annotation, renderer->GetName());
  }
}
