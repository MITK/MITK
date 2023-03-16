/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSliceNavigationHelper.h"

#include <mitkArbitraryTimeGeometry.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkPoint.h>
#include <mitkRenderingManager.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSliceNavigationController.h>

unsigned int mitk::SliceNavigationHelper::SelectSliceByPoint(const TimeGeometry* timeGeometry,
                                                             const Point3D& point)
{
  int selectedSlice = -1;
  if (nullptr == timeGeometry)
  {
    return selectedSlice;
  }

  // get the BaseGeometry of the selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  auto currentGeometry = timeGeometry->GetGeometryForTimePoint(selectedTimePoint);
  if (nullptr == currentGeometry)
  {
    // time point not valid for the ime geometry
    mitkThrow() << "Cannot extract a base geometry. A time point is selected that is not covered by"
                << "the given time geometry. Selected time point: " << selectedTimePoint;
  }

  const auto* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(currentGeometry.GetPointer());
  if (nullptr == slicedGeometry)
  {
    return selectedSlice;
  }

  double bestDistance = itk::NumericTraits<double>::max();
  if (slicedGeometry->GetEvenlySpaced())
  {
    PlaneGeometry* plane = slicedGeometry->GetPlaneGeometry(0);
    const Vector3D& direction = slicedGeometry->GetDirectionVector();

    Point3D projectedPoint;
    plane->Project(point, projectedPoint);

    // check whether the point is somewhere within the slice stack volume
    if (direction[0] * (point[0] - projectedPoint[0]) + direction[1] * (point[1] - projectedPoint[1]) +
        direction[2] * (point[2] - projectedPoint[2]) >= 0)
    {
      selectedSlice = static_cast<int>(plane->Distance(point) / slicedGeometry->GetSpacing()[2] + 0.5);
    }
  }
  else
  {
    int numberOfSlices = slicedGeometry->GetSlices();
    Point3D projectedPoint;
    for (int slice = 0; slice < numberOfSlices; ++slice)
    {
      slicedGeometry->GetPlaneGeometry(slice)->Project(point, projectedPoint);
      const Vector3D distance = projectedPoint - point;
      ScalarType currentDistance = distance.GetSquaredNorm();

      if (currentDistance < bestDistance)
      {
        bestDistance = currentDistance;
        selectedSlice = slice;
      }
    }
  }

  if (selectedSlice < 0)
  {
    selectedSlice = 0; // do not allow negative slices
  }

  return selectedSlice;
}

mitk::TimeGeometry::Pointer mitk::SliceNavigationHelper::CreateOrientedTimeGeometry(const TimeGeometry* timeGeometry,
                                                                                    AnatomicalPlane orientation,
                                                                                    bool top,
                                                                                    bool frontside,
                                                                                    bool rotated)
{
  if (nullptr == timeGeometry)
  {
    return nullptr;
  }

  // initialize the viewplane
  SlicedGeometry3D::Pointer slicedGeometry;
  BaseGeometry::ConstPointer currentGeometry;

  // get the BaseGeometry of the selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  currentGeometry = timeGeometry->GetGeometryForTimePoint(selectedTimePoint);
  if(nullptr == currentGeometry)
  {
    // time point not valid for the time geometry
    mitkThrow() << "Cannot extract a base geometry. A time point is selected that is not covered by"
                << "the given time geometry. Selected time point: " << selectedTimePoint;
  }

  slicedGeometry = SlicedGeometry3D::New();
  slicedGeometry->InitializePlanes(currentGeometry, orientation, top, frontside, rotated);

  // use the existing time geometry but replace all time steps with the newly created
  // sliced geometry (base geometry), initialized to the desired plane orientation
  auto createdTimeGeometry = timeGeometry->Clone();
  createdTimeGeometry->ReplaceTimeStepGeometries(slicedGeometry);
  return createdTimeGeometry;
}

mitk::PlaneGeometry* mitk::SliceNavigationHelper::GetCurrentPlaneGeometry(const TimeGeometry* timeGeometry,
                                                                          TimePointType timePoint,
                                                                          unsigned int slicePosition)
{
  if (nullptr == timeGeometry)
  {
    return nullptr;
  }

  const auto* slicedGeometry =
    dynamic_cast<SlicedGeometry3D*>(timeGeometry->GetGeometryForTimePoint(timePoint).GetPointer());

  if (nullptr == slicedGeometry)
  {
    // time point not valid for the time geometry
    mitkThrow() << "Cannot extract a sliced geometry. A time point is selected that is not covered by"
                << "the given time geometry. Selected time point: " << timePoint;
  }

  return slicedGeometry->GetPlaneGeometry(slicePosition);
}
