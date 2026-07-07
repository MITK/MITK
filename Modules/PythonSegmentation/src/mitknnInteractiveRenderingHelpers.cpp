/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnInteractiveRenderingHelpers.h"

#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>

void mitk::nnInteractive::HideNodeIn3DRenderWindows(DataNode* node)
{
  if (node == nullptr)
    return;

  for (const auto& [renderWindow, renderer] : BaseRenderer::GetAll3DRenderWindows())
    node->SetVisibility(false, renderer);
}
