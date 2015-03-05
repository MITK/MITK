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

#include <mitkUnstructuredGridClusteringFilter.h>

#include <vtkSmartPointer.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDelaunay3D.h>
#include <vtkPolyVertex.h>


mitk::UnstructuredGridClusteringFilter::UnstructuredGridClusteringFilter() : m_eps(0.0), m_MinPts(0), m_Meshing(true)
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::UnstructuredGridClusteringFilter::~UnstructuredGridClusteringFilter(){}

std::map<int, bool> visited;
std::map<int, bool> isNoise;
std::map<int, bool> clusterMember;
vtkSmartPointer<vtkPointLocator> pLocator;

void mitk::UnstructuredGridClusteringFilter::GenerateOutputInformation()
{
  m_UnstructGrid = this->GetOutput();
}

void mitk::UnstructuredGridClusteringFilter::GenerateData()
{
  mitk::UnstructuredGrid::Pointer inputGrid = const_cast<mitk::UnstructuredGrid*>(this->GetInput());
  if(inputGrid.IsNull()) return;

  vtkSmartPointer<vtkUnstructuredGrid> vtkInpGrid = inputGrid->GetVtkUnstructuredGrid();
  vtkSmartPointer<vtkPoints> inpPoints = vtkInpGrid->GetPoints();
  pLocator =vtkSmartPointer<vtkPointLocator>::New();
  std::vector< vtkSmartPointer<vtkPoints> > clusterVector;

  pLocator->SetDataSet(vtkInpGrid);
  pLocator->AutomaticOn();
  pLocator->SetNumberOfPointsPerBucket(2);
  pLocator->BuildLocator();

  //fill the visited map with false for checking
  for(int i=0; i<inpPoints->GetNumberOfPoints();i++)
  {
    visited[i] = false;
    isNoise[i] = false;
    clusterMember[i] = false;
  }

  for(int i=0; i<inpPoints->GetNumberOfPoints();i++)
  {
    if(!visited[i])
    {
      visited[i] = true; //mark P as visited
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New(); //represent N
      pLocator->FindPointsWithinRadius(m_eps, inpPoints->GetPoint(i), idList); //N = D.regionQuery(P, eps)
      if(idList->GetNumberOfIds() < m_MinPts) //if sizeof(N) < MinPts
      {
        isNoise[i] = true; //mark P as NOISE
      }
      else
      {
        vtkSmartPointer<vtkPoints> cluster = vtkSmartPointer<vtkPoints>::New(); //represent a cluster
        clusterVector.push_back(cluster); //C = next cluster
        this->ExpandCluster(i,idList,cluster,inpPoints); //expandCluster(P, N, C, eps, MinPts) mod. the parameter list
      }
    }
  }

  //OUTPUT LOGIC
  m_Clusters = clusterVector;
  int numberOfClusterPoints = 0;
  int IdOfBiggestCluster = 0;

  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
    vtkSmartPointer<vtkPoints> points = m_Clusters.at(i);
    for(int j=0; j<points->GetNumberOfPoints();j++)
    {
      double point[3];
      points->GetPoint(j,point);
    }
    if(points->GetNumberOfPoints() > numberOfClusterPoints)
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
  for(int i=0; i<m_Clusters.at(IdOfBiggestCluster)->GetNumberOfPoints(); i++)
  {
    verts->GetPointIds()->SetId(i,i);
  }

  biggestCluster->Allocate(1);
  biggestCluster->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  biggestCluster->SetPoints(m_Clusters.at(IdOfBiggestCluster));

  if(m_Meshing)
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
}

void mitk::UnstructuredGridClusteringFilter::ExpandCluster(int id, vtkIdList *pointIDs, vtkPoints* cluster, vtkPoints* inpPoints)
{
  cluster->InsertNextPoint(inpPoints->GetPoint(id)); //add P to cluster C
  clusterMember[id] = true; //right?

  vtkSmartPointer<vtkPoints> neighbours = vtkSmartPointer<vtkPoints>::New(); //same N as in other function
  inpPoints->GetPoints(pointIDs,neighbours);

  for(int i=0; i<pointIDs->GetNumberOfIds();i++) //for each point P' in N
  {
    if(!visited[pointIDs->GetId(i)]) //if P' is not visited
    {
      visited[pointIDs->GetId(i)] = true; //mark P' as visited
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New(); //represent N'
      pLocator->FindPointsWithinRadius(m_eps, inpPoints->GetPoint(pointIDs->GetId(i)), idList); //N' = D.regionQuery(P', eps)
      if(idList->GetNumberOfIds() >= m_MinPts) //if sizeof(N') >= MinPts
      {
        for(int j=0; j<idList->GetNumberOfIds();j++) //N = N joined with N'
        {
          pointIDs->InsertNextId(idList->GetId(j));
        }
      }
    }
    if(!clusterMember[pointIDs->GetId(i)]) //if P' is not yet member of any cluster
    {
      clusterMember[pointIDs->GetId(i)] = true;
      cluster->InsertNextPoint(inpPoints->GetPoint(pointIDs->GetId(i))); //add P' to cluster C
    }
  }
}

std::vector<mitk::UnstructuredGrid::Pointer> mitk::UnstructuredGridClusteringFilter::GetAllClusters()
{
  std::vector< mitk::UnstructuredGrid::Pointer > mitkUGridVector;

  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> cluster = vtkSmartPointer<vtkUnstructuredGrid>::New();

    vtkSmartPointer<vtkPoints> points = m_Clusters.at(i);

    vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

    verts->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for(int i=0; i<points->GetNumberOfPoints(); i++)
    {
      verts->GetPointIds()->SetId(i,i);
    }

    cluster->Allocate(1);
    cluster->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
    cluster->SetPoints(points);

    mitk::UnstructuredGrid::Pointer mitkGrid = mitk::UnstructuredGrid::New();

    if(m_Meshing)
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
