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

#include "vtkMitkRectangleProp.h"

#include <vtkObjectFactory.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkActor2D.h>
#include <vtkOpenGLPolyDataMapper2D.h>
#include <vtkViewport.h>


vtkStandardNewMacro(vtkMitkRectangleProp);

vtkMitkRectangleProp::vtkMitkRectangleProp():
  m_Height(0),
  m_Width(0),
  m_OriginX(0),
  m_OriginY(0)
{
  vtkSmartPointer<vtkOpenGLPolyDataMapper2D> mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper2D>::New();
  m_PolyData = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  m_PolyData->SetPoints(points);
  m_PolyData->SetLines(lines);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToDisplay();
  mapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();

  CreateRectangle();

  mapper->SetInputData(m_PolyData);
  SetMapper(mapper);
  GetProperty()->SetLineWidth(2);
}

vtkMitkRectangleProp::~vtkMitkRectangleProp()
{
}

int vtkMitkRectangleProp::RenderOverlay(vtkViewport* viewport)
{
  if (!this->Mapper)
  {
    vtkErrorMacro(<< "vtkActor2D::Render - No mapper set");
    return 0;
  }

  if(!GetVisibility()) return 0;
  if(viewport->GetSize()[0] != m_Width || viewport->GetSize()[1] != m_Height)
  {
    m_Width = viewport->GetSize()[0];
    m_Height = viewport->GetSize()[1];
    m_OriginX = viewport->GetOrigin()[0];
    m_OriginY = viewport->GetOrigin()[1];
    UpdateRectangle();
  }
  this->Mapper->RenderOverlay(viewport, this);

  return 1;
}

void vtkMitkRectangleProp::UpdateRectangle()
{
  vtkSmartPointer<vtkPoints> points = m_PolyData->GetPoints();
  float offset = (GetProperty()->GetLineWidth()-1.);
  float wLine = m_OriginX+m_Width-1;
  float hLine = m_OriginY+m_Height-1;
  float offX = m_OriginX+offset;
  float offY = m_OriginY+offset;

  points->SetPoint(m_BottomLeftR  ,m_OriginX, offY, 0.0);
  points->SetPoint(m_BottomRightL ,m_Width+m_OriginX, offY, 0.0);

  points->SetPoint(m_BottomLeftU  ,offX, m_OriginY, 0.0);
  points->SetPoint(m_TopLeftD     ,offX, m_OriginY+m_Height, 0.0);

  points->SetPoint(m_TopLeftR     ,m_OriginX, hLine, 0.0);
  points->SetPoint(m_TopRightL    ,m_Width+m_OriginX, hLine, 0.0);

  points->SetPoint(m_TopRightD    ,wLine, m_OriginY+m_Height, 0.0);
  points->SetPoint(m_BottomRightU ,wLine, m_OriginY, 0.0);
}

void vtkMitkRectangleProp::CreateRectangle()
{
  vtkSmartPointer<vtkPoints> points = m_PolyData->GetPoints();
  vtkSmartPointer<vtkCellArray> lines = m_PolyData->GetLines();

  //4 corner points
  m_BottomLeftR  = points->InsertNextPoint(0.0, 0.0, 0.0);
  m_BottomRightL = points->InsertNextPoint(1.0, 0.0, 0.0);

  m_BottomLeftU  = points->InsertNextPoint(0.0, 0.0, 0.0);
  m_TopLeftD     = points->InsertNextPoint(0.0, 1.0, 0.0);

  m_TopLeftR     = points->InsertNextPoint(0.0, 1.0, 0.0);
  m_TopRightL    = points->InsertNextPoint(1.0, 1.0, 0.0);

  m_TopRightD    = points->InsertNextPoint(1.0, 1.0, 0.0);
  m_BottomRightU = points->InsertNextPoint(1.0, 0.0, 0.0);

  vtkSmartPointer<vtkLine> lineVtk;
  lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0,m_BottomLeftR);
  lineVtk->GetPointIds()->SetId(1,m_BottomRightL);
  lines->InsertNextCell(lineVtk);

  lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0,m_BottomLeftU);
  lineVtk->GetPointIds()->SetId(1,m_TopLeftD);
  lines->InsertNextCell(lineVtk);

  lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0,m_TopLeftR);
  lineVtk->GetPointIds()->SetId(1,m_TopRightL);
  lines->InsertNextCell(lineVtk);

  lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0,m_TopRightD);
  lineVtk->GetPointIds()->SetId(1,m_BottomRightU);
  lines->InsertNextCell(lineVtk);
}

void vtkMitkRectangleProp::SetColor(float col1, float col2, float col3)
{
  GetProperty()->SetColor(col1,col2,col3);
}

void vtkMitkRectangleProp::SetLineWidth(unsigned int lineWidth)
{
  GetProperty()->SetLineWidth(lineWidth);
}
