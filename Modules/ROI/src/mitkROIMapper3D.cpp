/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper3D.h>
#include <mitkROI.h>

#include "mitkROIMapperHelper.h"

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>

mitk::ROIMapper3D::LocalStorage::LocalStorage()
{
}

mitk::ROIMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::ROIMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  Superclass::SetDefaultProperties(node, renderer, override);
  ROIMapperHelper::SetDefaultProperties(node, renderer, override);
}

mitk::ROIMapper3D::ROIMapper3D()
{
}

mitk::ROIMapper3D::~ROIMapper3D()
{
}

void mitk::ROIMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  const auto timePoint = renderer->GetWorldTimeGeometry()->TimeStepToTimePoint(renderer->GetTimeStep());
  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  const auto* dataNode = this->GetDataNode();

  if (localStorage->GetLastGenerateDataTime() >= dataNode->GetMTime() &&
      localStorage->GetLastTimePoint() == timePoint)
  {
    return;
  }

  localStorage->SetLastTimePoint(timePoint);

  auto data = static_cast<const ROI*>(this->GetData());

  if (!data->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return;

  const auto t = data->GetTimeGeometry()->TimePointToTimeStep(timePoint);
  auto propAssembly = vtkSmartPointer<vtkPropAssembly>::New();

  if (dataNode->IsVisible(renderer))
  {
    const auto* geometry = data->GetGeometry();
    const auto halfSpacing = geometry->GetSpacing() * 0.5f;

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
      cube->Update();

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(cube->GetOutputPort());

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      this->ApplyColorAndOpacityProperties(renderer, actor);
      ROIMapperHelper::ApplyIndividualProperties(roi, t, actor);

      propAssembly->AddPart(actor);
    }
  }

  localStorage->SetPropAssembly(propAssembly);
  localStorage->UpdateGenerateDataTime();
}

void mitk::ROIMapper3D::ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor)
{
  auto* property = actor->GetProperty();

  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}

vtkProp* mitk::ROIMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
