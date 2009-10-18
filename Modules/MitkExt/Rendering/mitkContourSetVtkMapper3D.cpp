/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkContourSetVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkVtkPropRenderer.h"


#include <vtkActor.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>
#include <vtkLinearTransform.h>
#include <vtkTubeFilter.h>
#include <vtkPolygon.h>


#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <stdlib.h>


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
  m_VtkPolyDataMapper->Delete();
}

vtkProp* mitk::ContourSetVtkMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_Actor;
}

void mitk::ContourSetVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();
  
  mitk::ContourSet::Pointer input  = const_cast<mitk::ContourSet*>(this->GetInput());

  if ( renderer->GetDisplayGeometryUpdateTime() > this->GetInput()->GetMTime() )
  {
    m_ContourSet = vtkPolyData::New();

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();

    mitk::ContourSet::Pointer input =  const_cast<mitk::ContourSet*>(this->GetInput());
    mitk::ContourSet::ContourVectorType contourVec = input->GetContours();
    mitk::ContourSet::ContourIterator contourIt = contourVec.begin();
    
    vtkIdType firstPointIndex= 0, lastPointIndex=0;

    vtkIdType ptIndex = 0;
    while ( contourIt != contourVec.end() )
    {
      mitk::Contour* nextContour = (mitk::Contour*) (*contourIt).second;
      Contour::InputType idx = nextContour->GetContourPath()->StartOfInput();


      Contour::InputType end = nextContour->GetContourPath()->EndOfInput();
      if (end > 50000) end = 0;

      mitk::Contour::PointsContainerPointer contourPoints = nextContour->GetPoints();
      mitk::Contour::PointsContainerIterator pointsIt = contourPoints->Begin();
      unsigned int counter = 0;

      firstPointIndex=ptIndex;
      while ( pointsIt != contourPoints->End() )
      {
        if (counter %2 == 0)
        {        
          Contour::BoundingBoxType::PointType point;
          point = pointsIt.Value();
          points->InsertPoint(ptIndex, point[0],point[1],point[2]);
          if (ptIndex > firstPointIndex)
          {
            int cell[2] = {ptIndex-1,ptIndex};
            lines->InsertNextCell((vtkIdType)2,(vtkIdType*) cell);
          }
          lastPointIndex=ptIndex;
          ptIndex++;
        }
        pointsIt++;
        idx+=1;
      }

      if (nextContour->GetClosed())
      {
        int cell[2] = {lastPointIndex,firstPointIndex};
        lines->InsertNextCell((vtkIdType)2,(vtkIdType*) cell);
      }
      contourIt++;
    }

    m_ContourSet->SetPoints(points);
    m_ContourSet->SetLines(lines);
    m_ContourSet->Update();

    m_TubeFilter->SetInput(m_ContourSet);
    m_TubeFilter->SetRadius(1);
    m_TubeFilter->Update();
    m_VtkPolyDataMapper->SetInput(m_TubeFilter->GetOutput());

    vtkFloatingPointType rgba[4]={0.0f,1.0f,0.0f,0.6f};

    m_Actor->GetProperty()->SetColor(rgba);
    m_Actor->SetMapper(m_VtkPolyDataMapper);
  }
  
  SetVtkMapperImmediateModeRendering(m_VtkPolyDataMapper);
}

const mitk::ContourSet* mitk::ContourSetVtkMapper3D::GetInput()
{
  return static_cast<const mitk::ContourSet* > ( GetData() );
}
