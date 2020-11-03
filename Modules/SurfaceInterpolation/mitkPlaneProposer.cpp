/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlaneProposer.h"

#include <mitkGeometryData.h>
#include <mitkPlaneFit.h>
#include <mitkPointSet.h>
#include <mitkSliceNavigationController.h>
#include <mitkUnstructuredGrid.h>
#include <mitkVector.h>

#include <vtkCenterOfMass.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

bool compare(std::pair<double, int> i, std::pair<double, int> j)
{
  return (i.first > j.first);
}

mitk::PlaneProposer::PlaneProposer() : m_UseDistances(false), m_UseLeastSquares(false), m_NumberOfClustersToUse(3)
{
}

mitk::PlaneProposer::~PlaneProposer()
{
}

void mitk::PlaneProposer::SetUnstructuredGrids(std::vector<mitk::UnstructuredGrid::Pointer> &grids)
{
  if (grids.empty())
  {
    MITK_WARN << "Input grids are empty!";
    return;
  }
  m_Grids = grids;
}

void mitk::PlaneProposer::SetUseDistances(bool status)
{
  m_UseDistances = status;
}

void mitk::PlaneProposer::SetNumberOfClustersToUse(unsigned int num)
{
  m_NumberOfClustersToUse = num;
}

void mitk::PlaneProposer::SetSliceNavigationController(mitk::SliceNavigationController::Pointer &snc)
{
  m_SNC = snc;
}

std::array<std::array<double, 3>, 3> mitk::PlaneProposer::GetCentroids()
{
  return m_Centroids;
}

mitk::PlaneProposer::PlaneInfo mitk::PlaneProposer::GetProposedPlaneInfo()
{
  return m_ProposedPlaneInfo;
}

void mitk::PlaneProposer::CreatePlaneInfo()
{
  // find the avg distances of every cluster maybe a square mean for better looking at high values?
  std::vector<std::pair<double, int>> avgDistances;

  // get the number of points and the id of every cluster
  std::vector<std::pair<int, int>> sizeIDs;

  for (unsigned int i = 0; i < m_Grids.size(); i++)
  {
    mitk::UnstructuredGrid::Pointer cluster = m_Grids.at(i);
    vtkSmartPointer<vtkDoubleArray> data =
      dynamic_cast<vtkDoubleArray *>(cluster->GetVtkUnstructuredGrid()->GetPointData()->GetArray(0));
    double avg = 0.0;
    for (int j = 0; j < data->GetSize(); j++)
    {
      avg += data->GetValue(j);
    }
    avgDistances.push_back(std::make_pair(avg / static_cast<double>(data->GetSize()), i));
  }

  // now sort the clusters with their distances
  for (unsigned int i = 0; i < m_Grids.size(); i++)
  {
    sizeIDs.push_back(std::make_pair(m_Grids.at(i)->GetVtkUnstructuredGrid()->GetNumberOfPoints(), i));
  }

  // sort the point IDs for finding the biggest clusters and clusters with the
  // highest distance
  // sizeIDs is sorted by number of points in each cluster
  // avgDistances is sorted by avg distance to next edge
  std::sort(sizeIDs.begin(), sizeIDs.end(), compare);
  std::sort(avgDistances.begin(), avgDistances.end(), compare);

  if (m_Grids.size() < m_NumberOfClustersToUse || m_UseLeastSquares)
  {
    // We need at least three points to define a plane
    m_NumberOfClustersToUse = m_Grids.size();
    m_ProposedPlaneInfo = this->CreatePlaneByLeastSquares(sizeIDs, avgDistances);
  }
  else
  {
    m_ProposedPlaneInfo = this->CreatePlaneByCentroids(sizeIDs, avgDistances);
  }

  if (m_SNC.IsNotNull())
  {
    m_SNC->ReorientSlices(m_SNC->GetCurrentPlaneGeometry()->GetCenter(), m_ProposedPlaneInfo.x, m_ProposedPlaneInfo.y);
    m_SNC->SelectSliceByPoint(m_ProposedPlaneInfo.pointOnPlane);
  }
}

