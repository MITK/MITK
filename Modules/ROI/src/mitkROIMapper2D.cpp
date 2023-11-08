/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper2D.h>

#include "mitkROIMapperHelper.h"

#include <vtkCubeSource.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataPlaneCutter.h>
#include <vtkPropAssembly.h>

#include <boost/algorithm/string.hpp>

#include <regex>

namespace
{
  mitk::Point3D GetBottomLeftPoint(vtkPoints* points, mitk::BaseRenderer* renderer)
  {
    mitk::Point3D point = points->GetPoint(0);
    mitk::Point2D bottomLeftDisplayPoint;
    renderer->WorldToDisplay(point, bottomLeftDisplayPoint);

    auto numPoints = points->GetNumberOfPoints();
    mitk::Point2D displayPoint;

    for (decltype(numPoints) i = 1; i < numPoints; ++i)
    {
      point.FillPoint(points->GetPoint(i));
      renderer->WorldToDisplay(point, displayPoint);

      bottomLeftDisplayPoint[0] = std::min(bottomLeftDisplayPoint[0], displayPoint[0]);
      bottomLeftDisplayPoint[1] = std::min(bottomLeftDisplayPoint[1], displayPoint[1]);
    }

    renderer->DisplayToWorld(bottomLeftDisplayPoint, point);

    return point;
  }
}

mitk::ROIMapper2D::LocalStorage::LocalStorage()
{
}

mitk::ROIMapper2D::LocalStorage::~LocalStorage()
{
}

const mitk::PlaneGeometry* mitk::ROIMapper2D::LocalStorage::GetLastPlaneGeometry() const
{
  return m_LastPlaneGeometry;
}

void mitk::ROIMapper2D::LocalStorage::SetLastPlaneGeometry(const PlaneGeometry* planeGeometry)
{
  m_LastPlaneGeometry = planeGeometry;
}

void mitk::ROIMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  Superclass::SetDefaultProperties(node, renderer, override);
  ROIMapperHelper::SetDefaultProperties(node, renderer, override);
}

mitk::ROIMapper2D::ROIMapper2D()
{
}

mitk::ROIMapper2D::~ROIMapper2D()
{
}

void mitk::ROIMapper2D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  const auto timePoint = renderer->GetWorldTimeGeometry()->TimeStepToTimePoint(renderer->GetTimeStep());
  const auto* planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  const auto* dataNode = this->GetDataNode();

  if (localStorage->GetLastPlaneGeometry() != nullptr && localStorage->GetLastPlaneGeometry()->IsOnPlane(planeGeometry) &&
      localStorage->GetLastGenerateDataTime() >= dataNode->GetMTime() &&
      localStorage->GetLastTimePoint() == timePoint)
  {
    return;
  }

  localStorage->SetLastPlaneGeometry(planeGeometry->Clone());
  localStorage->SetLastTimePoint(timePoint);

  auto data = static_cast<const ROI*>(this->GetData());

  if (!data->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return;

  const auto t = data->GetTimeGeometry()->TimePointToTimeStep(timePoint);
  auto propAssembly = vtkSmartPointer<vtkPropAssembly>::New();

  if (dataNode->IsVisible(renderer))
  {
    const auto* geometry = data->GetGeometry(t);
    const auto halfSpacing = geometry->GetSpacing() * 0.5f;

    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(planeGeometry->GetOrigin().data());
    plane->SetNormal(planeGeometry->GetNormal().data());

    for (const auto& [id, roi] : *data)
    {
      if (!roi.HasTimeStep(t))
        continue;

      Point3D min;
      geometry->IndexToWorld(roi.GetMin(t), min);
      min -= halfSpacing;

      Point3D max;
      geometry->IndexToWorld(roi.GetMax(t), max);
      max += halfSpacing;

      auto cube = vtkSmartPointer<vtkCubeSource>::New();
      cube->SetBounds(min[0], max[0], min[1], max[1], min[2], max[2]);

      auto cutter = vtkSmartPointer<vtkPolyDataPlaneCutter>::New();
      cutter->SetInputConnection(cube->GetOutputPort());
      cutter->SetPlane(plane);
      cutter->Update();

      auto* slicePolyData = cutter->GetOutput();

      if (slicePolyData->GetNumberOfLines() == 0)
        continue;

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(cutter->GetOutputPort());

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      this->ApplyColorAndOpacityProperties(renderer, actor);
      ROIMapperHelper::ApplyIndividualProperties(roi, t, actor);

      propAssembly->AddPart(actor);

      if (std::string caption; dataNode->GetStringProperty("caption", caption, renderer))
      {
        caption = ROIMapperHelper::ParseCaption(caption, roi, t);

        if (!caption.empty())
        {
          auto bottomLeftPoint = GetBottomLeftPoint(slicePolyData->GetPoints(), renderer);
          auto captionActor = ROIMapperHelper::CreateCaptionActor(caption, bottomLeftPoint, actor->GetProperty(), dataNode, renderer);

          propAssembly->AddPart(captionActor);
        }
      }
    }
  }

  localStorage->SetPropAssembly(propAssembly);
  localStorage->UpdateGenerateDataTime();
}

void mitk::ROIMapper2D::ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor)
{
  auto* property = actor->GetProperty();

  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}

vtkProp* mitk::ROIMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
