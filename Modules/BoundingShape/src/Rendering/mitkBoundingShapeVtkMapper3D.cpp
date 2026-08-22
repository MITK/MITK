/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBoundingShapeVtkMapper3D.h>
#include "mitkBoundingShapeUtil.h"
#include <mitkBaseProperty.h>
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCubeSource.h>
#include <vtkDataArray.h>
#include <vtkDataSetMapper.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransformFilter.h>
#include <vtkUnsignedCharArray.h>

namespace mitk
{
  class BoundingShapeVtkMapper3D::Impl
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      LocalStorage(const LocalStorage &) = delete;
      LocalStorage &operator=(const LocalStorage &) = delete;

      vtkSmartPointer<vtkActor> Actor;
      vtkSmartPointer<vtkActor> HandleActor;
      vtkSmartPointer<vtkActor> SelectedHandleActor;
      vtkSmartPointer<vtkPolyDataMapper> Mapper;
      vtkSmartPointer<vtkPolyDataMapper> HandleMapper;
      vtkSmartPointer<vtkPolyDataMapper> SelectedHandleMapper;
      vtkSmartPointer<vtkPropAssembly> PropAssembly;
    };

  public:
    mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
  };
}

mitk::BoundingShapeVtkMapper3D::Impl::LocalStorage::LocalStorage()
  : Actor(vtkSmartPointer<vtkActor>::New()),
    HandleActor(vtkSmartPointer<vtkActor>::New()),
    SelectedHandleActor(vtkSmartPointer<vtkActor>::New()),
    Mapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
    HandleMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
    SelectedHandleMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
    PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
  // the box and its handles are interaction widgets and keep their nominal color
  // regardless of scene lighting
  Actor->GetProperty()->LightingOff();
  HandleActor->GetProperty()->LightingOff();
  SelectedHandleActor->GetProperty()->LightingOff();

  HandleActor->GetProperty()->SetColor(1, 0, 0);
  SelectedHandleActor->GetProperty()->SetColor(0, 1, 0);

  // the actors are wired up once and are afterwards controlled through their input data
  // and visibility alone, so that regenerating does not churn the mappers or the assembly
  Actor->SetMapper(Mapper);
  HandleActor->SetMapper(HandleMapper);
  SelectedHandleActor->SetMapper(SelectedHandleMapper);

  // an actor is turned on only once its mapper has data; a visible one without would
  // contribute uninitialized bounds to the assembly
  Actor->VisibilityOff();
  HandleActor->VisibilityOff();
  SelectedHandleActor->VisibilityOff();

  PropAssembly->AddPart(Actor);
  PropAssembly->AddPart(HandleActor);
  PropAssembly->AddPart(SelectedHandleActor);
}

mitk::BoundingShapeVtkMapper3D::Impl::LocalStorage::~LocalStorage()
{
}

void mitk::BoundingShapeVtkMapper3D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  node->AddProperty("color", ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);
  node->AddProperty("opacity", FloatProperty::New(0.2f), renderer, overwrite);
}

mitk::BoundingShapeVtkMapper3D::BoundingShapeVtkMapper3D() : m_Impl(new Impl)
{
}

mitk::BoundingShapeVtkMapper3D::~BoundingShapeVtkMapper3D()
{
  delete m_Impl;
}

void mitk::BoundingShapeVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor)
{
  auto* property = actor->GetProperty();

  float color[3];
  GetBoundingShapeColor(this->GetDataNode(), renderer, color);
  property->SetColor(color[0], color[1], color[2]);

  float opacity = 0.2f;
  this->GetDataNode()->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}

void mitk::BoundingShapeVtkMapper3D::ApplyBoundingShapeProperties(BaseRenderer *renderer, vtkActor *actor)
{
  if (actor == nullptr)
    return;

  auto dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  float lineWidth = 1.0f;
  dataNode->GetFloatProperty("Bounding Shape.Line.Width", lineWidth, renderer);

  auto property = actor->GetProperty();
  property->SetLineWidth(lineWidth);
}

