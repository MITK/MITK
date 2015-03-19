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


mitk::ClusteredPlaneSuggestionFilter::ClusteredPlaneSuggestionFilter(): m_Meshing(false), m_MinPts(4), m_Eps(1.2)
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

//  std::cout << "NUMBER OF INPUT GRID: " << inpGrid->GetVtkUnstructuredGrid()->GetNumberOfPoints() << std::endl;

//  std::cout << "POINTDATA-SIZE OF INPUT GRID: " << inpGrid->GetVtkUnstructuredGrid()->GetPointData()->GetArray(0)->GetSize() << std::endl;

//  std::fstream ds;
//  ds.open("/home/riecker/test.txt");

//  vtkSmartPointer<vtkDoubleArray> array = dynamic_cast<vtkDoubleArray*>(inpGrid->GetVtkUnstructuredGrid()->GetPointData()->GetArray(0));
//  for(int i=0; i<array->GetSize();i++)
//  {
//    ds << "point: " << i << " distance " << array->GetValue(i) << std::endl;
//  }
//  ds.close();

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

  std::fstream fs;
  fs.open("/home/riecker/clusters.txt");

  //test if the scalar transfer works
  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
      fs << "Cluster-ID: " << i << std::endl;
      mitk::UnstructuredGrid::Pointer cluster = m_Clusters.at(i);
      vtkSmartPointer<vtkDoubleArray> data = dynamic_cast<vtkDoubleArray*>(cluster->GetVtkUnstructuredGrid()->GetPointData()->GetArray(0));
      for(int j=0; j<data->GetSize();j++)
      {
          fs << "Point " << j << " - Distance: " << data->GetValue(j) << std::endl;
      }
  }
  fs.close();
  //end

  //find the avg distances of every cluster maybe a square mean for better looking at high values?
  std::vector< std::pair<double/*value*/,int/*key/id*/> > avgDistances;
//  std::vector< double > avgDistances;
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

  //get the number of points and the id of every cluster
  std::vector< std::pair<int/*value*/,int/*key/id*/> > sizeIDs;

  for(unsigned int i=0; i<m_Clusters.size();i++)
  {
    sizeIDs.push_back(std::make_pair( m_Clusters.at(i)->GetVtkUnstructuredGrid()->GetNumberOfPoints(), i));
//    std::cout << "CLUSTER ID: " << i << " NUMBER OF POINTS: " << m_Clusters.at(i)->GetVtkUnstructuredGrid()->GetNumberOfPoints() << std::endl;
  }

  //sort the number of points for finding the three biggest
  //sizeIDs is sorted by number of points in each cluster
  //avgDistances is sorted by avg distance to next edge
  std::sort(sizeIDs.begin(), sizeIDs.end(), myComparison);
  std::sort(avgDistances.begin(), avgDistances.end(), myComparison);

  for(unsigned int i=0; i<avgDistances.size();i++)
  {
    std::cout << "DISTANCE OF CLUSTER " << i << ": " << avgDistances.at(i).first << std::endl;
  }

  //calc the overall mean distance
//  double overallDistance = 0.0;
//  for(unsigned int i=0; i<avgDistances.size();i++)
//  {
//    double dist = avgDistances.at(i).first;
//    if(dist==dist)
//      overallDistance += dist;
//  }
//  overallDistance /= static_cast<double>(avgDistances.size());
  //end calc the overall mean distance

  int number = 0; //max number of biggest clusters which are used for the plane

  //if less than 3 clusters are found
//  if(m_Clusters.size()<3)
    number = m_Clusters.size();
//  else
//    number = 3;

//    double mean = 0.0;
//    for(unsigned int i=0; i<m_Clusters.size();i++)
//    {
//      mean += sizeIDs.at(i).first;
//    }
//    mean /= static_cast<double>(m_Clusters.size());

//    for(unsigned int i=0; i<m_Clusters.size();i++)
//    {
//      if(sizeIDs.at(i).first<mean)
//      {
//        number = i;
//        break;
//      }
//    }

//    std::cout << "MEAN: " << mean << std::endl;
//    std::cout << "NUMBER: " << number << std::endl;

//    if(number<2 && m_Clusters.size()>=2)
//      number = 2;

  //calc the number of clusters which distances are higher than the average
//  for(unsigned int i=0; i<avgDistances.size();i++)
//  {
//    std::cout << "Cluster " << i << ": " << avgDistances.at(i).first << " >= " << overallDistance << std::endl;
//    if(avgDistances.at(i).first >= overallDistance)
//      number++;
//  }
//  std::cout << "Number of clusters which distance is higher than the average: " << number << std::endl;

    //filter small clusters out!
//    int pointsOverAll = 0;
//    for(unsigned int i=0; i<m_Clusters.size();i++)
//    {
//      pointsOverAll += m_Clusters.at(i)->GetVtkUnstructuredGrid()->GetNumberOfPoints();
//    }
//    int avg = pointsOverAll / m_Clusters.size();

//    for(unsigned int i=0; i<m_Clusters.size();i++)
//    {
//      std::cout << "Cluster ID " << avgDistances.at(i).second << " Distance: " << avgDistances.at(i).first << std::endl;
//      if(sizeIDs.at(i).first >= avg)
//        number++;
//      else
//        break;
//    }

//    std::cout << "NUMBER OF CLUSTERS: " << number << std::endl;


  //Generate a pointset from UnstructuredGrid for the PlaneFitFilter:
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  int pointId = 0;

  for(int j=0; j<number; j++) //iterate over the three(or less) biggest clusters
  {
    //Get the cluster with "id" from all clusters, "id" is saved in sizeIDs.second
    //sizeIDs.first represent the number of points which is only needed for sorting the "id"s
//    vtkSmartPointer<vtkUnstructuredGrid> tmpGrid = m_Clusters.at(avgDistances.at(j).second)->GetVtkUnstructuredGrid(); //highest distance
    vtkSmartPointer<vtkUnstructuredGrid> tmpGrid = m_Clusters.at(sizeIDs.at(j).second)->GetVtkUnstructuredGrid(); //biggest cluster
//    std::cout << "CLUSTER ID: " << avgDistances.at(j).second << " NUMBER OF POINTS: " <<  avgDistances.at(j).first << std::endl;
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

//  std::cout << "POINTSET POINTS: " << pointset->GetSize() << std::endl;

  mitk::PlaneFit::Pointer planeFilter = mitk::PlaneFit::New();
  planeFilter->SetInput(pointset);
  planeFilter->Update();

  m_GeoData = planeFilter->GetOutput();

  if(m_GeoData.IsNull())
  {
    MITK_ERROR << "GeometryData output from PlaneFit filter is null";
    return;
  }

  m_Clusters.clear();
  avgDistances.clear();
//  mitk::PlaneGeometry* planeGeometry = dynamic_cast<mitk::PlaneGeometry*>( planeFilter->GetOutput()->GetGeometry());
}

void mitk::ClusteredPlaneSuggestionFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::ConstPointer inputImage = this->GetInput();

  m_MainCluster = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;
}
