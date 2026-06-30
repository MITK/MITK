/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBoundingShapeUtil.h"
#include <mitkBaseProperty.h>
#include <mitkBoundingShapeVtkMapper2D.h>

#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCoordinate.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkStripper.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>

#include <array>

namespace mitk
{
  class BoundingShapeVtkMapper2D::Impl
  {
  public:
    Impl()
    {
      Point3D initialPoint;
      initialPoint.Fill(0);

      for (int i = 0; i < 6; ++i)
        HandlePropertyList.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    std::vector<Handle> HandlePropertyList;
    mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
  };
}

mitk::BoundingShapeVtkMapper2D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New()),
  m_HandleActor(vtkSmartPointer<vtkActor2D>::New()),
  m_SelectedHandleActor(vtkSmartPointer<vtkActor2D>::New()),
  m_Mapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
  m_HandleMapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
  m_SelectedHandleMapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
  m_Cutter(vtkSmartPointer<vtkCutter>::New()),
  m_CuttingPlane(vtkSmartPointer<vtkPlane>::New()),
  m_LastSliceNumber(0),
  m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
  m_ZoomFactor(1.0)
{
  m_Actor->SetMapper(m_Mapper);
  m_Actor->VisibilityOn();

  m_HandleActor->SetMapper(m_HandleMapper);
  m_HandleActor->VisibilityOn();

  m_SelectedHandleActor->VisibilityOn();
  m_SelectedHandleActor->GetProperty()->SetColor(0, 1.0, 0);
  m_SelectedHandleActor->SetMapper(m_SelectedHandleMapper);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  m_SelectedHandleMapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();

  m_Cutter->SetCutFunction(m_CuttingPlane);

  for (int i = 0; i < 6; ++i)
    m_Handles.push_back(vtkSmartPointer<vtkCubeSource>::New());

  m_PropAssembly->AddPart(m_Actor);
  m_PropAssembly->AddPart(m_HandleActor);
  m_PropAssembly->VisibilityOn();
}

bool mitk::BoundingShapeVtkMapper2D::LocalStorage::IsUpdateRequired(mitk::BaseRenderer *renderer,
  mitk::Mapper *mapper,
  mitk::DataNode *dataNode)
{
  const mitk::PlaneGeometry *worldGeometry = renderer->GetCurrentWorldPlaneGeometry();

  if (m_LastGenerateDataTime < worldGeometry->GetMTime())
    return true;

  unsigned int sliceNumber = renderer->GetSlice();

  if (m_LastSliceNumber != sliceNumber)
    return true;

  if (mapper && m_LastGenerateDataTime < mapper->GetMTime())
    return true;

  if (dataNode)
  {
    if (m_LastGenerateDataTime < dataNode->GetMTime())
      return true;

    mitk::BaseData *data = dataNode->GetData();

    if (data && m_LastGenerateDataTime < data->GetMTime())
      return true;
  }

  return false;
}

mitk::BoundingShapeVtkMapper2D::LocalStorage::~LocalStorage()
{
}

void mitk::BoundingShapeVtkMapper2D::Update(mitk::BaseRenderer *renderer)
{
  this->GenerateDataForRenderer(renderer);
}

void mitk::BoundingShapeVtkMapper2D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  node->AddProperty("opacity", FloatProperty::New(0.2f), renderer, overwrite);
}

mitk::BoundingShapeVtkMapper2D::BoundingShapeVtkMapper2D() : m_Impl(new Impl)
{
}

mitk::BoundingShapeVtkMapper2D::~BoundingShapeVtkMapper2D()
{
  delete m_Impl;
}

