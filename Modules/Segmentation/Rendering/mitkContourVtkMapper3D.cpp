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


#include "mitkContourVtkMapper3D.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkVtkPropRenderer.h"
#include "mitkContour.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAssembly.h>
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <vtkCellArray.h>
#pragma GCC diagnostic warning "-Wstrict-aliasing"
#include <vtkFollower.h>
#include <vtkLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolygon.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>

mitk::ContourVtkMapper3D::ContourVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_VtkPointList = vtkAppendPolyData::New();
  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);

  m_TubeFilter = vtkTubeFilter::New();
}

mitk::ContourVtkMapper3D::~ContourVtkMapper3D()
{
  if(m_VtkPolyDataMapper)
    m_VtkPolyDataMapper->Delete();

  if(m_TubeFilter)
    m_TubeFilter->Delete();

  if(m_VtkPointList)
    m_VtkPointList->Delete();

  if(m_Contour)
    m_Contour->Delete();

  if(m_Actor)
    m_Actor->Delete();
}

vtkProp* mitk::ContourVtkMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_Actor;
}

void mitk::ContourVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible )
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();

  m_Contour = vtkPolyData::New();

  mitk::Contour::Pointer input  = const_cast<mitk::Contour*>(this->GetInput());
  bool makeContour = true;

  if ( makeContour )
  {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    int numPts=input->GetNumberOfPoints();
    if ( numPts > 200000 )
      numPts = 200000;
    mitk::Contour::PathPointer path = input->GetContourPath();
    mitk::Contour::PathType::InputType cstart = path->StartOfInput();
    mitk::Contour::PathType::InputType cend   = path->EndOfInput();
    mitk::Contour::PathType::InputType cstep  = (cend-cstart+1)/numPts;
    mitk::Contour::PathType::InputType ccur;

    vtkIdType ptIndex = 0;
    vtkIdType lastPointIndex = 0;

    mitk::Contour::PointsContainerPointer contourPoints = input->GetPoints();

    double vtkpoint[3];

    int i;
    float pointSize = 2;
    this->GetDataNode()->GetFloatProperty("spheres size", pointSize);

    bool showPoints = true;
    this->GetDataNode()->GetBoolProperty("show points", showPoints);
    if ( showPoints )
    {
      m_VtkPointList = vtkAppendPolyData::New();
    }
    for ( i=0, ccur=cstart; i<numPts; ++i, ccur+=cstep )
    {
      itk2vtk(path->Evaluate(ccur), vtkpoint);
      points->InsertPoint(ptIndex, vtkpoint);
      if ( ptIndex > 0 )
      {
        vtkIdType cell[2] = {ptIndex-1,ptIndex};
        lines->InsertNextCell((vtkIdType)2,cell);
      }
      lastPointIndex = ptIndex;
      ++ptIndex;

      if ( showPoints )
      {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();

        sphere->SetRadius(pointSize);
        sphere->SetCenter(vtkpoint);

        m_VtkPointList->AddInputConnection(sphere->GetOutputPort());
        sphere->Update();
      }
    }

    if ( input->GetClosed() )
    {
      vtkIdType cell[2] = {lastPointIndex,0};
      lines->InsertNextCell((vtkIdType)2, cell);
    }

    m_Contour->SetPoints(points);
    m_Contour->SetLines(lines);

    m_TubeFilter->SetInputData(m_Contour);
    m_TubeFilter->SetRadius(pointSize / 2.0f);
    m_TubeFilter->SetNumberOfSides(8);
    m_TubeFilter->Update();

    if ( showPoints )
    {
      m_VtkPointList->AddInputConnection(m_TubeFilter->GetOutputPort());
      m_VtkPolyDataMapper->SetInputConnection(m_VtkPointList->GetOutputPort());
    }
    else
    {
      m_VtkPolyDataMapper->SetInputConnection(m_TubeFilter->GetOutputPort());
    }
    double rgba[4]={0.0f,1.0f,0.0f,0.6f};
    m_Actor->GetProperty()->SetColor(rgba);
    m_Actor->SetMapper(m_VtkPolyDataMapper);
  }
}

const mitk::Contour* mitk::ContourVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Contour* > ( GetDataNode()->GetData() );
}
