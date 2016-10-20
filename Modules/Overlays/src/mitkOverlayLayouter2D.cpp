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

#include <mitkVtkLayerController.h>
#include "mitkOverlay2DLayouter.h"
#include "mitkAnnotationService.h"

namespace mitk
{

OverlayLayouter2D::~OverlayLayouter2D()
{

}

void OverlayLayouter2D::RegisterAnnotationRenderer(const std::string &rendererID)
{
  AnnotationService::RegisterAnnotationRenderer<OverlayLayouter2D>(rendererID);
}


}
