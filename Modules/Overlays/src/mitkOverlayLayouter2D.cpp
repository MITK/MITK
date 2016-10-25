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

#include "mitkOverlayLayouter2D.h"
#include "mitkBaseRenderer.h"

#include "mitkAnnotationService.h"
#include "mitkOverlay2DLayouter.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string mitk::OverlayLayouter2D::ANNOTATIONRENDERER_ID = "OverlayLayouter2D";

  OverlayLayouter2D::~OverlayLayouter2D() {}
  const std::string OverlayLayouter2D::GetID() const { return ANNOTATIONRENDERER_ID; }
  OverlayLayouter2D *OverlayLayouter2D::GetAnnotationRenderer(const std::string &rendererID)
  {
    OverlayLayouter2D *result = nullptr;
    AbstractAnnotationRenderer *registeredService =
      AnnotationService::GetAnnotationRenderer(ANNOTATIONRENDERER_ID, rendererID);
    if (registeredService)
      result = dynamic_cast<OverlayLayouter2D *>(registeredService);
    if (!result)
    {
      result = new OverlayLayouter2D(rendererID);
      AnnotationService::RegisterAnnotationRenderer(result);
    }
    return result;
  }
}
