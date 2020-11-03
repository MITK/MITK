/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUnstructuredGridClusteringFilter.h>

#include <vector>

#include <vtkDataArray.h>
#include <vtkDelaunay3D.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyVertex.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVariant.h>

mitk::UnstructuredGridClusteringFilter::UnstructuredGridClusteringFilter()
  : m_eps(5.0), m_MinPts(4), m_Meshing(false), m_DistCalc(false)
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::UnstructuredGridClusteringFilter::~UnstructuredGridClusteringFilter()
{
}

std::map<int, bool> visited;
std::map<int, bool> isNoise;
std::map<int, bool> clusterMember;
vtkSmartPointer<vtkPointLocator> pLocator;
std::vector<vtkSmartPointer<vtkPoints>> clusterVector;

std::vector<std::vector<int>> clustersPointsIDs;

void mitk::UnstructuredGridClusteringFilter::GenerateOutputInformation()
{
  m_UnstructGrid = this->GetOutput();
}

void mitk::UnstructuredGridClusteringFilter::GenerateData()
{
  mitk::UnstructuredGrid::Pointer inputGrid = const_cast<mitk::UnstructuredGrid *>(this->GetInput());
  if (inputGrid.IsNull())
    return;

  vtkSmartPointer<vtkUnstructuredGrid> vtkInpGrid = inputGrid->GetVtkUnstructuredGrid();
  vtkSmartPointer<vtkPoints> inpPoints = vtkInpGrid->GetPoints();
  pLocator = vtkSmartPointer<vtkPointLocator>::New();

  vtkSmartPointer<vtkDoubleArray> distances = vtkSmartPointer<vtkDoubleArray>::New();
  if (inputGrid->GetVtkUnstructuredGrid()->GetPointData()->GetNumberOfArrays() > 0)
  {
    m_DistCalc = true;
    distances = dynamic_cast<vtkDoubleArray *>(vtkInpGrid->GetPointData()->GetArray(0));
  }

  pLocator->SetDataSet(vtkInpGrid);
  pLocator->AutomaticOn();
  pLocator->SetNumberOfPointsPerBucket(2);
  pLocator->BuildLocator();

  // fill the visited map with false for checking
  for (int i = 0; i < inpPoints->GetNumberOfPoints(); i++)
  {
    visited[i] = false;
    isNoise[i] = false;
    clusterMember[i] = false;
  }

  for (int i = 0; i < inpPoints->GetNumberOfPoints(); i++)
  {
    if (!visited[i])
    {
      visited[i] = true;                                                       // mark P as visited
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();   // represent N
      pLocator->FindPointsWithinRadius(m_eps, inpPoints->GetPoint(i), idList); // N = D.regionQuery(P, eps)
      if (idList->GetNumberOfIds() < m_MinPts)                                 // if sizeof(N) < MinPts
      {
        isNoise[i] = true; // mark P as NOISE
      }
      else
      {
        vtkSmartPointer<vtkPoints> cluster = vtkSmartPointer<vtkPoints>::New(); // represent a cluster
        clusterVector.push_back(cluster);                                       // C = next cluster
        this->ExpandCluster(
          i, idList, cluster, inpPoints); // expandCluster(P, N, C, eps, MinPts) mod. the parameter list
      }
    }
  }

  // OUTPUT LOGIC
  m_Clusters = clusterVector;
  int numberOfClusterPoints = 0;
  int IdOfBiggestCluster = 0;

  for (unsigned int i = 0; i < m_Clusters.size(); i++)
  {
    vtkSmartPointer<vtkDoubleArray> array = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkPoints> points = m_Clusters.at(i);
    if (m_DistCalc)
    {
      array->SetNumberOfComponents(1);
      array->SetNumberOfTuples(points->GetNumberOfPoints());
      for (int j = 0; j < points->GetNumberOfPoints(); j++)
      {
        double point[3];
        points->GetPoint(j, point);
        if (clustersPointsIDs.at(i).at(j) < inpPoints->GetNumberOfPoints())
        {
          if (distances->GetValue(clustersPointsIDs.at(i).at(j)) > 0.001)
          {
            double dist[1] = {distances->GetValue(clustersPointsIDs.at(i).at(j))};
            array->SetTuple(j, dist);
          }
          else
          {
            double dist[1] = {0.0};
            array->SetTuple(j, dist);
          }
        }
      }
      m_DistanceArrays.push_back(array);
    }
    if (points->GetNumberOfPoints() > numberOfClusterPoints)
    {
      numberOfClusterPoints = points->GetNumberOfPoints();
      IdOfBiggestCluster = i;
    }
  }

  vtkSmartPointer<vtkUnstructuredGrid> biggestCluster = vtkSmartPointer<vtkUnstructuredGrid>::New();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points = m_Clusters.at(IdOfBiggestCluster);

  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();
  verts->GetPointIds()->SetNumberOfIds(m_Clusters.at(IdOfBiggestCluster)->GetNumberOfPoints());
  for (int i = 0; i < m_Clusters.at(IdOfBiggestCluster)->GetNumberOfPoints(); i++)
  {
    verts->GetPointIds()->SetId(i, i);
  }

  biggestCluster->Allocate(1);
  biggestCluster->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  biggestCluster->SetPoints(m_Clusters.at(IdOfBiggestCluster));

  if (m_Meshing)
  {
    vtkSmartPointer<vtkDelaunay3D> mesher = vtkSmartPointer<vtkDelaunay3D>::New();
    mesher->SetInputData(biggestCluster);
    mesher->SetAlpha(0.9);
    mesher->Update();

    vtkSmartPointer<vtkUnstructuredGrid> output = mesher->GetOutput();
    m_UnstructGrid->SetVtkUnstructuredGrid(output);
  }
  else
  {
    m_UnstructGrid->SetVtkUnstructuredGrid(biggestCluster);
  }

  clusterVector.clear();
  clustersPointsIDs.clear();
}

