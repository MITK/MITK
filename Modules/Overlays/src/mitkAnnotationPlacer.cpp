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

#include "mitkAnnotationPlacer.h"
#include "mitkBaseRenderer.h"

#include "mitkAnnotationService.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string AnnotationPlacer::ANNOTATIONRENDERER_ID = "AnnotationPlacer";

  AnnotationPlacer::AnnotationPlacer(const std::string &rendererId)
    : AbstractAnnotationRenderer(rendererId, AnnotationPlacer::ANNOTATIONRENDERER_ID)
  {
  }
  AnnotationPlacer::~AnnotationPlacer() {}
  AnnotationPlacer *AnnotationPlacer::GetAnnotationRenderer(const std::string &rendererID)
  {
    AnnotationPlacer *result = nullptr;
    AbstractAnnotationRenderer *registeredService =
      AnnotationService::GetAnnotationRenderer(ANNOTATIONRENDERER_ID, rendererID);
    if (registeredService)
      result = dynamic_cast<AnnotationPlacer *>(registeredService);
    if (!result)
    {
      result = new AnnotationPlacer(rendererID);
      AnnotationService::RegisterAnnotationRenderer(result);
    }
    return result;
  }

  void AnnotationPlacer::AddOverlay(Overlay *overlay, const std::string &rendererID)
  {
    GetAnnotationRenderer(rendererID); // rename ??
    us::ServiceProperties props;
    props[Overlay::US_PROPKEY_AR_ID] = ANNOTATIONRENDERER_ID;
    props[Overlay::US_PROPKEY_RENDERER_ID] = rendererID;
    overlay->RegisterAsMicroservice(props);
  }

  void AnnotationPlacer::AddOverlay(Overlay *overlay, BaseRenderer *renderer)
  {
    AddOverlay(overlay, renderer->GetName());
  }
}
