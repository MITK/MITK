/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrosshairVtkMapper2D.h"

// mitk includes
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkProperties.h>

// vtk includes
#include <mitkIPropertyAliases.h>
#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>

namespace
{
  void SwapInvertedCoordinates(mitk::Point2D& displayBoundsMinimum, mitk::Point2D& displayBoundsMaximum)
  {
    for (int i = 0; i < 2; ++i)
    {
      if (displayBoundsMaximum[i] < displayBoundsMinimum[i])
      {
        auto temporarySwapVariable = displayBoundsMaximum[i];
        displayBoundsMaximum[i] = displayBoundsMinimum[i];
        displayBoundsMinimum[i] = temporarySwapVariable;
      }
    }
  }
}

const mitk::CrosshairData* mitk::CrosshairVtkMapper2D::GetInput() const
{
  return static_cast<CrosshairData*>(this->GetDataNode()->GetData());
}

mitk::CrosshairVtkMapper2D::CrosshairVtkMapper2D()
  : defaultGapSize(32)
{
}

mitk::CrosshairVtkMapper2D::~CrosshairVtkMapper2D()
{
}

void mitk::CrosshairVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  const auto* node = this->GetDataNode();
  if (nullptr == node)
  {
    return;
  }

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if (!visible)
  {
    // Do not render crosshair for base renderer, if the
    // crosshair data node is not visible in that base renderer.
    return;
  }

  const auto* data = static_cast<mitk::CrosshairData*>(node->GetData());
  if (nullptr == data)
  {
    // Do not render crosshair for base renderer, if the
    // base data node is no crosshair data.
    return;
  }

  this->GenerateDataForRenderer(renderer);
}

vtkProp* mitk::CrosshairVtkMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_CrosshairAssembly;
}

void mitk::CrosshairVtkMapper2D::SetDefaultProperties(DataNode* node,
                                                      BaseRenderer* renderer,
                                                      bool overwrite)
{
  CoreServicePointer<IPropertyAliases> aliases(CoreServices::GetPropertyAliases());
  node->AddProperty("Line width", FloatProperty::New(1), renderer, overwrite);
  aliases->AddAlias("line width", "Crosshair.Line Width", "");
  node->AddProperty("Crosshair.Gap Size", IntProperty::New(32), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::CrosshairVtkMapper2D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  ls->UpdateGenerateDataTime();

  this->CreateVtkCrosshair(renderer);

  this->ApplyAllProperties(renderer);
}

void mitk::CrosshairVtkMapper2D::CreateVtkCrosshair(BaseRenderer* renderer)
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  ls->m_CrosshairActor->SetVisibility(0);

  CrosshairData::ConstPointer input = this->GetInput();
  Point3D crosshairPosition = input->GetPosition();

  // check if node is inside the renderer geometry
  bool isInside = renderer->GetWorldTimeGeometry()->IsWorldPointInside(crosshairPosition);
  if (!isInside)
  {
    // no need to draw crosshair lines
    return;
  }

  int gapSize = defaultGapSize;
  this->GetDataNode()->GetPropertyValue("Crosshair.Gap Size", gapSize, nullptr);

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();

  // convert 3D crosshair crosshairPosition to 2D point
  Point2D displayPoint;
  renderer->WorldToDisplay(crosshairPosition, displayPoint);
  Point2D displayPointOne = displayPoint;
  Point2D displayPointTwo = displayPoint;
  Point3D worldPointOne;
  Point3D worldPointTwo;

  // convert geometry bounds from 3D to 2D
  const auto worldBounds = renderer->GetWorldTimeGeometry()->GetBoundsInWorld();
  Point3D worldBoundsMinimum;
  worldBoundsMinimum[0] = worldBounds[0];
  worldBoundsMinimum[1] = worldBounds[2];
  worldBoundsMinimum[2] = worldBounds[4];

  Point3D worldBoundsMaximum;
  worldBoundsMaximum[0] = worldBounds[1];
  worldBoundsMaximum[1] = worldBounds[3];
  worldBoundsMaximum[2] = worldBounds[5];

  Point2D displayBoundsMinimum;
  Point2D displayBoundsMaximum;
  renderer->WorldToDisplay(worldBoundsMinimum, displayBoundsMinimum);
  renderer->WorldToDisplay(worldBoundsMaximum, displayBoundsMaximum);

  SwapInvertedCoordinates(displayBoundsMinimum, displayBoundsMaximum);

  // define points for the first / left half of the crosshair x-line
  displayPointOne[0] = displayBoundsMinimum[0];
  displayPointTwo[0] = displayPoint[0] - gapSize;
  renderer->DisplayToWorld(displayPointOne, worldPointOne);
  renderer->DisplayToWorld(displayPointTwo, worldPointTwo);
  this->DrawLine(worldPointOne, worldPointTwo, lines, points);

  // define points for the second / right half of the crosshair x-line
  displayPointOne[0] = displayPoint[0] + gapSize;
  displayPointTwo[0] = displayBoundsMaximum[0];
  renderer->DisplayToWorld(displayPointOne, worldPointOne);
  renderer->DisplayToWorld(displayPointTwo, worldPointTwo);
  this->DrawLine(worldPointOne, worldPointTwo, lines, points);

  // reset 2D points
  displayPointOne = displayPoint;
  displayPointTwo = displayPoint;

  // define points for the first / bottom half of the crosshair y-line
  displayPointOne[1] = displayBoundsMinimum[1];
  displayPointTwo[1] = displayPoint[1] - gapSize;
  renderer->DisplayToWorld(displayPointOne, worldPointOne);
  renderer->DisplayToWorld(displayPointTwo, worldPointTwo);
  this->DrawLine(worldPointOne, worldPointTwo, lines, points);

  // define points for the second / top half of the crosshair y-line
  displayPointOne[1] = displayPoint[1] + gapSize;
  displayPointTwo[1] = displayBoundsMaximum[1];
  renderer->DisplayToWorld(displayPointOne, worldPointOne);
  renderer->DisplayToWorld(displayPointTwo, worldPointTwo);
  this->DrawLine(worldPointOne, worldPointTwo, lines, points);

  // set vtk data with the created points and lines
  linesPolyData->SetPoints(points);
  linesPolyData->SetLines(lines);
  ls->m_Mapper->SetInputData(linesPolyData);
  ls->m_CrosshairActor->SetMapper(ls->m_Mapper);
  ls->m_CrosshairActor->SetVisibility(1);
}