void mitk::BoundingShapeVtkMapper3D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  auto dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  vtkCamera *camera = renderer->GetVtkRenderer()->GetActiveCamera();

  auto localStorage = m_Impl->LocalStorageHandler.GetLocalStorage(renderer);

  // the handle size and the baked face shading depend on the camera, so any camera change
  // requires regeneration as well
  bool needGenerateData = localStorage->GetLastGenerateDataTime() < dataNode->GetMTime() ||
                          localStorage->GetLastGenerateDataTime() < camera->GetMTime();

  if (needGenerateData)
  {
    bool isVisible = true;
    dataNode->GetVisibility(isVisible, renderer);

    if (!isVisible)
    {
      // the handles belong to the shape and have to go with it, not just its body
      localStorage->PropAssembly->VisibilityOff();
      localStorage->Actor->VisibilityOff();
      localStorage->HandleActor->VisibilityOff();
      localStorage->SelectedHandleActor->VisibilityOff();
      return;
    }

    // set the input-object at time t for the mapper
    auto *geometryData = dynamic_cast<GeometryData *>(dataNode->GetData());
    if (geometryData == nullptr)
      return;

    mitk::BaseGeometry::Pointer geometry = geometryData->GetGeometry();
    mitk::Vector3D spacing = geometry->GetSpacing();

    // calculate cornerpoints from geometry
    std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, true);

    Point3D p0 = cornerPoints[0];
    Point3D p1 = cornerPoints[1];
    Point3D p2 = cornerPoints[2];
    Point3D p4 = cornerPoints[4];

    Point3D extent;
    extent[0] =
      sqrt((p0[0] - p4[0]) * (p0[0] - p4[0]) + (p0[1] - p4[1]) * (p0[1] - p4[1]) + (p0[2] - p4[2]) * (p0[2] - p4[2]));
    extent[1] =
      sqrt((p0[0] - p2[0]) * (p0[0] - p2[0]) + (p0[1] - p2[1]) * (p0[1] - p2[1]) + (p0[2] - p2[2]) * (p0[2] - p2[2]));
    extent[2] =
      sqrt((p0[0] - p1[0]) * (p0[0] - p1[0]) + (p0[1] - p1[1]) * (p0[1] - p1[1]) + (p0[2] - p1[2]) * (p0[2] - p1[2]));

    // calculate center based on half way of the distance between two opposing cornerpoints
    mitk::Point3D center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

    auto cube = vtkCubeSource::New();
    cube->SetXLength(extent[0] / spacing[0]);
    cube->SetYLength(extent[1] / spacing[1]);
    cube->SetZLength(extent[2] / spacing[2]);

    // calculates translation based on offset+extent not on the transformation matrix
    vtkSmartPointer<vtkMatrix4x4> imageTransform = geometry->GetVtkTransform()->GetMatrix();
    auto translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(center[0] - imageTransform->GetElement(0, 3),
                           center[1] - imageTransform->GetElement(1, 3),
                           center[2] - imageTransform->GetElement(2, 3));

    auto transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(imageTransform);
    transform->PostMultiply();
    transform->Concatenate(translation);
    transform->Update();
    cube->Update();

    auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
    transformFilter->SetInputData(cube->GetOutput());
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    cube->Delete();

    vtkSmartPointer<vtkPolyData> polydata = transformFilter->GetPolyDataOutput();
    if (polydata == nullptr)
    {
      localStorage->PropAssembly->VisibilityOff();
      localStorage->Actor->VisibilityOff();
      localStorage->HandleActor->VisibilityOff();
      localStorage->SelectedHandleActor->VisibilityOff();
      return;
    }

    // Bake camera-relative shading into per-face colors. The box is deliberately unlit
    // (scene lights must not affect an interaction widget), but with one flat color the
    // translucent faces of the convex box blend into a uniform silhouette: every pixel
    // is covered by exactly one front and one back face. Grading each face by its
    // orientation towards the camera makes the faces distinguishable again, including
    // those on the far side.
    vtkDataArray *pointNormals = polydata->GetPointData()->GetNormals();
    if (pointNormals != nullptr)
    {
      float color[3];
      GetBoundingShapeColor(dataNode, renderer, color);

      double viewDirection[3];
      camera->GetViewPlaneNormal(viewDirection); // points from the focal point towards the camera
      vtkMath::Normalize(viewDirection);

      auto faceColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
      faceColors->SetNumberOfComponents(3);

      const vtkIdType numberOfCells = polydata->GetNumberOfCells();
      for (vtkIdType cellId = 0; cellId < numberOfCells; ++cellId)
      {
        vtkIdType numberOfCellPoints = 0;
        const vtkIdType *cellPointIds = nullptr;
        polydata->GetCellPoints(cellId, numberOfCellPoints, cellPointIds);

        // one tuple per cell either way, so that the array stays aligned with the cell ids
        double brightness = 1.0;

        if (numberOfCellPoints > 0)
        {
          // all points of a face share the outward face normal
          double normal[3];
          pointNormals->GetTuple(cellPointIds[0], normal);
          vtkMath::Normalize(normal);

          constexpr double minBrightness = 0.15;
          const double facingRatio = 0.5 * (1.0 + vtkMath::Dot(normal, viewDirection));
          brightness = minBrightness + (1.0 - minBrightness) * facingRatio;
        }

        unsigned char rgb[3];
        for (int component = 0; component < 3; ++component)
          rgb[component] = static_cast<unsigned char>(255.0 * color[component] * brightness + 0.5);

        faceColors->InsertNextTypedTuple(rgb);
      }

      polydata->GetCellData()->SetScalars(faceColors);
    }

    auto appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();

    // handles are interaction affordances: the interactor adds the active-handle property
    // when it attaches to the node and removes it when it detaches, so without the
    // property no handles are rendered at all
    mitk::IntProperty::Pointer activeHandleId =
      dynamic_cast<mitk::IntProperty *>(dataNode->GetProperty(BoundingShapeActiveHandleIdPropertyName));

    bool hasIdleHandles = false;
    bool selected = false;

    if (activeHandleId != nullptr)
    {
      const double handleSize = GetHandleSize(renderer, dataNode);

      for (const auto &handle : ComputeHandles(cornerPoints, nullptr))
      {
        auto handlePolyData = CreateHandlePolyData(geometry, handle.GetPosition(), handleSize);

        if (activeHandleId->GetValue() == handle.GetIndex())
        {
          localStorage->SelectedHandleMapper->SetInputData(handlePolyData);
          selected = true;
        }
        else
        {
          appendPoly->AddInputData(handlePolyData);
          hasIdleHandles = true;
        }
      }
    }

    // vtkAppendPolyData requires at least one input; with none the actor stays hidden
    // and its stale output is never shown
    if (hasIdleHandles)
    {
      appendPoly->Update();
      localStorage->HandleMapper->SetInputData(appendPoly->GetOutput());
    }

    localStorage->Mapper->SetInputData(polydata);

    this->ApplyColorAndOpacityProperties(renderer, localStorage->Actor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->Actor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->HandleActor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->SelectedHandleActor);

    localStorage->Actor->VisibilityOn();
    localStorage->HandleActor->SetVisibility(hasIdleHandles);
    // show the selected (green) handle only when one is active this frame; its input is refreshed
    // only on selection, so otherwise a deselected handle lingers with stale geometry
    localStorage->SelectedHandleActor->SetVisibility(selected);

    localStorage->PropAssembly->VisibilityOn();

    localStorage->UpdateGenerateDataTime();
  }
}
vtkProp *mitk::BoundingShapeVtkMapper3D::GetVtkProp(BaseRenderer *renderer)
{
  return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->PropAssembly;
}
