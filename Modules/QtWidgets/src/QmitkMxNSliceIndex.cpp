/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNSliceIndex.h"

#include <mitkAnatomicalPlanes.h>
#include <mitkBaseRenderer.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkTimeNavigationController.h>

bool QmitkMxNSliceIndex::IsDisplayedSliceInverted(mitk::BaseRenderer* renderer)
{
  const auto* sliceNavigation = renderer->GetSliceNavigationController();
  if (nullptr == sliceNavigation)
  {
    return false;
  }

  const auto viewDirection = sliceNavigation->GetViewDirection();
  if (mitk::AnatomicalPlane::Original == viewDirection)
  {
    return false;
  }

  const auto* inputTimeGeometry = sliceNavigation->GetInputWorldTimeGeometry();
  const auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  if (nullptr == inputTimeGeometry || nullptr == timeNavigation)
  {
    return false;
  }

  // The same time step fallback CreateWorldGeometry uses to build the
  // renderer planes.
  const auto selectedTimeStep = timeNavigation->GetSelectedTimeStep();
  const mitk::BaseGeometry::ConstPointer referenceGeometry = inputTimeGeometry->GetGeometryForTimeStep(
    inputTimeGeometry->IsValidTimeStep(selectedTimeStep) ? selectedTimeStep : 0);
  const auto* rendererGeometry = renderer->GetCurrentWorldGeometry();
  if (referenceGeometry.IsNull() || nullptr == rendererGeometry)
  {
    return false;
  }

  return mitk::SliceNavigationHelper::IsSliceIndexInverted(referenceGeometry, rendererGeometry, viewDirection);
}
