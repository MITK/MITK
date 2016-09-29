/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "../DataManagement/mitkBoundingShapeUtil.h"
#include <vtkAppendPolyData.h>
#include "mitkBoundingShapeVtkMapper3D.h"
#include <mitkBaseProperty.h>
#include <vtkCubeSource.h>
#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkTransformFilter.h>


namespace mitk
{
  class BoundingShapeVtkMapper3D::Impl
  {
  public:
    Impl()
      : DistanceFromCam(1.0)
    {
      Point3D initialPoint;
      initialPoint.Fill(0);

      for (int i = 0; i < 6; ++i)
        HandlePropertyList.push_back(Handle(initialPoint, i, GetHandleIndices(i)));
    }

    double DistanceFromCam;
    std::vector<Handle> HandlePropertyList;
    mitk::LocalStorageHandler<LocalStorage> LocalStorageHandler;
  };
}

mitk::BoundingShapeVtkMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New()), m_HandleActor(vtkSmartPointer<vtkActor>::New()), m_SelectedHandleActor(vtkSmartPointer<vtkActor>::New()), m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
  // initialize handles
  for (int i = 0; i < 6; i++)
  {
    m_Handles.push_back(vtkSmartPointer<vtkSphereSource>::New());
  }

}

mitk::BoundingShapeVtkMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::BoundingShapeVtkMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node == nullptr)
    return;

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::BoundingShapeVtkMapper3D::BoundingShapeVtkMapper3D()
  : m_Impl(new Impl)
{
}

mitk::BoundingShapeVtkMapper3D::~BoundingShapeVtkMapper3D()
{
  delete m_Impl;
}

void mitk::BoundingShapeVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*)
{
}

void mitk::BoundingShapeVtkMapper3D::ApplyBoundingShapeProperties(BaseRenderer* renderer, vtkActor* actor)
{
  if (actor == nullptr)
    return;

  mitk::DataNode* dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  bool render = false;
  dataNode->GetBoolProperty("boundingshape.3drendering", render);

  actor->SetVisibility(render);

  float lineWidth = 1.0f;
  dataNode->GetFloatProperty("boundingshape.line.width", lineWidth, renderer);

  vtkProperty* property = actor->GetProperty();
  property->SetLineWidth(lineWidth);

}

