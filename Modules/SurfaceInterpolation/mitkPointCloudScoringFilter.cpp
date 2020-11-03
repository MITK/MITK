/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointCloudScoringFilter.h"

#include <cmath>

#include <vtkDoubleArray.h>
#include <vtkKdTree.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyVertex.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

mitk::PointCloudScoringFilter::PointCloudScoringFilter() : m_NumberOfOutpPoints(0)
{
  this->SetNumberOfIndexedOutputs(1);
}

mitk::PointCloudScoringFilter::~PointCloudScoringFilter()
{
}

void mitk::PointCloudScoringFilter::GenerateData()
{
  if (UnstructuredGridToUnstructuredGridFilter::GetNumberOfInputs() != 2)
  {
    MITK_ERROR << "Not enough input arguments for PointCloudScoringFilter" << std::endl;
    return;
  }

  DataObjectPointerArray inputs = UnstructuredGridToUnstructuredGridFilter::GetInputs();
  mitk::UnstructuredGrid::Pointer edgeGrid = dynamic_cast<mitk::UnstructuredGrid *>(inputs.at(0).GetPointer());
  mitk::UnstructuredGrid::Pointer segmGrid = dynamic_cast<mitk::UnstructuredGrid *>(inputs.at(1).GetPointer());

  if (edgeGrid->IsEmpty() || segmGrid->IsEmpty())
  {
    if (edgeGrid->IsEmpty())
      MITK_ERROR << "edgeGrid is empty" << std::endl;
    if (segmGrid->IsEmpty())
      MITK_ERROR << "segmGrid is empty" << std::endl;
  }

  if (m_FilteredScores.size() > 0)
    m_FilteredScores.clear();

  vtkSmartPointer<vtkUnstructuredGrid> edgevtkGrid = edgeGrid->GetVtkUnstructuredGrid();
  vtkSmartPointer<vtkUnstructuredGrid> segmvtkGrid = segmGrid->GetVtkUnstructuredGrid();

  // KdTree from here
  vtkSmartPointer<vtkPoints> kdPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkKdTree> kdTree = vtkSmartPointer<vtkKdTree>::New();

  for (int i = 0; i < edgevtkGrid->GetNumberOfPoints(); i++)
  {
    kdPoints->InsertNextPoint(edgevtkGrid->GetPoint(i));
  }

  kdTree->BuildLocatorFromPoints(kdPoints);
  // KdTree until here

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  for (int i = 0; i < segmvtkGrid->GetNumberOfPoints(); i++)
  {
    points->InsertNextPoint(segmvtkGrid->GetPoint(i));
  }

  std::vector<ScorePair> score;
  std::vector<double> distances;

  double dist_glob = 0.0;
  double dist = 0.0;

  for (int i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double point[3];
    points->GetPoint(i, point);
    kdTree->FindClosestPoint(point[0], point[1], point[2], dist);
    dist_glob += dist;
    distances.push_back(dist);
    score.push_back(std::make_pair(i, dist));
  }

  double avg = dist_glob / points->GetNumberOfPoints();

  double tmpVar = 0.0;
  double highest = 0.0;

  for (unsigned int i = 0; i < distances.size(); i++)
  {
    tmpVar = tmpVar + ((distances.at(i) - avg) * (distances.at(i) - avg));
    if (distances.at(i) > highest)
      highest = distances.at(i);
  }

  // CUBIC MEAN
  double cubicAll = 0.0;
  for (unsigned i = 0; i < score.size(); i++)
  {
    cubicAll = cubicAll + score.at(i).second * score.at(i).second * score.at(i).second;
  }
  double root2 = cubicAll / static_cast<double>(score.size());
  double cubic = pow(root2, 1.0 / 3.0);
  // CUBIC END

  double metricValue = cubic;

  for (unsigned int i = 0; i < score.size(); i++)
  {
    if (score.at(i).second > metricValue)
    {
      m_FilteredScores.push_back(std::make_pair(score.at(i).first, score.at(i).second));
    }
  }

  m_NumberOfOutpPoints = m_FilteredScores.size();

  vtkSmartPointer<vtkPoints> filteredPoints = vtkSmartPointer<vtkPoints>::New();

  // storing the distances in the uGrid PointData
  vtkSmartPointer<vtkDoubleArray> pointDataDistances = vtkSmartPointer<vtkDoubleArray>::New();
  pointDataDistances->SetNumberOfComponents(1);
  pointDataDistances->SetNumberOfTuples(m_FilteredScores.size());
  pointDataDistances->SetName("Distances");

  for (unsigned int i = 0; i < m_FilteredScores.size(); i++)
  {
    mitk::Point3D point;
    point = segmvtkGrid->GetPoint(m_FilteredScores.at(i).first);
    filteredPoints->InsertNextPoint(point[0], point[1], point[2]);
    if (score.at(i).second > 0.001)
    {
      double dist[1] = {score.at(i).second};
      pointDataDistances->InsertTuple(i, dist);
    }
    else
    {
      double dist[1] = {0.0};
      pointDataDistances->InsertTuple(i, dist);
    }
  }

  unsigned int numPoints = filteredPoints->GetNumberOfPoints();

  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(numPoints);
  for (unsigned int i = 0; i < numPoints; i++)
  {
    verts->GetPointIds()->SetId(i, i);
  }

  vtkSmartPointer<vtkUnstructuredGrid> uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGrid->Allocate(1);

  uGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  uGrid->SetPoints(filteredPoints);
  uGrid->GetPointData()->AddArray(pointDataDistances);

  mitk::UnstructuredGrid::Pointer outputGrid = mitk::UnstructuredGrid::New();
  outputGrid->SetVtkUnstructuredGrid(uGrid);
  this->SetNthOutput(0, outputGrid);

  score.clear();
  distances.clear();
}

void mitk::PointCloudScoringFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
