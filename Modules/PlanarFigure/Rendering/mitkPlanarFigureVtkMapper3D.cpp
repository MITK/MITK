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

#include "mitkPlanarFigureVtkMapper3D.h"
#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkAbstractTransformGeometry.h"
#include <mitkPlanarFigure.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>

mitk::PlanarFigureVtkMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New()),
    m_LastMTime(0)
{
}

mitk::PlanarFigureVtkMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(DataNode*, BaseRenderer*, bool)
{
}

mitk::PlanarFigureVtkMapper3D::PlanarFigureVtkMapper3D()
{
}

mitk::PlanarFigureVtkMapper3D::~PlanarFigureVtkMapper3D()
{
}

void mitk::PlanarFigureVtkMapper3D::ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor)
{
  if (actor == NULL)
    return;

  mitk::DataNode* dataNode = this->GetDataNode();

  if (dataNode == NULL)
    return;

  bool selected = false;
  dataNode->GetBoolProperty("selected", selected, renderer);

  float color[3];
  dataNode->GetColor(color, renderer, selected ? "planarfigure.selected.line.color" : "color");

  float opacity = 1.0f;
  dataNode->GetOpacity(opacity, renderer);

  vtkProperty* property = actor->GetProperty();
  property->SetColor(color[0], color[1], color[2]);
  property->SetOpacity(opacity);
}

void mitk::PlanarFigureVtkMapper3D::ApplyPlanarFigureProperties(BaseRenderer* renderer, vtkActor* actor)
{
  if (actor == NULL)
    return;

  mitk::DataNode* dataNode = this->GetDataNode();

  if (dataNode == NULL)
    return;

  bool render = false;
  dataNode->GetBoolProperty("planarfigure.3drendering", render);

  actor->SetVisibility(render);

  float lineWidth = 1.0f;
  dataNode->GetFloatProperty("planarfigure.line.width", lineWidth, renderer);

  vtkProperty* property = actor->GetProperty();
  property->SetLineWidth(lineWidth);
}

void mitk::PlanarFigureVtkMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  typedef PlanarFigure::PolyLineType PolyLine;

  DataNode* node = this->GetDataNode();

  if (node == NULL)
    return;

  PlanarFigure* planarFigure = dynamic_cast<PlanarFigure*>(node->GetData());

  if (planarFigure == NULL || !planarFigure->IsPlaced())
    return;

  LocalStorage* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  unsigned long mTime = planarFigure->GetMTime();

  if (mTime > localStorage->m_LastMTime)
  {
    localStorage->m_LastMTime = mTime;

    const PlaneGeometry* planeGeometry = dynamic_cast<const PlaneGeometry*>(planarFigure->GetPlaneGeometry());
    const AbstractTransformGeometry* abstractTransformGeometry = dynamic_cast<const AbstractTransformGeometry*>(planarFigure->GetPlaneGeometry());

    if (planeGeometry == NULL && abstractTransformGeometry == NULL)
      return;

    size_t numPolyLines = planarFigure->GetPolyLinesSize();

    if (numPolyLines == 0)
      return;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType baseIndex = 0;

    for (size_t i = 0; i < numPolyLines; ++i)
    {
      PolyLine polyLine = planarFigure->GetPolyLine(i);
      vtkIdType numPoints = polyLine.size();

      if (numPoints < 2)
        continue;

      PolyLine::const_iterator polyLineEnd = polyLine.end();

      for (PolyLine::const_iterator polyLineIt = polyLine.begin(); polyLineIt != polyLineEnd; ++polyLineIt)
      {
        Point3D point;
        planeGeometry->Map(*polyLineIt, point);
        points->InsertNextPoint(point.GetDataPointer());
      }

      vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();

      vtkIdList* pointIds = line->GetPointIds();

      if (planarFigure->IsClosed() && numPoints > 2)
      {
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

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    localStorage->m_Actor->SetMapper(mapper);
  }

  this->ApplyColorAndOpacityProperties(renderer, localStorage->m_Actor);
  this->ApplyPlanarFigureProperties(renderer, localStorage->m_Actor);
}

vtkProp* mitk::PlanarFigureVtkMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}

void mitk::PlanarFigureVtkMapper3D::UpdateVtkTransform(BaseRenderer*)
{
}