void mitk::BoundingShapeVtkMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  DataNode* node = this->GetDataNode();

  if (node == nullptr)
    return;


  LocalStorage* localStorage = m_Impl->LocalStorageHandler.GetLocalStorage(renderer);

  vtkCamera* cam = 0;
  const mitk::VtkPropRenderer *propRenderer = dynamic_cast<const mitk::VtkPropRenderer * >(renderer);
  if (propRenderer)
  {
    // get vtk renderer
    vtkRenderer* vtkrenderer = propRenderer->GetVtkRenderer();
    if (vtkrenderer)
    {
      // get vtk camera
      vtkCamera* vtkcam = vtkrenderer->GetActiveCamera();
      if (vtkcam)
      {
        // vtk smart pointer handling
        cam = vtkcam;
        cam->Register(nullptr);
      }
    }
  }

  bool needGenerateData = localStorage->GetLastGenerateDataTime() < node->GetMTime();
  double distance = cam->GetDistance();
  if (abs(cam->GetDistance() - m_Impl->DistanceFromCam) > 0.001)
  {
    m_Impl->DistanceFromCam = distance;
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

    // set the input-object at time t for the mapper
    GeometryData* geometryData = dynamic_cast<GeometryData*>(node->GetData());
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
    extent[0] = sqrt((p0[0] - p4[0])*(p0[0] - p4[0]) + (p0[1] - p4[1])*(p0[1] - p4[1]) + (p0[2] - p4[2])*(p0[2] - p4[2]));
    extent[1] = sqrt((p0[0] - p2[0])*(p0[0] - p2[0]) + (p0[1] - p2[1])*(p0[1] - p2[1]) + (p0[2] - p2[2])*(p0[2] - p2[2]));
    extent[2] = sqrt((p0[0] - p1[0])*(p0[0] - p1[0]) + (p0[1] - p1[1])*(p0[1] - p1[1]) + (p0[2] - p1[2])*(p0[2] - p1[2]));

    // calculate center based on half way of the distance between two opposing cornerpoints
    mitk::Point3D center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

    if (m_Impl->HandlePropertyList.size() == 6){
      // set handle positions
      Point3D pointLeft = CalcAvgPoint(cornerPoints[5], cornerPoints[6]);
      Point3D pointRight = CalcAvgPoint(cornerPoints[1], cornerPoints[2]);
      Point3D pointTop = CalcAvgPoint(cornerPoints[0], cornerPoints[6]);
      Point3D pointBottom = CalcAvgPoint(cornerPoints[7], cornerPoints[1]);
      Point3D pointFront = CalcAvgPoint(cornerPoints[2], cornerPoints[7]);
      Point3D pointBack = CalcAvgPoint(cornerPoints[4], cornerPoints[1]);

      m_Impl->HandlePropertyList[0].SetPosition(pointLeft);
      m_Impl->HandlePropertyList[1].SetPosition(pointRight);
      m_Impl->HandlePropertyList[2].SetPosition(pointTop);
      m_Impl->HandlePropertyList[3].SetPosition(pointBottom);
      m_Impl->HandlePropertyList[4].SetPosition(pointFront);
      m_Impl->HandlePropertyList[5].SetPosition(pointBack);
    }

    auto cube = vtkCubeSource::New();
    cube->SetXLength(extent[0] / spacing[0]);
    cube->SetYLength(extent[1] / spacing[1]);
    cube->SetZLength(extent[2] / spacing[2]);

    // calculates translation based on offset+extent not on the transformation matrix
    vtkSmartPointer<vtkMatrix4x4> imageTransform = geometry->GetVtkTransform()->GetMatrix();
    auto translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(center[0] - imageTransform->GetElement(0, 3), center[1] - imageTransform->GetElement(1, 3), center[2] - imageTransform->GetElement(2, 3));

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
      localStorage->m_Actor->VisibilityOff();
      return;
    }

    mitk::DoubleProperty::Pointer handleSizeProperty = dynamic_cast<mitk::DoubleProperty*>(this->GetDataNode()->GetProperty("handle size factor"));

    ScalarType initialHandleSize;
    if (handleSizeProperty != nullptr)
      initialHandleSize = handleSizeProperty->GetValue();
    else
      initialHandleSize = 1.0/40.0;

    double handlesize = ((cam->GetDistance()*std::tan(vtkMath::RadiansFromDegrees(cam->GetViewAngle()))) / 2.0)*initialHandleSize;

    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_HandleActor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_HandleActor);
    if (localStorage->m_PropAssembly->GetParts()->IsItemPresent(localStorage->m_Actor))
      localStorage->m_PropAssembly->RemovePart(localStorage->m_Actor);

    auto selectedhandlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    auto appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();

    mitk::IntProperty::Pointer activeHandleId = dynamic_cast<mitk::IntProperty*>(node->GetProperty("BoundingShapeInteractor.active handle id"));

    int i = 0;
    for (auto &handle : localStorage->m_Handles)
    {
      Point3D handlecenter = m_Impl->HandlePropertyList[i].GetPosition();
      handle->SetCenter(handlecenter[0], handlecenter[1], handlecenter[2]);
      handle->SetRadius(handlesize);
      handle->Update();
      if (activeHandleId == nullptr)
      {
        appendPoly->AddInputConnection(handle->GetOutputPort());
      }
      else
      {
        if (activeHandleId->GetValue() != m_Impl->HandlePropertyList[i].GetIndex())
        {
          appendPoly->AddInputConnection(handle->GetOutputPort());
        }
        else
        {
          selectedhandlemapper->SetInputData(handle->GetOutput());
          localStorage->m_SelectedHandleActor->SetMapper(selectedhandlemapper);
          localStorage->m_SelectedHandleActor->GetProperty()->SetColor(0, 1, 0);
          localStorage->m_SelectedHandleActor->GetMapper()->SetInputDataObject(handle->GetOutput());
          localStorage->m_PropAssembly->AddPart(localStorage->m_SelectedHandleActor);
        }
      }
      i++;
    }
    appendPoly->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);

    auto handlemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    handlemapper->SetInputData(appendPoly->GetOutput());

    localStorage->m_Actor->SetMapper(mapper);
    localStorage->m_Actor->GetMapper()->SetInputDataObject(polydata);
    localStorage->m_Actor->GetProperty()->SetOpacity(0.3);

    mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty*>(node->GetProperty("color"));
    if (selectedColor != nullptr){
      mitk::Color color = selectedColor->GetColor();
      localStorage->m_Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    }

    localStorage->m_HandleActor->SetMapper(handlemapper);
    if (activeHandleId == nullptr)
    {
      localStorage->m_HandleActor->GetProperty()->SetColor(1, 1, 1);
    }
    else
    {
      localStorage->m_HandleActor->GetProperty()->SetColor(1, 0, 0);
    }

    localStorage->m_HandleActor->GetMapper()->SetInputDataObject(appendPoly->GetOutput());

    this->ApplyColorAndOpacityProperties(renderer, localStorage->m_Actor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->m_Actor);

    this->ApplyColorAndOpacityProperties(renderer, localStorage->m_HandleActor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->m_HandleActor);

    this->ApplyColorAndOpacityProperties(renderer, localStorage->m_SelectedHandleActor);
    this->ApplyBoundingShapeProperties(renderer, localStorage->m_SelectedHandleActor);

    // apply properties read from the PropertyList
    // this->ApplyProperties(localStorage->m_Actor, renderer);
    // this->ApplyProperties(localStorage->m_HandleActor, renderer);
    // this->ApplyProperties(localStorage->m_SelectedHandleActor, renderer);

    localStorage->m_Actor->VisibilityOn();
    localStorage->m_HandleActor->VisibilityOn();
    localStorage->m_SelectedHandleActor->VisibilityOn();

    localStorage->m_PropAssembly->AddPart(localStorage->m_Actor);
    localStorage->m_PropAssembly->AddPart(localStorage->m_HandleActor);
    localStorage->m_PropAssembly->VisibilityOn();

    localStorage->UpdateGenerateDataTime();
  }
}
vtkProp* mitk::BoundingShapeVtkMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_Impl->LocalStorageHandler.GetLocalStorage(renderer)->m_PropAssembly;
}

