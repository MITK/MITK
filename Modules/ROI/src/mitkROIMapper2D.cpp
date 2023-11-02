/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper2D.h>
#include <mitkROI.h>

#include <vtkCaptionActor2D.h>
#include <vtkCubeSource.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataPlaneCutter.h>
#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// Implemented in mitkROIMapper3D.cpp
extern void ApplyIndividualProperties(const mitk::IPropertyProvider* properties, vtkActor* actor);

namespace
{
  std::string GetName(const mitk::ROI::Element& roi)
  {
    auto property = roi.GetConstProperty("name");

    if (property.IsNotNull())
    {
      auto nameProperty = dynamic_cast<const mitk::StringProperty*>(property.GetPointer());

      if (nameProperty != nullptr)
        return nameProperty->GetValue();
    }

    return "";
  }

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

      auto* slicePolyData = cutter->GetOutput();

      if (slicePolyData->GetNumberOfLines() == 0)
        continue;

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(cutter->GetOutputPort());

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);

      this->ApplyColorAndOpacityProperties(renderer, actor);
      ApplyIndividualProperties(roi.Properties, actor);

      propAssembly->AddPart(actor);

      auto name = GetName(roi);

      if (!name.empty())
      {
        auto bottomLeftPoint = GetBottomLeftPoint(slicePolyData->GetPoints(), renderer);
        auto captionActor = CreateCaptionActor(name, bottomLeftPoint.data(), actor->GetProperty(), renderer);

        propAssembly->AddPart(captionActor);
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
  this->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}

vtkProp* mitk::ROIMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}

vtkSmartPointer<vtkCaptionActor2D> mitk::ROIMapper2D::CreateCaptionActor(const std::string& caption, double* attachmentPoint, vtkProperty* property, const BaseRenderer* renderer) const
{
  auto captionActor = vtkSmartPointer<vtkCaptionActor2D>::New();
  captionActor->SetPosition(property->GetLineWidth() * 0.5, property->GetLineWidth() * 0.5);
  captionActor->GetTextActor()->SetTextScaleModeToNone();
  captionActor->SetAttachmentPoint(attachmentPoint);
  captionActor->SetCaption(caption.c_str());
  captionActor->BorderOff();
  captionActor->LeaderOff();

  auto* textProperty = captionActor->GetCaptionTextProperty();
  textProperty->SetColor(property->GetColor());
  textProperty->SetOpacity(property->GetOpacity());
  textProperty->ShadowOff();

  auto* dataNode = this->GetDataNode();

  int fontSize = 16;
  dataNode->GetIntProperty("font.size", fontSize, renderer);
  textProperty->SetFontSize(fontSize);

  bool bold = false;
  dataNode->GetBoolProperty("font.bold", bold, renderer);
  textProperty->SetBold(bold);

  bool italic = false;
  dataNode->GetBoolProperty("font.italic", italic, renderer);
  textProperty->SetItalic(italic);

  return captionActor;
}
