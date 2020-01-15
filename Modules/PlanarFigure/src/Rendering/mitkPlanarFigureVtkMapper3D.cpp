/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureVtkMapper3D.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include <mitkPlanarFigure.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkPolygon.h>

mitk::PlanarFigureVtkMapper3D::LocalStorage::LocalStorage() : m_Actor(vtkSmartPointer<vtkActor>::New()), m_LastMTime(0)
{
}

mitk::PlanarFigureVtkMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(DataNode *node, BaseRenderer *, bool)
{
  node->AddProperty("planarfigure.3drendering", mitk::BoolProperty::New(false));
  node->AddProperty("planarfigure.3drendering.fill", mitk::BoolProperty::New(false));
}

mitk::PlanarFigureVtkMapper3D::PlanarFigureVtkMapper3D() : m_FillPf(false)
{
}

mitk::PlanarFigureVtkMapper3D::~PlanarFigureVtkMapper3D()
{
}

void mitk::PlanarFigureVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor)
{
  if (actor == nullptr)
    return;

  const mitk::DataNode *dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  bool selected = false;
  dataNode->GetBoolProperty("selected", selected, renderer);

  float color[3];
  dataNode->GetColor(color, renderer, selected ? "planarfigure.selected.line.color" : "color");

  float opacity = 1.0f;
  dataNode->GetOpacity(opacity, renderer);

  vtkProperty *property = actor->GetProperty();
  property->SetColor(color[0], color[1], color[2]);
  property->SetOpacity(opacity);
}

void mitk::PlanarFigureVtkMapper3D::ApplyPlanarFigureProperties(BaseRenderer *renderer, vtkActor *actor)
{
  if (actor == nullptr)
    return;

  const mitk::DataNode *dataNode = this->GetDataNode();

  if (dataNode == nullptr)
    return;

  bool render = false;
  dataNode->GetBoolProperty("planarfigure.3drendering", render);

  actor->SetVisibility(render);

  float lineWidth = 1.0f;
  dataNode->GetFloatProperty("planarfigure.line.width", lineWidth, renderer);

  vtkProperty *property = actor->GetProperty();
  property->SetLineWidth(lineWidth);
}

void mitk::PlanarFigureVtkMapper3D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  typedef PlanarFigure::PolyLineType PolyLine;

  const DataNode *node = this->GetDataNode();

  if (node == nullptr)
    return;

  auto *planarFigure = dynamic_cast<PlanarFigure *>(node->GetData());

  if (planarFigure == nullptr || !planarFigure->IsPlaced())
    return;

  LocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  unsigned long mTime = planarFigure->GetMTime();

  bool fillPf = false;
  bool refresh = false;
  node->GetBoolProperty("planarfigure.3drendering.fill", fillPf);
  if (m_FillPf != fillPf)
  {
    m_FillPf = fillPf;
    refresh = true;
  }

  if (mTime > localStorage->m_LastMTime || refresh)
  {
    localStorage->m_LastMTime = mTime;

    const auto *planeGeometry = dynamic_cast<const PlaneGeometry *>(planarFigure->GetPlaneGeometry());
    const auto *abstractTransformGeometry =
      dynamic_cast<const AbstractTransformGeometry *>(planarFigure->GetPlaneGeometry());

    if (planeGeometry == nullptr && abstractTransformGeometry == nullptr)
      return;

    const size_t numPolyLines = planarFigure->GetPolyLinesSize();

    if (numPolyLines == 0)
      return;

    const vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    const vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType baseIndex = 0;

    for (size_t i = 0; i < numPolyLines; ++i)
    {
      const PolyLine polyLine = planarFigure->GetPolyLine(i);
      const vtkIdType numPoints = polyLine.size();

      vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();

      if (numPoints < 2)
        continue;

      auto polyLineEnd = polyLine.cend();

      for (auto polyLineIt = polyLine.cbegin(); polyLineIt != polyLineEnd; ++polyLineIt)
      {
        Point3D point;
        planeGeometry->Map(*polyLineIt, point);
        points->InsertNextPoint(point.GetDataPointer());

        const vtkIdType id = polygon->GetPoints()->InsertNextPoint(point[0], point[1], point[2]);
        polygon->GetPointIds()->InsertNextId(id);
      }

      vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();

      vtkIdList *pointIds = line->GetPointIds();

      if (planarFigure->IsClosed() && numPoints > 2)
      {
        polygons->InsertNextCell(polygon);
        pointIds->SetNumberOfIds(numPoints + 1);
        pointIds->SetId(numPoints, baseIndex);
      }
      else
      {
        pointIds->SetNumberOfIds(numPoints);
      }

      for (vtkIdType j = 0; j < numPoints; ++j)
        pointIds->SetId(j, baseIndex + j);

      cells->InsertNextCell(line);

      baseIndex += points->GetNumberOfPoints();
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(cells);
    if (m_FillPf)
      polyData->SetPolys(polygons);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    localStorage->m_Actor->SetMapper(mapper);
  }

  this->ApplyColorAndOpacityProperties(renderer, localStorage->m_Actor);
  this->ApplyPlanarFigureProperties(renderer, localStorage->m_Actor);
}

vtkProp *mitk::PlanarFigureVtkMapper3D::GetVtkProp(BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}

void mitk::PlanarFigureVtkMapper3D::UpdateVtkTransform(BaseRenderer *)
{
}
