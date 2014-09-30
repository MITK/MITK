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

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

// std includes
#include <string>
#include <sstream>

// MITK includes
#include <mitkIOUtil.h>
#include <mitkConnectomicsStatisticsCalculator.h>

// VTK includes
#include <vtkDebugLeaks.h>


class mitkConnectomicsStatisticsCalculatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkConnectomicsStatisticsCalculatorTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18097.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(StatisticsCalculatorUpdate);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ConnectomicsNetwork::Pointer m_Network;
  std::string m_NetworkPath;

public:

  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp()
  {
    //load network

    m_NetworkPath = GetTestDataFilePath("DiffusionImaging/Connectomics/reference.cnf");

    std::vector<mitk::BaseData::Pointer> networkFile = mitk::IOUtil::Load( m_NetworkPath );
    if( networkFile.empty() )
    {
      std::string errorMessage = "File at " + m_NetworkPath + " could not be read. Aborting.";
      CPPUNIT_ASSERT_MESSAGE( errorMessage, !networkFile.empty() );
      return;
    }
    mitk::BaseData* networkBaseData = networkFile.at(0);
    mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( networkBaseData );

    if( !network )
    {
      std::string errorMessage = "Read file at " + m_NetworkPath + " could not be recognized as network. Aborting.";
      CPPUNIT_ASSERT_MESSAGE( errorMessage, network);
      return;
    }

    m_Network = network;
  }

  void tearDown()
  {
    m_Network = NULL;
    m_NetworkPath = "";
  }

  void StatisticsCalculatorUpdate()
  {
    mitk::ConnectomicsStatisticsCalculator::Pointer statisticsCalculator = mitk::ConnectomicsStatisticsCalculator::New();

    statisticsCalculator->SetNetwork( m_Network );
    statisticsCalculator->Update();

    double eps( 0.0001 );

    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfVertices", mitk::Equal( statisticsCalculator->GetNumberOfVertices( ), 4 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfEdges" , mitk::Equal( statisticsCalculator->GetNumberOfEdges( ), 5 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageDegree", mitk::Equal( statisticsCalculator->GetAverageDegree( ), 2.5 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetConnectionDensity", mitk::Equal( statisticsCalculator->GetConnectionDensity( ), 0.833333 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfConnectedComponents", mitk::Equal( statisticsCalculator->GetNumberOfConnectedComponents( ), 1 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageComponentSize", mitk::Equal( statisticsCalculator->GetAverageComponentSize( ), 4 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetLargestComponentSize", mitk::Equal( statisticsCalculator->GetLargestComponentSize( ), 4 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRatioOfNodesInLargestComponent", mitk::Equal( statisticsCalculator->GetRatioOfNodesInLargestComponent( ), 1 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetHopPlotExponent", mitk::Equal( statisticsCalculator->GetHopPlotExponent( ), 0.192645 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetEffectiveHopDiameter", mitk::Equal( statisticsCalculator->GetEffectiveHopDiameter( ), 1 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageClusteringCoefficientsC", mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsC( ), 0.833333 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageClusteringCoefficientsD", mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsD( ), 0.916667 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageClusteringCoefficientsE", mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsE( ), 0.833333 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageVertexBetweennessCentrality", mitk::Equal( statisticsCalculator->GetAverageVertexBetweennessCentrality( ), 0.25 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageEdgeBetweennessCentrality", mitk::Equal( statisticsCalculator->GetAverageEdgeBetweennessCentrality( ), 1.4 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfIsolatedPoints" , mitk::Equal( statisticsCalculator->GetNumberOfIsolatedPoints( ), 0 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRatioOfIsolatedPoints", mitk::Equal( statisticsCalculator->GetRatioOfIsolatedPoints( ), 0 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfEndPoints", mitk::Equal( statisticsCalculator->GetNumberOfEndPoints( ), 0 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRatioOfEndPoints", mitk::Equal( statisticsCalculator->GetRatioOfEndPoints( ), 0 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetDiameter", mitk::Equal( statisticsCalculator->GetDiameter( ), 2 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetDiameter90", mitk::Equal( statisticsCalculator->GetDiameter90( ), 2 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRadius" , mitk::Equal( statisticsCalculator->GetRadius( ), 1 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRadius90", mitk::Equal( statisticsCalculator->GetRadius90( ), 1 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageEccentricity", mitk::Equal( statisticsCalculator->GetAverageEccentricity( ), 1.5 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAverageEccentricity90", mitk::Equal( statisticsCalculator->GetAverageEccentricity90( ), 1.5 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetAveragePathLength" , mitk::Equal( statisticsCalculator->GetAveragePathLength( ), 1.16667 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetNumberOfCentralPoints" , mitk::Equal( statisticsCalculator->GetNumberOfCentralPoints( ), 2 , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetRatioOfCentralPoints", mitk::Equal( statisticsCalculator->GetRatioOfCentralPoints( ), 0.5 , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetSpectralRadius( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetSecondLargestEigenValue( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetAdjacencyTrace( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetAdjacencyEnergy( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetLaplacianTrace( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetLaplacianEnergy( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetLaplacianSpectralGap( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianTrace( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianEnergy( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf2s( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf1s( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf0s( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianLowerSlope( ), , eps, true ) );
    // CPPUNIT_ASSERT_MESSAGE( " ", mitk::Equal( statisticsCalculator->GetNormalizedLaplacianUpperSlope( ), , eps, true ) );
    CPPUNIT_ASSERT_MESSAGE( "GetSmallWorldness", mitk::Equal( statisticsCalculator->GetSmallWorldness( ), 1.72908 , eps, true ) );

  }
};

MITK_TEST_SUITE_REGISTRATION(mitkConnectomicsStatisticsCalculator)
