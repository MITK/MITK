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
#include <vtkPolyDataMapper2D.h>
#include <vtkViewport.h>


vtkStandardNewMacro(vtkMitkRectangleProp);

vtkMitkRectangleProp::vtkMitkRectangleProp()
{
  vtkSmartPointer<vtkPolyDataMapper2D> mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_RectangleActor = vtkSmartPointer<vtkActor2D>::New();
  m_PolyData = vtkSmartPointer<vtkPolyData>::New();

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  m_PolyData->SetPoints(points);
  m_PolyData->SetLines(lines);

  this->CreateRectangle();

  mapper->SetInputData(m_PolyData);
  m_RectangleActor->SetMapper(mapper);
}

vtkMitkRectangleProp::~vtkMitkRectangleProp()
{
}

int vtkMitkRectangleProp::RenderOverlay(vtkViewport* viewport)
{
  double right = viewport->GetSize()[0];
  double top = viewport->GetSize()[1];
//  double bottom = 0.0;
  double left = viewport->GetOrigin()[0] + 0.5;
  double bottom = viewport->GetOrigin()[1] + 0.5;
//  double left = 0.0;
  double defaultDepth = 0.0;

  vtkSmartPointer<vtkPoints> points = m_PolyData->GetPoints();
  //change the corners to adapt the size of the renderwindow was resized
  points->SetPoint(m_BottomLeft, left, bottom, defaultDepth);
  points->SetPoint(m_TopLeft, left, top, defaultDepth);
  points->SetPoint(m_TopRight, right, top, defaultDepth);
  points->SetPoint(m_BottomRight, right, bottom, defaultDepth);

  //adapt color
  m_RectangleActor->GetProperty()->SetColor( m_Color[0], m_Color[1], m_Color[2]);

  //render the actor
  m_RectangleActor->RenderOverlay(viewport);
  return 1;
}

void vtkMitkRectangleProp::CreateRectangle()
{
  vtkSmartPointer<vtkPoints> points = m_PolyData->GetPoints();
  vtkSmartPointer<vtkCellArray> lines = m_PolyData->GetLines();

  //just some default values until a renderwindow/viewport is initialized
  double bottom = 0.0;
  double left = 0.0;
  double right = 200.0;
  double top = 200.0;
  double defaultDepth = 0.0;

  //4 corner points
  m_BottomLeft = points->InsertNextPoint(left, bottom, defaultDepth);
  m_BottomRight = points->InsertNextPoint(right, bottom, defaultDepth);
  m_TopLeft = points->InsertNextPoint(left, top, defaultDepth);
  m_TopRight = points->InsertNextPoint(right, top, defaultDepth);

  vtkSmartPointer<vtkLine> lineVtk =  vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetNumberOfIds(5);
  //connect the lines
  lineVtk->GetPointIds()->SetId(0,m_BottomLeft);
  lineVtk->GetPointIds()->SetId(1,m_BottomRight);
  lineVtk->GetPointIds()->SetId(2,m_TopRight);
  lineVtk->GetPointIds()->SetId(3,m_TopLeft);
  lineVtk->GetPointIds()->SetId(4,m_BottomLeft);

  lines->InsertNextCell(lineVtk);
}

void vtkMitkRectangleProp::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renWin)
{
  m_RenderWindow = renWin;
}

void vtkMitkRectangleProp::SetColor(float col1, float col2, float col3)
{
  m_Color[0] = col1;
  m_Color[1] = col2;
  m_Color[2] = col3;
}
