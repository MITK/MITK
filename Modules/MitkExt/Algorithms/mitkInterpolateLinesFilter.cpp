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

#include "mitkInterpolateLinesFilter.h"
#include "mitkMesh.h"
#include "mitkSurface.h"

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCardinalSpline.h>

#include <vector>


mitk::InterpolateLinesFilter::InterpolateLinesFilter()
: m_SplineResolution(10), m_GeometryForInterpolation(NULL), m_Length(0.0)
{
  m_SpX=vtkCardinalSpline::New();
  m_SpY=vtkCardinalSpline::New();
  m_SpZ=vtkCardinalSpline::New();

}

mitk::InterpolateLinesFilter::~InterpolateLinesFilter()
{
  m_SpX->Delete();
  m_SpY->Delete();
  m_SpZ->Delete();
}

void mitk::InterpolateLinesFilter::GenerateOutputInformation()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::Surface::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

  if(m_GeometryForInterpolation.IsNotNull())
    output->SetGeometry(static_cast<Geometry3D*>(m_GeometryForInterpolation->Clone().GetPointer()));
  else
    output->SetGeometry(static_cast<Geometry3D*>(input->GetGeometry()->Clone().GetPointer()));
}

void mitk::InterpolateLinesFilter::GenerateData()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::Surface::Pointer output = this->GetOutput();

  vtkPolyData *polyData = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *cellarray = vtkCellArray::New();

  mitk::Mesh::PointType thisPoint;

  m_Length = 0.0;

  //iterate through all cells and build tubes
  Mesh::ConstCellIterator cellIt, cellEnd;
  cellEnd = input->GetMesh()->GetCells()->End();
  for( cellIt = input->GetMesh()->GetCells()->Begin(); cellIt != cellEnd; ++cellIt )
  {
    if(((*cellIt->Value()).GetType()==mitk::Mesh::CellType::POLYGON_CELL)
      && ((*cellIt->Value()).GetNumberOfPoints()>=2))
        BuildPointAndVectorList(*cellIt->Value(), points, cellarray);
  }

  polyData->SetPoints( points );
  points->Delete();
  polyData->SetLines( cellarray );
  cellarray->Delete();

  output->SetVtkPolyData(polyData);

  polyData->Delete();
}

void mitk::InterpolateLinesFilter::BuildPointAndVectorList(mitk::Mesh::CellType& cell, vtkPoints* points, vtkCellArray* cellarray)
{
  const mitk::Mesh* input = GetInput();

  Mesh::PointIdIterator ptIt;
  Mesh::PointIdIterator ptEnd;

  ptEnd = cell.PointIdsEnd();

  Point3D pt;

  int i, size=cell.GetNumberOfPoints();

  int closed_loop_pre_load=0;//m_SplineResolution;
  if(m_GeometryForInterpolation.IsNull())
  {
    //bei geschlossener Kontur: vor dem ersten Punkt die zwei letzten einfügen für glatten Übergang
    ptIt = ptEnd; ptIt-=closed_loop_pre_load+1;
    for(i=0;i<closed_loop_pre_load;++i, ++ptIt)
    {
      pt = input->GetPoint(*ptIt);
      m_SpX->AddPoint(i, pt[0]); m_SpY->AddPoint(i, pt[1]); m_SpZ->AddPoint(i, pt[2]);
    }
    //Punkte einfügen
    for(ptIt = cell.PointIdsBegin();i<size+closed_loop_pre_load;++i, ++ptIt)
    {
      pt = input->GetPoint(*ptIt);
      m_SpX->AddPoint(i, pt[0]); m_SpY->AddPoint(i, pt[1]); m_SpZ->AddPoint(i, pt[2]);
    }
    //bei geschlossener Kontur: nach dem letzten Punkt die zwei ersten einfügen für glatten Übergang
    int j;
    for(j=0,ptIt = cell.PointIdsBegin();j<closed_loop_pre_load;++j,++i, ++ptIt)
    {
      pt = input->GetPoint(*ptIt);
      m_SpX->AddPoint(i, pt[0]); m_SpY->AddPoint(i, pt[1]); m_SpZ->AddPoint(i, pt[2]);
    }
    //bool first = true;
    Point3D lastPt, firstPt;
    Vector3D vec;
    float t, step=1.0f/m_SplineResolution;
    size=(size-1)*m_SplineResolution;
    i=closed_loop_pre_load;

    cellarray->InsertNextCell(size);
    for(t=closed_loop_pre_load;i<size+closed_loop_pre_load;++i, t+=step)
    {
      float pt[3];
      FillVector3D(pt, m_SpX->Evaluate(t), m_SpY->Evaluate(t), m_SpZ->Evaluate(t));
      cellarray->InsertCellPoint(points->InsertNextPoint(pt));
    }
  }
  else //m_GeometryForInterpolation!=NULL
  {
    Point2D pt2d;
    //bei geschlossener Kontur: vor dem ersten Punkt die zwei letzten einfügen für glatten Übergang
    ptIt = ptEnd; ptIt-=closed_loop_pre_load+1;
    for(i=0;i<closed_loop_pre_load;++i, ++ptIt)
    {
      m_GeometryForInterpolation->Map(input->GetPoint(*ptIt), pt2d);
      m_SpX->AddPoint(i, pt2d[0]); m_SpY->AddPoint(i, pt2d[1]);
    }
    //Punkte einfügen
    for(ptIt = cell.PointIdsBegin();i<size+closed_loop_pre_load;++i, ++ptIt)
    {
      m_GeometryForInterpolation->Map(input->GetPoint(*ptIt), pt2d);
      m_SpX->AddPoint(i, pt2d[0]); m_SpY->AddPoint(i, pt2d[1]);
    }
    //bei geschlossener Kontur: nach dem letzten Punkt die zwei ersten einfügen für glatten Übergang
    int j;
    for(j=0,ptIt = cell.PointIdsBegin();j<closed_loop_pre_load;++j,++i, ++ptIt)
    {
      m_GeometryForInterpolation->Map(input->GetPoint(*ptIt), pt2d);
      m_SpX->AddPoint(i, pt2d[0]); m_SpY->AddPoint(i, pt2d[1]);
    }
    bool first = true;
    Point3D lastPt; lastPt.Fill(0);
    Vector3D vec;
    float t, step=1.0f/m_SplineResolution;
    size=(size-1)*m_SplineResolution;
    i=closed_loop_pre_load;

    cellarray->InsertNextCell(size);
    for(t=closed_loop_pre_load;i<size+closed_loop_pre_load;++i, t+=step)
    {
      pt2d[0] = m_SpX->Evaluate(t); pt2d[1] = m_SpY->Evaluate(t);
      m_GeometryForInterpolation->Map(pt2d, pt);

      if(first==false)
      {
        vec=pt-lastPt;
        m_Length+=vec.GetNorm();
      }
      first=false;

      float pvtk[3];
      itk2vtk(pt, pvtk);
      cellarray->InsertCellPoint(points->InsertNextPoint(pvtk));
      lastPt = pt;
    }
  }
}

const mitk::Mesh *mitk::InterpolateLinesFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Mesh * >
    (this->ProcessObject::GetInput(0) );
}

void mitk::InterpolateLinesFilter::SetInput(const mitk::Mesh *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::Mesh * >( input ) );
}
