/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "vtkMitkRectangleProp.h"

#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkViewport.h>

vtkStandardNewMacro(vtkMitkRectangleProp);

vtkMitkRectangleProp::vtkMitkRectangleProp()
  : m_Height(0),
    m_Width(0),
    m_OriginX(0),
    m_OriginY(0),
    m_PolyData(vtkSmartPointer<vtkPolyData>::New())
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  m_PolyData->SetPoints(points);

  auto lines = vtkSmartPointer<vtkCellArray>::New();
  m_PolyData->SetLines(lines);

  auto mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();

  auto transformCoordinate = vtkSmartPointer<vtkCoordinate>::New();
  transformCoordinate->SetCoordinateSystemToDisplay();
  mapper->SetTransformCoordinate(transformCoordinate);

  this->CreateRectangle();

  mapper->SetInputData(m_PolyData);
  this->SetMapper(mapper);

  this->GetProperty()->SetLineWidth(2);
}

vtkMitkRectangleProp::~vtkMitkRectangleProp()
{
}

int vtkMitkRectangleProp::RenderOverlay(vtkViewport *viewport)
{
  if (nullptr == this->Mapper)
  {
    vtkErrorMacro(<< "vtkActor2D::Render - No mapper set");
    return 0;
  }

  if (!GetVisibility())
    return 0;

  if (viewport->GetSize()[0] != m_Width || viewport->GetSize()[1] != m_Height)
  {
    m_Width = viewport->GetSize()[0];
    m_Height = viewport->GetSize()[1];
    m_OriginX = viewport->GetOrigin()[0];
    m_OriginY = viewport->GetOrigin()[1];
    this->UpdateRectangle();
  }

  this->Mapper->RenderOverlay(viewport, this);
  return 1;
}

void vtkMitkRectangleProp::UpdateRectangle()
{
  float offset = this->GetProperty()->GetLineWidth() * 0.5f;

  auto points = m_PolyData->GetPoints();
  points->SetPoint(m_BottomLeft, m_OriginX + offset, m_OriginY + offset, 0.0f);
  points->SetPoint(m_BottomRight, m_OriginX + m_Width - offset, m_OriginY + offset, 0.0f);
  points->SetPoint(m_TopRight, m_OriginX + m_Width - offset, m_OriginY + m_Height - offset, 0.0f);
  points->SetPoint(m_TopLeft, m_OriginX + offset, m_OriginY + m_Height - offset, 0.0f);
}

void vtkMitkRectangleProp::CreateRectangle()
{
  auto points = m_PolyData->GetPoints();
  auto lines = m_PolyData->GetLines();

  m_BottomLeft = points->InsertNextPoint(0.0f, 0.0f, 0.0f);
  m_BottomRight = points->InsertNextPoint(1.0f, 0.0f, 0.0f);
  m_TopRight = points->InsertNextPoint(1.0f, 1.0f, 0.0f);
  m_TopLeft = points->InsertNextPoint(0.0f, 1.0f, 0.0f);

  auto line = vtkSmartPointer<vtkLine>::New();
  line->GetPointIds()->SetId(0, m_BottomLeft);
  line->GetPointIds()->SetId(1, m_BottomRight);
  lines->InsertNextCell(line);

  line = vtkSmartPointer<vtkLine>::New();
  line->GetPointIds()->SetId(0, m_BottomRight);
  line->GetPointIds()->SetId(1, m_TopRight);
  lines->InsertNextCell(line);

  line = vtkSmartPointer<vtkLine>::New();
  line->GetPointIds()->SetId(0, m_TopRight);
  line->GetPointIds()->SetId(1, m_TopLeft);
  lines->InsertNextCell(line);

  line = vtkSmartPointer<vtkLine>::New();
  line->GetPointIds()->SetId(0, m_TopLeft);
  line->GetPointIds()->SetId(1, m_BottomLeft);
  lines->InsertNextCell(line);
}

void vtkMitkRectangleProp::SetColor(float red, float green, float blue)
{
  this->GetProperty()->SetColor(red, green, blue);
}

void vtkMitkRectangleProp::SetLineWidth(unsigned int lineWidth)
{
  this->GetProperty()->SetLineWidth(lineWidth);
}
