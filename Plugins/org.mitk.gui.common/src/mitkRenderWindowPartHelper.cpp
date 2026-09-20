/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindowPartHelper.h"

#include "mitkIRenderWindowPart.h"

#include <mitkBaseGeometry.h>
#include <mitkSliceNavigationController.h>

#include <QmitkRenderWindow.h>

bool mitk::RenderWindowPartHelper::IsRenderWindowPartAlignedWithGeometry(const IRenderWindowPart* renderWindowPart, const BaseGeometry* geometry)
{
  if (nullptr == renderWindowPart || nullptr == geometry || !renderWindowPart->HasCoupledRenderWindows())
    return true;

  // Coupled render windows share one world geometry. The 3D window exposes it
  // without the axis permutation the 2D windows apply for their view direction.
  auto* renderWindow = renderWindowPart->GetQmitkRenderWindow("3d");

  if (nullptr == renderWindow)
    return true;

  const BaseGeometry* renderWindowGeometry = renderWindow->GetSliceNavigationController()->GetCurrentGeometry3D();

  if (nullptr == renderWindowGeometry)
    return true;

  return Equal(*geometry->GetBoundingBox(), *renderWindowGeometry->GetBoundingBox(), eps, false);
}
