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

#include "MiniAppManager.h"

// std includes
#include <string>
#include <sstream>

// CTK includes
#include "ctkCommandLineParser.h"

// MITK includes
#include <mitkBaseDataIOFactory.h>
#include <mitkConnectomicsObjectFactory.h>
#include <mitkConnectomicsStatisticsCalculator.h>

int NetworkStatistics(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("inputNetwork", "i", ctkCommandLineParser::String, "input connectomics network (.cnf)", us::Any(), false);
  parser.addArgument("outputFile", "o", ctkCommandLineParser::String, "name of output file", us::Any(), false);

  parser.addArgument("noGlobalStatistics", "g", ctkCommandLineParser::Bool, "Do not calculate global statistics");


  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  //default values
  bool noGlobalStatistics( false );

  // parse command line arguments
  std::string networkName = us::any_cast<std::string>(parsedArgs["inputNetwork"]);
  std::string outName = us::any_cast<std::string>(parsedArgs["outputFile"]);



  if (parsedArgs.count("noGlobalStatistics"))
    noGlobalStatistics = us::any_cast<bool>(parsedArgs["noGlobalStatistics"]);

  try
  {
    const std::string s1="", s2="";
    RegisterConnectomicsObjectFactory();

    // load network
    std::vector<mitk::BaseData::Pointer> networkFile =
      mitk::BaseDataIO::LoadBaseDataFromFile( networkName, s1, s2, false );
    if( networkFile.empty() )
    {
      std::string errorMessage = "File at " + networkName + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData* networkBaseData = networkFile.at(0);
    mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( networkBaseData );

    if( !network )
    {
      std::string errorMessage = "Read file at " + networkName + " could not be recognized as network. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }

    mitk::ConnectomicsStatisticsCalculator::Pointer statisticsCalculator = mitk::ConnectomicsStatisticsCalculator::New();

    statisticsCalculator->SetNetwork( network );
    statisticsCalculator->Update();

    std::stringstream headerStream;

    // global statistics
    if( !noGlobalStatistics )
    {
      std::string globalOutName = outName + "_global.txt";
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

      ofstream outFile( globalOutName.c_str(), ios::out );

      if( ! outFile.is_open() )
      {
        std::string errorMessage = "Could not open " + globalOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }

      outFile << headerStream.str() << dataStream.str();
      outFile.close();
    } // end global statistics
  }
  catch (itk::ExceptionObject e)
  {
    MITK_INFO << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    MITK_INFO << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_INFO << "ERROR!?!";
    return EXIT_FAILURE;
  }
  MITK_INFO << "DONE";
  return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(NetworkStatistics);