void mitk::CrosshairVtkMapper2D::DrawLine(Point3D p0, Point3D p1, vtkCellArray* lines, vtkPoints* points)
{
  vtkIdType pidStart = points->InsertNextPoint(p0[0], p0[1], p0[2]);
  vtkIdType pidEnd = points->InsertNextPoint(p1[0], p1[1], p1[2]);

  vtkSmartPointer<vtkLine> lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0, pidStart);
  lineVtk->GetPointIds()->SetId(1, pidEnd);

  lines->InsertNextCell(lineVtk);
}

void mitk::CrosshairVtkMapper2D::ApplyAllProperties(BaseRenderer* renderer)
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_CrosshairActor);

  float thickness;
  this->GetDataNode()->GetFloatProperty("Line width", thickness, renderer);
  ls->m_CrosshairActor->GetProperty()->SetLineWidth(thickness);
}

void mitk::CrosshairVtkMapper2D::ApplyColorAndOpacityProperties2D(BaseRenderer* renderer, vtkActor2D* actor)
{
  float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  DataNode* node = GetDataNode();

  // check for color prop and use it for rendering if it exists
  node->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  node->GetOpacity(rgba[3], renderer, "opacity");

  double drgba[4] = { rgba[0], rgba[1], rgba[2], rgba[3] };
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
}

mitk::CrosshairVtkMapper2D::LocalStorage::LocalStorage()
{
  m_CrosshairAssembly = vtkSmartPointer<vtkPropAssembly>::New();
  m_CrosshairActor = vtkSmartPointer<vtkActor2D>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();

  m_CrosshairActor->SetMapper(m_Mapper);
  m_CrosshairActor->SetVisibility(0);
  m_CrosshairAssembly->AddPart(m_CrosshairActor);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  m_Mapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();
}

mitk::CrosshairVtkMapper2D::LocalStorage::~LocalStorage()
{
}
