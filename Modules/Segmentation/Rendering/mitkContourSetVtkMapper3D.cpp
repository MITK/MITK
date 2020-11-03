/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourSetVtkMapper3D.h"
#include "mitkColorProperty.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkVtkPropRenderer.h"

#include <vtkActor.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAssembly.h>
#include <vtkCellArray.h>
#include <vtkFollower.h>
#include <vtkLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolygon.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>
#include <vtkTubeFilter.h>

#include <cstdlib>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

mitk::ContourSetVtkMapper3D::ContourSetVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);

  m_ContourSet = vtkPolyData::New();
  m_TubeFilter = vtkTubeFilter::New();
}

mitk::ContourSetVtkMapper3D::~ContourSetVtkMapper3D()
{
  if (m_VtkPolyDataMapper)
    m_VtkPolyDataMapper->Delete();
  ;

  if (m_TubeFilter)
    m_TubeFilter->Delete();
  ;

  if (m_ContourSet)
    m_ContourSet->Delete();
  ;

  if (m_Actor)
    m_Actor->Delete();
  ;
}

vtkProp *mitk::ContourSetVtkMapper3D::GetVtkProp(mitk::BaseRenderer * /*renderer*/)
{
  return m_Actor;
}

void mitk::ContourSetVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
  {
    m_Actor->VisibilityOff();
    return;
  }

  m_Actor->VisibilityOn();

  if (renderer->GetCurrentWorldPlaneGeometryUpdateTime() > 0UL && renderer->GetCurrentWorldPlaneGeometryUpdateTime() < this->GetInput()->GetMTime())
  {
    m_ContourSet = vtkPolyData::New();

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();

    mitk::ContourSet::Pointer input = const_cast<mitk::ContourSet *>(this->GetInput());
    mitk::ContourSet::ContourVectorType contourVec = input->GetContours();
    auto contourIt = contourVec.begin();

    vtkIdType firstPointIndex = 0, lastPointIndex = 0;

    vtkIdType ptIndex = 0;
    while (contourIt != contourVec.end())
    {
      auto *nextContour = (mitk::Contour *)(*contourIt).second;
      Contour::InputType idx = nextContour->GetContourPath()->StartOfInput();

      Contour::InputType end = nextContour->GetContourPath()->EndOfInput();
      if (end > 50000)
        end = 0;

      mitk::Contour::PointsContainerPointer contourPoints = nextContour->GetPoints();
      mitk::Contour::PointsContainerIterator pointsIt = contourPoints->Begin();
      unsigned int counter = 0;

      firstPointIndex = ptIndex;
      while (pointsIt != contourPoints->End())
      {
        if (counter % 2 == 0)
        {
          Contour::BoundingBoxType::PointType point;
          point = pointsIt.Value();
          points->InsertPoint(ptIndex, point[0], point[1], point[2]);
          if (ptIndex > firstPointIndex)
          {
            vtkIdType cell[2] = {ptIndex - 1, ptIndex};
            lines->InsertNextCell((vtkIdType)2, cell);
          }
          lastPointIndex = ptIndex;
          ptIndex++;
        }
        pointsIt++;
        idx += 1;
      }

      if (nextContour->GetClosed())
      {
        vtkIdType cell[2] = {lastPointIndex, firstPointIndex};
        lines->InsertNextCell((vtkIdType)2, cell);
      }
      contourIt++;
    }

    m_ContourSet->SetPoints(points);
    m_ContourSet->SetLines(lines);

    m_TubeFilter->SetInputData(m_ContourSet);
    m_TubeFilter->SetRadius(1);
    m_TubeFilter->Update();
    m_VtkPolyDataMapper->SetInputConnection(m_TubeFilter->GetOutputPort());

    double rgba[4] = {0.0f, 1.0f, 0.0f, 0.6f};

    m_Actor->GetProperty()->SetColor(rgba);
    m_Actor->SetMapper(m_VtkPolyDataMapper);
  }
}

const mitk::ContourSet *mitk::ContourSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::ContourSet *>(GetDataNode()->GetData());
}
