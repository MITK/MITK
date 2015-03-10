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

#include "mitkPointCloudScoringFilter.h"

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkKdTree.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyVertex.h>

mitk::PointCloudScoringFilter::PointCloudScoringFilter():
  m_NumberOfOutpPoints(0)
{
  m_OutpGrid = mitk::UnstructuredGrid::New();

  this->SetNthOutput(0, m_OutpGrid);
}

mitk::PointCloudScoringFilter::~PointCloudScoringFilter(){}

void mitk::PointCloudScoringFilter::GenerateData()
{
  if(UnstructuredGridToUnstructuredGridFilter::GetNumberOfInputs()!=2)
  {
    MITK_ERROR << "Not enough input arguments for PointCloudScoringFilter" << std::endl;
    return;
  }

  DataObjectPointerArray inputs = UnstructuredGridToUnstructuredGridFilter::GetInputs();
  mitk::UnstructuredGrid::Pointer edgeGrid = dynamic_cast<mitk::UnstructuredGrid*>(inputs.at(0).GetPointer());
  mitk::UnstructuredGrid::Pointer segmGrid = dynamic_cast<mitk::UnstructuredGrid*>(inputs.at(1).GetPointer());

  if(edgeGrid->IsEmpty() || segmGrid->IsEmpty())
  {
    if(edgeGrid->IsEmpty())
      MITK_ERROR << "Cannot convert edgeGrid into Surfaces" << std::endl;
    if(segmGrid->IsEmpty())
      MITK_ERROR << "Cannot convert segmGrid into Surfaces" << std::endl;
  }

  vtkSmartPointer<vtkUnstructuredGrid> edgevtkGrid = edgeGrid->GetVtkUnstructuredGrid();
  vtkSmartPointer<vtkUnstructuredGrid> segmvtkGrid = segmGrid->GetVtkUnstructuredGrid();

  // KdTree from here
  vtkSmartPointer<vtkPoints> kdPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkKdTree> kdTree = vtkSmartPointer<vtkKdTree>::New();

  for(int i=0; i<edgevtkGrid->GetNumberOfPoints(); i++)
  {
    kdPoints->InsertNextPoint(edgevtkGrid->GetPoint(i));
  }

  kdTree->BuildLocatorFromPoints(kdPoints);
  // KdTree until here

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  for(int i=0; i<segmvtkGrid->GetNumberOfPoints(); i++)
  {
    points->InsertNextPoint(segmvtkGrid->GetPoint(i));
  }

  std::vector< ScorePair > score;

  double dist_glob = 0.0;
  double dist = 0.0;

  for(int i=0; i<points->GetNumberOfPoints(); i++)
  {
    double point[3];
    points->GetPoint(i,point);
    kdTree->FindClosestPoint(point[0],point[1],point[2],dist);
    dist_glob+=dist;
    score.push_back(std::make_pair(i,dist));
  }

  double avg = dist_glob / points->GetNumberOfPoints();

  for(unsigned int i=0; i<score.size();++i)
  {
    if(score.at(i).second > avg)
    {
      m_FilteredScores.push_back(std::make_pair(score.at(i).first,score.at(i).second));
    }
  }

  m_NumberOfOutpPoints = m_FilteredScores.size();

  vtkSmartPointer<vtkPoints> filteredPoints = vtkSmartPointer<vtkPoints>::New();

  for(unsigned int i=0; i<m_FilteredScores.size(); i++)
  {
    mitk::Point3D point;
    point = segmvtkGrid->GetPoint(m_FilteredScores.at(i).first);
    filteredPoints->InsertNextPoint(point[0],point[1],point[2]);
  }

  unsigned int numPoints = filteredPoints->GetNumberOfPoints();

  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(numPoints);
  for(unsigned int i=0; i<numPoints; i++)
  {
    verts->GetPointIds()->SetId(i,i);
  }

  vtkSmartPointer<vtkUnstructuredGrid> uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGrid->Allocate(1);

  uGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  uGrid->SetPoints(filteredPoints);

  m_OutpGrid->SetVtkUnstructuredGrid(uGrid);
}

void mitk::PointCloudScoringFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
