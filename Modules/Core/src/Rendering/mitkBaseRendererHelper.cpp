/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBaseRendererHelper.h"

#include <mitkSliceNavigationHelper.h>

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

  auto viewDirection = renderer->GetSliceNavigationController()->GetViewDirection();
  TimeGeometry::Pointer orientedTimeGeometry;
  try
  {
    switch (viewDirection)
    {
    case mitk::AnatomicalPlane::Original:
      break;
    case mitk::AnatomicalPlane::Axial:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, viewDirection, false, false, true);
      break;
    }
    case mitk::AnatomicalPlane::Coronal:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, viewDirection, false, true, false);
      break;
    }
    case mitk::AnatomicalPlane::Sagittal:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, viewDirection, true, true, false);
      break;
    }
    }
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e);
  }

  if (nullptr == orientedTimeGeometry)
  {
    return false;
  }

  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  const BaseGeometry* workingImageBaseGeometry = orientedTimeGeometry->GetGeometryForTimePoint(selectedTimePoint);
  if (nullptr == workingImageBaseGeometry)
  {
    // time point not valid for the time geometry
    mitkThrow() << "Cannot extract a base geometry. A time point is selected that is not covered by"
                << "the given time geometry. Selected time point: " << selectedTimePoint;
  }

  bool geometryIsEqual =
    Equal(*workingImageBaseGeometry->GetBoundingBox(), *renderWindowGeometry->GetBoundingBox(), eps, true);

  return geometryIsEqual;
}