mitk::PlaneProposer::PlaneInfo mitk::PlaneProposer::CreatePlaneByCentroids(
  const std::vector<std::pair<int, int>> &sizeIDs, const std::vector<std::pair<double, int>> &avgDistances)
{
  for (unsigned int j = 0; j < m_NumberOfClustersToUse; j++) // iterate over the clusters
  {
    // Get the cluster with "id" from all clusters, "id" is saved in sizeIDs.second
    // sizeIDs.first represent the number of points which is only needed for sorting the "id"s
    vtkSmartPointer<vtkUnstructuredGrid> tmpGrid;

    if (m_UseDistances)
      tmpGrid = m_Grids.at(avgDistances.at(j).second)->GetVtkUnstructuredGrid(); // highest distance
    else
      tmpGrid = m_Grids.at(sizeIDs.at(j).second)->GetVtkUnstructuredGrid(); // biggest cluster

    double tmpCenter[3];
    vtkCenterOfMass::ComputeCenterOfMass(tmpGrid->GetPoints(), nullptr, tmpCenter);
    std::array<double, 3> center;
    center[0] = tmpCenter[0];
    center[1] = tmpCenter[1];
    center[2] = tmpCenter[2];
    m_Centroids[j] = center;
  }

  double x[3];
  x[0] = m_Centroids[1][0] - m_Centroids[0][0];
  x[1] = m_Centroids[1][1] - m_Centroids[0][1];
  x[2] = m_Centroids[1][2] - m_Centroids[0][2];
  double y[3];
  y[0] = m_Centroids[2][0] - m_Centroids[0][0];
  y[1] = m_Centroids[2][1] - m_Centroids[0][1];
  y[2] = m_Centroids[2][2] - m_Centroids[0][2];
  double normal[3];
  vtkMath::Cross(x, y, normal); // TODO do it manually!
  mitk::PlaneProposer::PlaneInfo planeInfo;
  planeInfo.normal = normal;
  planeInfo.x = x;
  planeInfo.y = y;
  planeInfo.pointOnPlane[0] = m_Centroids[0][0];
  planeInfo.pointOnPlane[1] = m_Centroids[0][1];
  planeInfo.pointOnPlane[2] = m_Centroids[0][2];
  return planeInfo;
}

mitk::PlaneProposer::PlaneInfo mitk::PlaneProposer::CreatePlaneByLeastSquares(
  const std::vector<std::pair<int, int>> &sizeIDs, const std::vector<std::pair<double, int>> &avgDistances)
{
  // Generate a pointset from UnstructuredGrid for the PlaneFitFilter:
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  int pointId = 0;

  for (unsigned int j = 0; j < m_NumberOfClustersToUse; j++) // iterate over the clusters
  {
    // Get the cluster with "id" from all clusters, "id" is saved in sizeIDs.second
    // sizeIDs.first represent the number of points which is only needed for sorting the "id"s
    vtkSmartPointer<vtkUnstructuredGrid> tmpGrid;

    if (m_UseDistances)
      tmpGrid = m_Grids.at(avgDistances.at(j).second)->GetVtkUnstructuredGrid(); // highest distance
    else
      tmpGrid = m_Grids.at(sizeIDs.at(j).second)->GetVtkUnstructuredGrid(); // biggest cluster

    for (int i = 0; i < tmpGrid->GetNumberOfPoints(); i++)
    {
      mitk::Point3D point;
      point[0] = tmpGrid->GetPoint(i)[0];
      point[1] = tmpGrid->GetPoint(i)[1];
      point[2] = tmpGrid->GetPoint(i)[2];

      pointset->InsertPoint(pointId, point);
      pointId++;
    }
  }
  mitk::PlaneFit::Pointer planeFilter = mitk::PlaneFit::New();
  planeFilter->SetInput(pointset);
  planeFilter->Update();

  mitk::GeometryData::Pointer geoData = planeFilter->GetOutput();

  if (geoData.IsNull())
  {
    mitkThrow() << "GeometryData output from PlaneFit filter is null!";
  }
  mitk::PlaneProposer::PlaneInfo planeInfo;
  mitk::PlaneGeometry::Pointer plane = dynamic_cast<mitk::PlaneGeometry *>(geoData->GetGeometry());
  planeInfo.normal = plane->GetNormal();
  planeInfo.pointOnPlane = plane->GetCenter();
  return planeInfo;
}
