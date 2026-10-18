/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceIOHelper.h"

#include <mitkArbitraryTimeGeometry.h>
#include <mitkGeometry3D.h>
#include <mitkProportionalTimeGeometry.h>

#include <vtkLinearTransform.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTransformFilter.h>

namespace
{
  int GetPointDataType(const mitk::Surface *surface)
  {
    for (unsigned int timeStep = 0; timeStep < surface->GetSizeOfPolyDataSeries(); ++timeStep)
    {
      auto *polyData = surface->GetVtkPolyData(timeStep);

      if (nullptr != polyData && nullptr != polyData->GetPoints())
        return polyData->GetPoints()->GetDataType();
    }

    return VTK_FLOAT;
  }

  mitk::TimeGeometry::Pointer CreateTimeGeometry(mitk::TimeStepType timeSteps, const std::vector<mitk::TimeBounds> &timeBounds)
  {
    if (timeBounds.empty())
    {
      auto timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize(mitk::Geometry3D::New(), timeSteps);
      return timeGeometry.GetPointer();
    }

    const auto firstTimePoint = timeBounds.front()[0];
    const auto stepDuration = timeBounds.front()[1] - firstTimePoint;

    // Same arithmetic as ProportionalTimeGeometry, so bounds that were written
    // from one compare equal.
    bool isProportional = true;

    for (mitk::TimeStepType timeStep = 0; timeStep < timeSteps && isProportional; ++timeStep)
    {
      const auto minimumTimePoint = timeStep == 0 ? firstTimePoint : firstTimePoint + stepDuration * timeStep;
      const auto maximumTimePoint = firstTimePoint + stepDuration * (timeStep + 1);

      isProportional = timeBounds[timeStep][0] == minimumTimePoint && timeBounds[timeStep][1] == maximumTimePoint;
    }

    if (isProportional)
    {
      auto timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize(mitk::Geometry3D::New(), timeSteps);
      timeGeometry->SetFirstTimePoint(firstTimePoint);
      timeGeometry->SetStepDuration(stepDuration);
      return timeGeometry.GetPointer();
    }

    auto timeGeometry = mitk::ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();

    for (const auto &bounds : timeBounds)
      timeGeometry->AppendNewTimeStep(mitk::Geometry3D::New(), bounds[0], bounds[1]);

    timeGeometry->Update();
    return timeGeometry.GetPointer();
  }
}

vtkSmartPointer<vtkPolyData> mitk::SurfaceIOHelper::GetWorldPolyData(const Surface *surface, TimeStepType timeStep)
{
  auto *polyData = surface->GetVtkPolyData(static_cast<unsigned int>(timeStep));

  if (nullptr == polyData)
    mitkThrow() << "The surface has no polygonal data at time step " << timeStep << ".";

  auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  transformFilter->SetInputData(polyData);
  transformFilter->SetTransform(surface->GetGeometry(static_cast<int>(timeStep))->GetVtkTransform());
  transformFilter->Update();

  auto *worldPolyData = transformFilter->GetPolyDataOutput();

  if (nullptr == worldPolyData->GetPoints())
  {
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataType(GetPointDataType(surface));
    worldPolyData->SetPoints(points);
  }

  return worldPolyData;
}

bool mitk::SurfaceIOHelper::AreValidTimeBounds(const std::vector<TimeBounds> &timeBounds)
{
  for (std::size_t i = 0; i < timeBounds.size(); ++i)
  {
    if (!(timeBounds[i][0] < timeBounds[i][1]))
      return false;

    if (i > 0 && timeBounds[i][0] < timeBounds[i - 1][1])
      return false;
  }

  return true;
}

mitk::Surface::Pointer mitk::SurfaceIOHelper::CreateSurface(const std::vector<vtkSmartPointer<vtkPolyData>> &polyDatas,
                                                            const std::vector<TimeBounds> &timeBounds)
{
  const auto timeSteps = static_cast<unsigned int>(polyDatas.size());

  auto surface = Surface::New();
  surface->SetTimeGeometry(CreateTimeGeometry(timeSteps, timeBounds));
  surface->Expand(timeSteps);

  for (unsigned int timeStep = 0; timeStep < timeSteps; ++timeStep)
    surface->SetVtkPolyData(polyDatas[timeStep], timeStep);

  return surface;
}
