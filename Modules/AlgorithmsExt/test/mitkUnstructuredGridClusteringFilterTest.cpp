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

#include "mitkTestingMacros.h"
#include <mitkUnstructuredGridClusteringFilter.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>
#include <mitkTestFixture.h>
#include <mitkUnstructuredGrid.h>

#include <mitkIOUtil.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDebugLeaks.h>

class mitkUnstructuredGridClusteringFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUnstructuredGridClusteringFilterTestSuite);

  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(testUnstructuredGridClusteringFilterInitialization);
  MITK_TEST(testInput);
  MITK_TEST(testUnstructuredGridGeneration);
  MITK_TEST(testReturnedCluster);
  MITK_TEST(testClusterVector);
  MITK_TEST(testGetNumberOfFoundClusters);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::UnstructuredGrid::Pointer m_UnstructuredGrid;

public:

  void setUp() override
  {
    m_UnstructuredGrid = mitk::UnstructuredGrid::New();

    //Loading the test data
    std::vector< mitk::BaseData::Pointer > vector = mitk::IOUtil::Load(GetTestDataFilePath("UnstructuredGrid/scoredGrid.vtu"));
    mitk::BaseData::Pointer base = vector.at(0);
    m_UnstructuredGrid = dynamic_cast<mitk::UnstructuredGrid*>(base.GetPointer());
  }

  void testUnstructuredGridClusteringFilterInitialization()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of filter object", clusterFilter.IsNotNull());
  }

  void testInput()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    clusterFilter->SetInput(m_UnstructuredGrid);
    CPPUNIT_ASSERT_MESSAGE("Testing set / get input!", clusterFilter->GetInput() == m_UnstructuredGrid);
  }

  void testUnstructuredGridGeneration()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    clusterFilter->SetInput(m_UnstructuredGrid);
    clusterFilter->SetMeshing(false);
    clusterFilter->SetMinPts(4);
    clusterFilter->Seteps(1.2);
    clusterFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing output generation!", clusterFilter->GetOutput() != NULL);
  }

  void testReturnedCluster()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    clusterFilter->SetInput(m_UnstructuredGrid);
    clusterFilter->SetMeshing(false);
    clusterFilter->SetMinPts(4);
    clusterFilter->Seteps(1.2);
    clusterFilter->Update();
    mitk::UnstructuredGrid::Pointer cluster = clusterFilter->GetOutput();
    CPPUNIT_ASSERT_MESSAGE("Testing the output cluster!", cluster->GetVtkUnstructuredGrid()->GetPoints()->GetNumberOfPoints() == 620);
  }

  void testClusterVector()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    clusterFilter->SetInput(m_UnstructuredGrid);
    clusterFilter->SetMeshing(false);
    clusterFilter->SetMinPts(4);
    clusterFilter->Seteps(1.2);
    clusterFilter->Update();
    std::vector< mitk::UnstructuredGrid::Pointer > clustervector = clusterFilter->GetAllClusters();
    //test that all clusters have points:
    bool havePoints = true;
    for(unsigned int i=0; i<clustervector.size(); i++)
    {
      mitk::UnstructuredGrid::Pointer grid = clustervector.at(i);
      if(grid->GetVtkUnstructuredGrid()->GetPoints()->GetNumberOfPoints()<1)
        havePoints = false;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing number of found clusters!", havePoints && clustervector.size() == 17);
  }

  void testGetNumberOfFoundClusters()
  {
    mitk::UnstructuredGridClusteringFilter::Pointer clusterFilter = mitk::UnstructuredGridClusteringFilter::New();
    clusterFilter->SetInput(m_UnstructuredGrid);
    clusterFilter->SetMeshing(false);
    clusterFilter->SetMinPts(4);
    clusterFilter->Seteps(1.2);
    clusterFilter->Update();
    CPPUNIT_ASSERT_MESSAGE("Testing number of found clusters!", clusterFilter->GetNumberOfFoundClusters() == 17);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkUnstructuredGridClusteringFilter)
