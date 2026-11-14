/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseRendererHelper.h>

#include <mitkSlicedGeometry3D.h>

bool mitk::BaseRendererHelper::IsRendererGeometryAlignedWithGeometry(BaseRenderer* renderer, const TimeGeometry* timeGeometry)
{
  if (nullptr == renderer)
  {
    return false;
  }

  const BaseGeometry* renderWindowGeometry = renderer->GetCurrentWorldGeometry();
  if (nullptr == renderWindowGeometry)
  {
    return false;
  }

  if (nullptr == timeGeometry)
  {
    // no reference geometry given, so everything is fine
    return true;
  }

  const auto viewDirection = renderer->GetSliceNavigationController()->GetViewDirection();

  bool top = false;
  bool frontside = false;
  bool rotated = false;
  switch (viewDirection)
  {
  case mitk::AnatomicalPlane::Original:
    return false;
  case mitk::AnatomicalPlane::Axial:
    rotated = true;
    break;
  case mitk::AnatomicalPlane::Coronal:
    frontside = true;
    break;
  case mitk::AnatomicalPlane::Sagittal:
    top = true;
    frontside = true;
    break;
  }

  const auto referenceGeometry =
    timeGeometry->GetGeometryForTimeStep(timeGeometry->TimePointToClampedTimeStep(renderer->GetTime()));

  auto orientedGeometry = SlicedGeometry3D::New();
  orientedGeometry->InitializePlanes(referenceGeometry, viewDirection, top, frontside, rotated);

  return Equal(*orientedGeometry->GetBoundingBox(), *renderWindowGeometry->GetBoundingBox(), eps, true);
}
