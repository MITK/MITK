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
#include "mitkOverlay2DLayouter.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string mitk::AnnotationPlacer::ANNOTATIONRENDERER_ID = "AnnotationPlacer";

  AnnotationPlacer::~AnnotationPlacer() {}
  const std::string AnnotationPlacer::GetID() const { return ANNOTATIONRENDERER_ID; }
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
}
