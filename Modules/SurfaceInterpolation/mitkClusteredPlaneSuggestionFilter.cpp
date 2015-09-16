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

#include <mitkClusteredPlaneSuggestionFilter.h>

#include <deque>
#include <fstream>
#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointLocator.h>
#include <vtkPolyVertex.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

#include <mitkPlaneFit.h>
#include <mitkPointSet.h>
#include <mitkUnstructuredGridClusteringFilter.h>


mitk::ClusteredPlaneSuggestionFilter::ClusteredPlaneSuggestionFilter(): m_Meshing(false), m_MinPts(4), m_Eps(1.2), m_UseDistances(true), m_NumberOfUsedClusters(3)
{
  this->m_MainCluster = mitk::UnstructuredGrid::New();
  this->m_GeoData = mitk::GeometryData::New();
}

mitk::ClusteredPlaneSuggestionFilter::~ClusteredPlaneSuggestionFilter(){}

bool myComparison(std::pair<double,int> i, std::pair<double, int> j){ return (i.first>j.first); }

void mitk::ClusteredPlaneSuggestionFilter::GenerateData()
{
  mitk::UnstructuredGrid::Pointer inpGrid = const_cast<mitk::UnstructuredGrid*>(this->GetInput());

  if(inpGrid.IsNull())
  {
    MITK_ERROR << "Input or cast to UnstructuredGrid is null";
    return;
  }

  mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
  clusterFilter->SetInput(inpGrid);
  clusterFilter->SetMeshing(false);
  clusterFilter->SetMinPts(m_MinPts);
  clusterFilter->Seteps(m_Eps);
  clusterFilter->Update();

  vtkSmartPointer< vtkUnstructuredGrid > vtkGrid = clusterFilter->GetOutput()->GetVtkUnstructuredGrid();

  if(!vtkGrid)
  {
    MITK_ERROR << "vtkUnstructuredGrid output from clustering is null";
    return;
  }

  m_MainCluster->SetVtkUnstructuredGrid(vtkGrid);
  m_Clusters = clusterFilter->GetAllClusters();

  //find the avg distances of every cluster maybe a square mean for better looking at high values?
  std::vector< std::pair<double/*value*/,int/*key/id*/> > avgDistances;

  //get the number of points and the id of every cluster
  std::vector< std::pair<int/*value*/,int/*key/id*/> > sizeIDs;

  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
      mitk::UnstructuredGrid::Pointer cluster = m_Clusters.at(i);
      vtkSmartPointer<vtkDoubleArray> data = dynamic_cast<vtkDoubleArray*>(cluster->GetVtkUnstructuredGrid()->GetPointData()->GetArray(0));
      double avg = 0.0;
      for(int j=0; j<data->GetSize();j++)
      {
          avg += data->GetValue(j);
      }
      avgDistances.push_back(std::make_pair(avg/static_cast<double>(data->GetSize()),i));
  }
  //now sort the clusters with their distances

  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
    sizeIDs.push_back(std::make_pair( m_Clusters.at(i)->GetVtkUnstructuredGrid()->GetNumberOfPoints(), i));
  }

  //sort the point IDs for finding the biggest clusters and clusters with the
  //highest distance
  //sizeIDs is sorted by number of points in each cluster
  //avgDistances is sorted by avg distance to next edge
  std::sort(sizeIDs.begin(), sizeIDs.end(), myComparison);
  std::sort(avgDistances.begin(), avgDistances.end(), myComparison);

  int number = 0; //max number of biggest clusters which are used for the plane

  //if less than m_NumberOfUsedClusters clusters are found
  if(m_Clusters.size() < m_NumberOfUsedClusters)
    number = m_Clusters.size();
  else
    number = m_NumberOfUsedClusters;

  //Generate a pointset from UnstructuredGrid for the PlaneFitFilter:
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  int pointId = 0;

  for(int j=0; j<number; j++) //iterate over the three(or less) biggest clusters
  {
    //Get the cluster with "id" from all clusters, "id" is saved in sizeIDs.second
    //sizeIDs.first represent the number of points which is only needed for sorting the "id"s
    vtkSmartPointer<vtkUnstructuredGrid> tmpGrid;

    if(m_UseDistances)
      tmpGrid = m_Clusters.at(avgDistances.at(j).second)->GetVtkUnstructuredGrid(); //highest distance
    else
      tmpGrid = m_Clusters.at(sizeIDs.at(j).second)->GetVtkUnstructuredGrid(); //biggest cluster

    for(int i=0; i<tmpGrid->GetNumberOfPoints();i++)
    {
      mitk::Point3D point;
      point[0] = tmpGrid->GetPoint(i)[0];
      point[1] = tmpGrid->GetPoint(i)[1];
      point[2] = tmpGrid->GetPoint(i)[2];

      pointset->InsertPoint(pointId,point);
      pointId++;
    }
  }

  mitk::PlaneFit::Pointer planeFilter = mitk::PlaneFit::New();
  planeFilter->SetInput(pointset);
  planeFilter->Update();

  m_GeoData = planeFilter->GetOutput();

  if(m_GeoData.IsNull())
  {
    MITK_ERROR << "GeometryData output from PlaneFit filter is null";
    return;
  }

  avgDistances.clear();
}

void mitk::ClusteredPlaneSuggestionFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::ConstPointer inputImage = this->GetInput();

  m_MainCluster = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;
}