void mitk::UnstructuredGridClusteringFilter::ExpandCluster(int id,
                                                           vtkIdList *pointIDs,
                                                           vtkPoints *cluster,
                                                           vtkPoints *inpPoints)
{
  std::vector<int> x;
  x.push_back(id);
  cluster->InsertNextPoint(inpPoints->GetPoint(id)); // add P to cluster C
  clusterMember[id] = true;

  vtkSmartPointer<vtkPoints> neighbours = vtkSmartPointer<vtkPoints>::New(); // same N as in other function
  inpPoints->GetPoints(pointIDs, neighbours);

  for (int i = 0; i < pointIDs->GetNumberOfIds(); i++) // for each point P' in N
  {
    if (!visited[pointIDs->GetId(i)]) // if P' is not visited
    {
      visited[pointIDs->GetId(i)] = true;                                    // mark P' as visited
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New(); // represent N'
      pLocator->FindPointsWithinRadius(
        m_eps, inpPoints->GetPoint(pointIDs->GetId(i)), idList); // N' = D.regionQuery(P', eps)

      if (idList->GetNumberOfIds() >= m_MinPts) // if sizeof(N') >= MinPts
      {
        for (int j = 0; j < idList->GetNumberOfIds(); j++) // N = N joined with N'
        {
          if (idList->GetId(j) < inpPoints->GetNumberOfPoints()) // a litte bit hacked ?!
          {
            pointIDs->InsertNextId(idList->GetId(j));
          }
        }
      }
    }
    if (!clusterMember[pointIDs->GetId(i)]) // if P' is not yet member of any cluster
    {
      if (pointIDs->GetId(i) < inpPoints->GetNumberOfPoints())
      {
        clusterMember[pointIDs->GetId(i)] = true;
        x.push_back(pointIDs->GetId(i));
        cluster->InsertNextPoint(inpPoints->GetPoint(pointIDs->GetId(i))); // add P' to cluster C
      }
    }
  }

  clustersPointsIDs.push_back(x);
}

std::vector<mitk::UnstructuredGrid::Pointer> mitk::UnstructuredGridClusteringFilter::GetAllClusters()
{
  std::vector<mitk::UnstructuredGrid::Pointer> mitkUGridVector;

  for (unsigned int i = 0; i < m_Clusters.size(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> cluster = vtkSmartPointer<vtkUnstructuredGrid>::New();

    vtkSmartPointer<vtkPoints> points = m_Clusters.at(i);

    vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

    verts->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for (int j = 0; j < points->GetNumberOfPoints(); j++)
    {
      verts->GetPointIds()->SetId(j, j);
    }

    cluster->Allocate(1);
    cluster->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
    cluster->SetPoints(points);
    if (m_DistCalc)
    {
      cluster->GetPointData()->AddArray(m_DistanceArrays.at(i));
    }

    mitk::UnstructuredGrid::Pointer mitkGrid = mitk::UnstructuredGrid::New();

    if (m_Meshing)
    {
      vtkSmartPointer<vtkDelaunay3D> mesher = vtkSmartPointer<vtkDelaunay3D>::New();
      mesher->SetInputData(cluster);
      mesher->SetAlpha(0.9);
      mesher->Update();

      vtkSmartPointer<vtkUnstructuredGrid> output = mesher->GetOutput();
      mitkGrid->SetVtkUnstructuredGrid(output);
    }
    else
    {
      mitkGrid->SetVtkUnstructuredGrid(cluster);
    }

    mitkUGridVector.push_back(mitkGrid);
  }

  return mitkUGridVector;
}

int mitk::UnstructuredGridClusteringFilter::GetNumberOfFoundClusters()
{
  return m_Clusters.size();
}
