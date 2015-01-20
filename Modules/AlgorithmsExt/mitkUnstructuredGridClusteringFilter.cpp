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
#include <vtkUnstructuredGrid.h>


mitk::UnstructuredGridClusteringFilter::UnstructuredGridClusteringFilter() : m_eps(0.0), m_MinPts(0)
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
  mitk::UnstructuredGrid::Pointer inputGrid = const_cast<mitk::UnstructuredGrid*>(this->GetInput());
  if(inputGrid.IsNull()) return;

  vtkSmartPointer<vtkUnstructuredGrid> vtkInpGrid = inputGrid->GetVtkUnstructuredGrid();
  vtkSmartPointer<vtkPoints> inpPoints = vtkInpGrid->GetPoints();
  pLocator =vtkSmartPointer<vtkPointLocator>::New();
  std::vector<vtkPoints*> clusterVector;

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
    visited[i] = true; //mark P as visited
    vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
    pLocator->FindPointsWithinRadius(m_eps, inpPoints->GetPoint(i), idList); //find neighbours
    if(idList->GetNumberOfIds() < m_MinPts)
    {
      isNoise[i] = true;//point is noise - maybe just skip it?
    }
    else
    {
      vtkSmartPointer<vtkPoints> cluster = vtkSmartPointer<vtkPoints>::New();
      clusterVector.push_back(cluster);
      this->ExpandCluster(i,idList,cluster,inpPoints);
    }
  }

  m_UnstructGrid = this->GetOutput();
}

void mitk::UnstructuredGridClusteringFilter::ExpandCluster(int id, vtkIdList *pointIDs, vtkPoints* cluster, vtkPoints* inpPoints)
{
  cluster->InsertNextPoint(inpPoints->GetPoint(id)); //add Point to Cluster

  vtkSmartPointer<vtkPoints> neighbours = vtkSmartPointer<vtkPoints>::New();
  inpPoints->GetPoints(pointIDs,neighbours);

  for(int i=0; i<pointIDs->GetNumberOfIds();i++)
  {
    if(!visited[pointIDs->GetId(i)]) //if P is not visited
    {
      visited[pointIDs->GetId(i)] = true; // mark P as visited
      vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
      pLocator->FindPointsWithinRadius(m_eps, inpPoints->GetPoint(pointIDs->GetId(i)), idList); //find neighbours
      if(idList->GetNumberOfIds() >= m_MinPts)
      {
        for(int j=0; j<idList->GetNumberOfIds();j++)//join every point in range to the points
        {
          pointIDs->InsertNextId(idList->GetId(j));
        }
      }
    }
    if(!clusterMember[pointIDs->GetId(i)]) //P ist not yet member of any cluster
    {
      clusterMember[pointIDs->GetId(i)] = true;
      cluster->InsertNextPoint(inpPoints->GetPoint(pointIDs->GetId(i)));
    }
  }
}
