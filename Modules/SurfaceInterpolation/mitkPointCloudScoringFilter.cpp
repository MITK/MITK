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
#include <vtkPolyData.h>

mitk::PointCloudScoringFilter::PointCloudScoringFilter():
  m_NumberOfOutpPoints(0)
{
  m_OutpSurface = mitk::Surface::New();
  this->SetNumberOfRequiredInputs(2);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, m_OutpSurface);
}

mitk::PointCloudScoringFilter::~PointCloudScoringFilter(){}

void mitk::PointCloudScoringFilter::GenerateData()
{
  if(SurfaceToSurfaceFilter::GetNumberOfInputs()!=2)
  {
    MITK_ERROR << "Not enough input arguments for PointCloudScoringFilter" << std::endl;
    return;
  }

  DataObjectPointerArray inputs = SurfaceToSurfaceFilter::GetInputs();
  mitk::Surface::Pointer edgeSurface = dynamic_cast<mitk::Surface*>(inputs.at(0).GetPointer());
  mitk::Surface::Pointer segmSurface = dynamic_cast<mitk::Surface*>(inputs.at(1).GetPointer());

  if(edgeSurface->IsEmpty() || segmSurface->IsEmpty())
  {
    MITK_ERROR << "Cannot convert input into Surfaces" << std::endl;
    return;
  }

  vtkSmartPointer<vtkPolyData> edgePolyData = edgeSurface->GetVtkPolyData();
  vtkSmartPointer<vtkPolyData> segmPolyData = segmSurface->GetVtkPolyData();

  // KdTree from here
  vtkSmartPointer<vtkPoints> kdPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkKdTree> kdTree = vtkSmartPointer<vtkKdTree>::New();

  for(int i=0; i<edgePolyData->GetNumberOfPoints(); i++)
  {
    kdPoints->InsertNextPoint(edgePolyData->GetPoint(i));
  }

  kdTree->BuildLocatorFromPoints(kdPoints);
  // KdTree until here

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  for(int i=0; i<segmPolyData->GetNumberOfPoints(); i++)
  {
    points->InsertNextPoint(segmPolyData->GetPoint(i));
  }

  std::vector< ScorePair > score;

  double dist_glob;
  double dist;

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

  vtkSmartPointer<vtkPolyData> outpSurface = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> filteredPoints = vtkSmartPointer<vtkPoints>::New();

  for(unsigned int i=0; i<m_FilteredScores.size(); i++)
  {
    mitk::Point3D point;
    point = segmPolyData->GetPoint(m_FilteredScores.at(i).first);
    filteredPoints->InsertNextPoint(point[0],point[1],point[2]);
  }

  outpSurface->SetPoints(filteredPoints);
  m_OutpSurface->SetVtkPolyData(outpSurface);
}

void mitk::PointCloudScoringFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
