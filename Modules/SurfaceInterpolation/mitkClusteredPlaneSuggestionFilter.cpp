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

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointLocator.h>
#include <vtkPolyVertex.h>

#include <mitkPlaneFit.h>
#include <mitkPointSet.h>
#include <mitkUnstructuredGridClusteringFilter.h>


mitk::ClusteredPlaneSuggestionFilter::ClusteredPlaneSuggestionFilter(): m_Meshing(false), m_MinPts(4), m_Eps(1.2)
{
  this->m_MainCluster = mitk::UnstructuredGrid::New();
  this->m_GeoData = mitk::GeometryData::New();
}

mitk::ClusteredPlaneSuggestionFilter::~ClusteredPlaneSuggestionFilter(){}

void mitk::ClusteredPlaneSuggestionFilter::GenerateData()
{
  mitk::UnstructuredGrid::Pointer inpGrid = const_cast<mitk::UnstructuredGrid*>(this->GetInput());

  mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
  clusterFilter->SetInput(inpGrid);
  clusterFilter->SetMeshing(false);
  clusterFilter->SetMinPts(4);
  clusterFilter->Seteps(1.2);
  clusterFilter->Update();

  m_Clusters = clusterFilter->GetAllClusters();

  vtkSmartPointer< vtkUnstructuredGrid > vtkGrid = clusterFilter->GetOutput()->GetVtkUnstructuredGrid();
  m_MainCluster->SetVtkUnstructuredGrid(vtkGrid);

  //Generate a pointset from UnstructuredGrid for the PlaneFitFilter:
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  for(int i=0; i<vtkGrid->GetNumberOfPoints();i++)
  {
    mitk::Point3D point;
    point[0] = vtkGrid->GetPoint(i)[0];
    point[1] = vtkGrid->GetPoint(i)[1];
    point[2] = vtkGrid->GetPoint(i)[2];

    pointset->InsertPoint(i,point);
  }

  mitk::PlaneFit::Pointer planeFilter = mitk::PlaneFit::New();
  planeFilter->SetInput(pointset);
  planeFilter->Update();

  m_GeoData = planeFilter->GetOutput();

//  mitk::PlaneGeometry* planeGeometry = dynamic_cast<mitk::PlaneGeometry*>( planeFilter->GetOutput()->GetGeometry());
}

void mitk::ClusteredPlaneSuggestionFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::ConstPointer inputImage = this->GetInput();

  m_MainCluster = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;
}
