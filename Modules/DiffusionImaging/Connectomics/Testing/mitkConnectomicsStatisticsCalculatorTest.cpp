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

// macros
#include "mitkTestingMacros.h"

// std includes
#include <string>
#include <sstream>

// MITK includes
#include <mitkBaseDataIOFactory.h>
#include <mitkConnectomicsObjectFactory.h>
#include <mitkConnectomicsStatisticsCalculator.h>

static void CalculateStatistics(std::string networkName, std::string outName, std::vector< std::vector< std::string > > vectorOfLabelVectors )
{
  const std::string s1="", s2="";
  RegisterConnectomicsObjectFactory();

  // load network
  std::vector<mitk::BaseData::Pointer> networkFile =
    mitk::BaseDataIO::LoadBaseDataFromFile( networkName, s1, s2, false );
  if( networkFile.empty() )
  {
    std::string errorMessage = "File at " + networkName + " could not be read. Aborting.";
    MITK_TEST_CONDITION_REQUIRED( !networkFile.empty(), errorMessage);
    return;
  }
  mitk::BaseData* networkBaseData = networkFile.at(0);
  mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( networkBaseData );

  if( !network )
  {
    std::string errorMessage = "Read file at " + networkName + " could not be recognized as network. Aborting.";
    MITK_TEST_CONDITION_REQUIRED( network, errorMessage);
    return;
  }

  mitk::ConnectomicsStatisticsCalculator::Pointer statisticsCalculator = mitk::ConnectomicsStatisticsCalculator::New();

  statisticsCalculator->SetNetwork( network );
  statisticsCalculator->Update();

  std::stringstream headerStream;

  if( outName == "")
  {
    // test mode
    MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfVertices(), 4 , mitk::eps, true ),  "GetNumberOfVertices" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfEdges(), 5 , mitk::eps, true ),  "GetNumberOfEdges" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageDegree(), 2.5 , mitk::eps, true ),  "GetAverageDegree" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetConnectionDensity(), 0.833333 , mitk::eps, true ),  "GetConnectionDensity" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfConnectedComponents(), 1 , mitk::eps, true ),  "GetNumberOfConnectedComponents" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageComponentSize(), 4 , mitk::eps, true ),  "GetAverageComponentSize" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetLargestComponentSize(), 4 , mitk::eps, true ),  "GetLargestComponentSize" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRatioOfNodesInLargestComponent(), 1 , mitk::eps, true ),  "GetRatioOfNodesInLargestComponent" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetHopPlotExponent(), 0.192645 , mitk::eps, true ),  "GetHopPlotExponent" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetEffectiveHopDiameter(), 1 , mitk::eps, true ),  "GetEffectiveHopDiameter" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsC(), 0.833333 , mitk::eps, true ),  "GetAverageClusteringCoefficientsC" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsD(), 0.916667 , mitk::eps, true ),  "GetAverageClusteringCoefficientsD" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageClusteringCoefficientsE(), 0.833333 , mitk::eps, true ),  "GetAverageClusteringCoefficientsE" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageVertexBetweennessCentrality(), 0.25 , mitk::eps, true ),  "GetAverageVertexBetweennessCentrality" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageEdgeBetweennessCentrality(), 1.4 , mitk::eps, true ),  "GetAverageEdgeBetweennessCentrality" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfIsolatedPoints(), 0 , mitk::eps, true ),  "GetNumberOfIsolatedPoints" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRatioOfIsolatedPoints(), 0 , mitk::eps, true ),  "GetRatioOfIsolatedPoints" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfEndPoints(), 0 , mitk::eps, true ),  "GetNumberOfEndPoints" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRatioOfEndPoints(), 0 , mitk::eps, true ),  "GetRatioOfEndPoints" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetDiameter(), 2 , mitk::eps, true ),  "GetDiameter" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetDiameter90(), 2 , mitk::eps, true ),  "GetDiameter90" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRadius(), 1 , mitk::eps, true ),  "GetRadius" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRadius90(), 1 , mitk::eps, true ),  "GetRadius90" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageEccentricity(), 1.5 , mitk::eps, true ),  "GetAverageEccentricity" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAverageEccentricity90(), 1.5 , mitk::eps, true ),  "GetAverageEccentricity90" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAveragePathLength(), 1.16667 , mitk::eps, true ),  "GetAveragePathLength" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNumberOfCentralPoints(), 2 , mitk::eps, true ),  "GetNumberOfCentralPoints" )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetRatioOfCentralPoints(), 0.5 , mitk::eps, true ),  "GetRatioOfCentralPoints" )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetSpectralRadius(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetSecondLargestEigenValue(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAdjacencyTrace(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetAdjacencyEnergy(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetLaplacianTrace(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetLaplacianEnergy(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetLaplacianSpectralGap(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianTrace(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianEnergy(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf2s(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf1s(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianNumberOf0s(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianLowerSlope(), , mitk::eps, true ),  " " )
      // MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetNormalizedLaplacianUpperSlope(), , mitk::eps, true ),  " " )
      MITK_TEST_CONDITION_REQUIRED( mitk::Equal( statisticsCalculator->GetSmallWorldness(), 1.72908 , mitk::eps, true ),  "GetSmallWorldness" )
  }
  else
  {
    // batch mode
    headerStream  << "NumberOfVertices "
      << "NumberOfEdges "
      << "AverageDegree "
      << "ConnectionDensity "
      << "NumberOfConnectedComponents "
      << "AverageComponentSize "
      << "LargestComponentSize "
      << "RatioOfNodesInLargestComponent "
      << "HopPlotExponent "
      << "EffectiveHopDiameter "
      << "AverageClusteringCoefficientsC "
      << "AverageClusteringCoefficientsD "
      << "AverageClusteringCoefficientsE "
      << "AverageVertexBetweennessCentrality "
      << "AverageEdgeBetweennessCentrality "
      << "NumberOfIsolatedPoints "
      << "RatioOfIsolatedPoints "
      << "NumberOfEndPoints "
      << "RatioOfEndPoints "
      << "Diameter "
      << "Diameter90 "
      << "Radius "
      << "Radius90 "
      << "AverageEccentricity "
      << "AverageEccentricity90 "
      << "AveragePathLength "
      << "NumberOfCentralPoints "
      << "RatioOfCentralPoints "
      << "SpectralRadius "
      << "SecondLargestEigenValue "
      << "AdjacencyTrace "
      << "AdjacencyEnergy "
      << "LaplacianTrace "
      << "LaplacianEnergy "
      << "LaplacianSpectralGap "
      << "NormalizedLaplacianTrace "
      << "NormalizedLaplacianEnergy "
      << "NormalizedLaplacianNumberOf2s "
      << "NormalizedLaplacianNumberOf1s "
      << "NormalizedLaplacianNumberOf0s "
      << "NormalizedLaplacianLowerSlope "
      << "NormalizedLaplacianUpperSlope"
      << "SmallWorldness"
      << std::endl;

    std::stringstream dataStream;
    dataStream << statisticsCalculator->GetNumberOfVertices() << " "
      << statisticsCalculator->GetNumberOfEdges() << " "
      << statisticsCalculator->GetAverageDegree() << " "
      << statisticsCalculator->GetConnectionDensity() << " "
      << statisticsCalculator->GetNumberOfConnectedComponents() << " "
      << statisticsCalculator->GetAverageComponentSize() << " "
      << statisticsCalculator->GetLargestComponentSize() << " "
      << statisticsCalculator->GetRatioOfNodesInLargestComponent() << " "
      << statisticsCalculator->GetHopPlotExponent() << " "
      << statisticsCalculator->GetEffectiveHopDiameter() << " "
      << statisticsCalculator->GetAverageClusteringCoefficientsC() << " "
      << statisticsCalculator->GetAverageClusteringCoefficientsD() << " "
      << statisticsCalculator->GetAverageClusteringCoefficientsE() << " "
      << statisticsCalculator->GetAverageVertexBetweennessCentrality() << " "
      << statisticsCalculator->GetAverageEdgeBetweennessCentrality() << " "
      << statisticsCalculator->GetNumberOfIsolatedPoints() << " "
      << statisticsCalculator->GetRatioOfIsolatedPoints() << " "
      << statisticsCalculator->GetNumberOfEndPoints() << " "
      << statisticsCalculator->GetRatioOfEndPoints() << " "
      << statisticsCalculator->GetDiameter() << " "
      << statisticsCalculator->GetDiameter90() << " "
      << statisticsCalculator->GetRadius() << " "
      << statisticsCalculator->GetRadius90() << " "
      << statisticsCalculator->GetAverageEccentricity() << " "
      << statisticsCalculator->GetAverageEccentricity90() << " "
      << statisticsCalculator->GetAveragePathLength() << " "
      << statisticsCalculator->GetNumberOfCentralPoints() << " "
      << statisticsCalculator->GetRatioOfCentralPoints() << " "
      << statisticsCalculator->GetSpectralRadius() << " "
      << statisticsCalculator->GetSecondLargestEigenValue() << " "
      << statisticsCalculator->GetAdjacencyTrace() << " "
      << statisticsCalculator->GetAdjacencyEnergy() << " "
      << statisticsCalculator->GetLaplacianTrace() << " "
      << statisticsCalculator->GetLaplacianEnergy() << " "
      << statisticsCalculator->GetLaplacianSpectralGap() << " "
      << statisticsCalculator->GetNormalizedLaplacianTrace() << " "
      << statisticsCalculator->GetNormalizedLaplacianEnergy() << " "
      << statisticsCalculator->GetNormalizedLaplacianNumberOf2s() << " "
      << statisticsCalculator->GetNormalizedLaplacianNumberOf1s() << " "
      << statisticsCalculator->GetNormalizedLaplacianNumberOf0s() << " "
      << statisticsCalculator->GetNormalizedLaplacianLowerSlope() << " "
      << statisticsCalculator->GetNormalizedLaplacianUpperSlope() << " "
      << statisticsCalculator->GetSmallWorldness()
      << std::endl;

    ofstream outFile( outName.c_str(), ios::out );

    if( ! outFile.is_open() )
    {
      std::string errorMessage = "Could not open " + outName + " for writing.";
      MITK_TEST_CONDITION_REQUIRED( outFile.is_open(), errorMessage);
      return;
    }

    outFile << headerStream.str() << dataStream.str();
    outFile.close();
  }
}

/**
* @brief mitkConnectomicsStatisticsCalculatorTest A test of a number of network statistics
*/
int mitkConnectomicsStatisticsCalculatorTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN(mitkConnectomicsStatisticsCalculatorTest);

  bool invalidNumberOfArguments( argc == 1 );

  if( invalidNumberOfArguments )
  {
    std::string errorMessage = "Wrong number of arguments.\nUsage: <network-filename> <network-out-filename> [list of local labels]";
    MITK_TEST_CONDITION_REQUIRED( invalidNumberOfArguments, errorMessage);
    return 0;
  }

  std::string networkName = argv[1];
  std::string outName = "";
  std::vector< std::vector< std::string > > vectorOfLabelVectors;

  if( argc > 2 )
  {
    outName = argv[2];
    for(int loop(3); loop < argc; loop++ )
    {
      vectorOfLabelVectors.at(0).push_back( argv[loop] );
    }
  }

  CalculateStatistics( networkName, outName, vectorOfLabelVectors );

  MITK_TEST_END();
}
