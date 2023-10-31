/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper2D.h>
#include <mitkROI.h>

#include <vtkCubeSource.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataPlaneCutter.h>
#include <vtkSmartPointer.h>

// Implemented in mitkROIMapper3D.cpp
extern void ApplyIndividualProperties(const mitk::IPropertyProvider* properties, vtkActor* actor);

mitk::ROIMapper2D::LocalStorage::LocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
}

mitk::ROIMapper2D::LocalStorage::~LocalStorage()
{
}

vtkPropAssembly* mitk::ROIMapper2D::LocalStorage::GetPropAssembly() const
{
  return m_PropAssembly;
}

void mitk::ROIMapper2D::LocalStorage::SetPropAssembly(vtkPropAssembly* propAssembly)
{
  m_PropAssembly = propAssembly;
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
  // Implemented in ROIMapper3D
}

mitk::ROIMapper2D::ROIMapper2D()
{
}

mitk::ROIMapper2D::~ROIMapper2D()
{
}

void mitk::ROIMapper2D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  const auto* dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  auto* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  const auto* planeGeometry = renderer->GetCurrentWorldPlaneGeometry();

  if (localStorage->GetLastPlaneGeometry() != nullptr && localStorage->GetLastPlaneGeometry()->IsOnPlane(planeGeometry) &&
      localStorage->GetLastGenerateDataTime() >= dataNode->GetMTime())
  {
    return;
  }

  localStorage->SetLastPlaneGeometry(planeGeometry->Clone());

  const auto* data = dynamic_cast<ROI*>(this->GetData());

  if (data == nullptr)
    return;

  auto propAssembly = vtkSmartPointer<vtkPropAssembly>::New();

  if (dataNode->IsVisible(renderer))
  {
    const auto* geometry = data->GetGeometry();
    const auto halfSpacing = geometry->GetSpacing() * 0.5f;

    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(planeGeometry->GetOrigin().data());
    plane->SetNormal(planeGeometry->GetNormal().data());

    for (const auto& roi : *data)
    {
      Point3D min;
      geometry->IndexToWorld(roi.Min, min);
      min -= halfSpacing;

      Point3D max;
      geometry->IndexToWorld(roi.Max, max);
      max += halfSpacing;

      auto cube = vtkSmartPointer<vtkCubeSource>::New();
      cube->SetBounds(min[0], max[0], min[1], max[1], min[2], max[2]);

      auto cutter = vtkSmartPointer<vtkPolyDataPlaneCutter>::New();
      cutter->SetInputConnection(cube->GetOutputPort());
      cutter->SetPlane(plane);
      cutter->Update();

      if (cutter->GetOutput()->GetNumberOfLines() == 0)
        continue;

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(cutter->GetOutputPort());

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      this->ApplyColorAndOpacityProperties(renderer, actor);
      ApplyIndividualProperties(roi.Properties, actor);

      propAssembly->AddPart(actor);
    }
  }

  localStorage->SetPropAssembly(propAssembly);
  localStorage->UpdateGenerateDataTime();
}

void mitk::ROIMapper2D::ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor)
{
  auto* property = actor->GetProperty();

  float opacity = 1.0f;
  this->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}

vtkProp* mitk::ROIMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
