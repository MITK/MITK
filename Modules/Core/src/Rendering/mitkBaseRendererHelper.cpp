/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBaseRendererHelper.h"

#include <mitkSliceNavigationHelper.h>

bool mitk::BaseRendererHelper::IsRendererAlignedWithSegmentation(BaseRenderer* renderer, const TimeGeometry* timeGeometry)
{
  const BaseGeometry* renderWindowGeometry = renderer->GetCurrentWorldGeometry();

  if (nullptr == renderWindowGeometry || nullptr == timeGeometry)
  {
    return false;
  }

  auto viewDirection = renderer->GetSliceNavigationController()->GetViewDirection();
  TimeGeometry::Pointer orientedTimeGeometry;
  try
  {
    switch (viewDirection)
    {
    case mitk::SliceNavigationController::Original:
      break;
    case mitk::SliceNavigationController::Axial:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, PlaneGeometry::Axial, false, false, true);
      break;
    }
    case mitk::SliceNavigationController::Coronal:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, PlaneGeometry::Coronal, false, true, false);
      break;
    }
    case mitk::SliceNavigationController::Sagittal:
    {
      orientedTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
        timeGeometry, PlaneGeometry::Sagittal, true, true, false);
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