void mitk::BoundingShapeVtkMapper2D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  const DataNode::Pointer node = GetDataNode();
  if (node == nullptr)
    return;

  LocalStorage *localStorage = m_Impl->LocalStorageHandler.GetLocalStorage(renderer);

  // either update if GeometryData was modified or if the zooming was performed
  bool needGenerateData = localStorage->IsUpdateRequired(
    renderer, this, GetDataNode()); // true; // localStorage->GetLastGenerateDataTime() < node->GetMTime() ||
  // localStorage->GetLastGenerateDataTime() < node->GetData()->GetMTime();
  // //localStorage->IsGenerateDataRequired(renderer, this, GetDataNode());

  double scale = renderer->GetScaleFactorMMPerDisplayUnit();

  if (std::abs(scale - localStorage->m_ZoomFactor) > 0.001)
  {
    localStorage->m_ZoomFactor = scale;
    needGenerateData = true;
  }

  if (needGenerateData)
  {

    bool visible = true;
    GetDataNode()->GetVisibility(visible, renderer, "visible");

    if (!visible)
    {
      localStorage->m_Actor->VisibilityOff();
      return;
    }
    GeometryData::Pointer shape = static_cast<GeometryData *>(node->GetData());
    if (shape == nullptr)
      return;

    mitk::BaseGeometry::Pointer geometry = shape->GetGeometry();
    mitk::Vector3D spacing = geometry->GetSpacing();

    // calculate cornerpoints and extent from geometry with visualization offset
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

    // create cube for rendering bounding box
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
    if (polydata == nullptr || (polydata->GetNumberOfPoints() < 1))
    {
      localStorage->m_Actor->VisibilityOff();
      localStorage->m_HandleActor->VisibilityOff();
      localStorage->m_SelectedHandleActor->VisibilityOff();
      return;
    }

    // estimate current image plane to decide whether the cube is visible or not
    const PlaneGeometry *planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
    if ((planeGeometry == nullptr) || (!planeGeometry->IsValid()) || (!planeGeometry->HasReferenceGeometry()))
      return;

    double origin[3];
    origin[0] = planeGeometry->GetOrigin()[0];
    origin[1] = planeGeometry->GetOrigin()[1];
    origin[2] = planeGeometry->GetOrigin()[2];

    double displayPlaneNormal[3];
    displayPlaneNormal[0] = planeGeometry->GetNormal()[0];
    displayPlaneNormal[1] = planeGeometry->GetNormal()[1];
    displayPlaneNormal[2] = planeGeometry->GetNormal()[2];
    vtkMath::Normalize(displayPlaneNormal);

    localStorage->m_CuttingPlane->SetOrigin(origin);
    localStorage->m_CuttingPlane->SetNormal(displayPlaneNormal);

    // add cube polydata to local storage
    localStorage->m_Cutter->SetInputData(polydata);
    localStorage->m_Cutter->SetGenerateCutScalars(1);
    localStorage->m_Cutter->Update();

    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_HandleActor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_HandleActor);
    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_Actor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_Actor);

    vtkCoordinate *tcoord = vtkCoordinate::New();
    tcoord->SetCoordinateSystemToWorld();
    localStorage->m_HandleMapper->SetTransformCoordinate(tcoord);
    tcoord->Delete();

    if (localStorage->m_Cutter->GetOutput()->GetNumberOfPoints() > 0) // if plane is visible in the renderwindow
    {
      mitk::DoubleProperty::Pointer handleSizeProperty =
        dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty("Bounding Shape.Handle Size Factor"));

      ScalarType initialHandleSize;
      if (handleSizeProperty != nullptr)
        initialHandleSize = handleSizeProperty->GetValue();
      else
        initialHandleSize = 0.02;

      mitk::Point2D displaySize = renderer->GetDisplaySizeInMM();
      double handleSize = ((displaySize[0] + displaySize[1]) / 2.0) * initialHandleSize;

      auto appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();
      // add handles and their assigned properties to the local storage
      mitk::IntProperty::Pointer activeHandleId =
        dynamic_cast<mitk::IntProperty *>(node->GetProperty("Bounding Shape.Active Handle ID"));

      // direction cosines of the geometry, used to orient the handle markers with the box
      double dirCos[3][3];
      for (int c = 0; c < 3; ++c)
        for (int r = 0; r < 3; ++r)
          dirCos[r][c] = imageTransform->GetElement(r, c) / spacing[c];

      const Point3D planeOrigin = planeGeometry->GetOrigin();
      const Vector3D planeNormal = planeGeometry->GetNormal();

      bool visible = false;
      bool selected = false;
      for (unsigned int handleIdx = 0; handleIdx < localStorage->m_Handles.size(); ++handleIdx)
      {
        // place the handle where its box face crosses the current slice, so it stays on the
        // rendered cross-section outline for oblique boxes; hide it when the slice misses the face
        const std::array<int, 4> faceCornerIndices = GetHandleFaceCornerIndices(handleIdx);
        const std::array<Point3D, 4> faceCorners = {cornerPoints[faceCornerIndices[0]],
                                                    cornerPoints[faceCornerIndices[1]],
                                                    cornerPoints[faceCornerIndices[2]],
                                                    cornerPoints[faceCornerIndices[3]]};
        Point3D handleCenter;
        if (!GetFacePlaneIntersectionCenter(faceCorners, planeOrigin, planeNormal, handleCenter))
          continue;

        auto &handle = localStorage->m_Handles[handleIdx];
        handle->SetXLength(handleSize);
        handle->SetYLength(handleSize);
        handle->SetZLength(handleSize);
        handle->SetCenter(0.0, 0.0, 0.0);

        // orient the marker with the box and move it onto the handle position
        auto handleMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        handleMatrix->Identity();
        for (int c = 0; c < 3; ++c)
          for (int r = 0; r < 3; ++r)
            handleMatrix->SetElement(r, c, dirCos[r][c]);
        handleMatrix->SetElement(0, 3, handleCenter[0]);
        handleMatrix->SetElement(1, 3, handleCenter[1]);
        handleMatrix->SetElement(2, 3, handleCenter[2]);

        auto handleTransform = vtkSmartPointer<vtkTransform>::New();
        handleTransform->SetMatrix(handleMatrix);

        auto handleTransformFilter = vtkSmartPointer<vtkTransformFilter>::New();
        handleTransformFilter->SetInputConnection(handle->GetOutputPort());
        handleTransformFilter->SetTransform(handleTransform);
        handleTransformFilter->Update();

        auto orientedHandle = vtkSmartPointer<vtkPolyData>::New();
        orientedHandle->DeepCopy(handleTransformFilter->GetPolyDataOutput());

        if (activeHandleId != nullptr && activeHandleId->GetValue() == static_cast<int>(handleIdx))
        {
          localStorage->m_SelectedHandleMapper->SetInputData(orientedHandle);
          localStorage->m_SelectedHandleActor->VisibilityOn();
          selected = true;
        }
        else
        {
          appendPoly->AddInputData(orientedHandle);
        }
        visible = true;
      }

      if (visible)
      {
        appendPoly->Update();
      }
      else
      {
        localStorage->m_HandleActor->VisibilityOff();
        localStorage->m_SelectedHandleActor->VisibilityOff();
      }

      auto stripper = vtkSmartPointer<vtkStripper>::New();
      stripper->SetInputData(localStorage->m_Cutter->GetOutput());
      stripper->Update();

      auto cutPolyData = vtkSmartPointer<vtkPolyData>::New();
      cutPolyData->SetPoints(stripper->GetOutput()->GetPoints());
      cutPolyData->SetPolys(stripper->GetOutput()->GetLines());

      localStorage->m_Actor->GetMapper()->SetInputDataObject(cutPolyData);

      this->ApplyColorAndOpacityProperties(renderer, localStorage->m_Actor);

      if (activeHandleId != nullptr)
      {
        localStorage->m_HandleActor->GetProperty()->SetColor(1, 0, 0);
      }
      else
      {
        localStorage->m_HandleActor->GetProperty()->SetColor(1, 1, 1);
      }
      localStorage->m_HandleActor->GetMapper()->SetInputDataObject(appendPoly->GetOutput());

      // add parts to the overall storage
      localStorage->m_PropAssembly->AddPart(localStorage->m_Actor);
      localStorage->m_PropAssembly->AddPart(localStorage->m_HandleActor);
      if (selected)
      {
        localStorage->m_PropAssembly->AddPart(localStorage->m_SelectedHandleActor);
      }
      // hide the selected (green) handle whenever none is active this frame; its input is only
      // refreshed on selection, so otherwise the last green handle lingers at its old position
      localStorage->m_SelectedHandleActor->SetVisibility(selected);

      localStorage->m_PropAssembly->VisibilityOn();
      localStorage->m_Actor->VisibilityOn();
      localStorage->m_HandleActor->VisibilityOn();
    }
    else
    {
      localStorage->m_PropAssembly->VisibilityOff();
      localStorage->m_Actor->VisibilityOff();
      localStorage->m_HandleActor->VisibilityOff();
      localStorage->m_SelectedHandleActor->VisibilityOff();
      localStorage->UpdateGenerateDataTime();
    }
    localStorage->UpdateGenerateDataTime();
  }
}

vtkProp *mitk::BoundingShapeVtkMapper2D::GetVtkProp(BaseRenderer *renderer)
{
  return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->m_PropAssembly;
}

void mitk::BoundingShapeVtkMapper2D::ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor)
{
  auto* property = actor->GetProperty();

  std::array<float, 3> color = { 1.0, 0.0, 0.0 };
  this->GetDataNode()->GetColor(color.data(), renderer);
  property->SetColor(color[0], color[1], color[2]);

  float opacity = 0.2f;
  this->GetDataNode()->GetOpacity(opacity, renderer);
  property->SetOpacity(opacity);
}
