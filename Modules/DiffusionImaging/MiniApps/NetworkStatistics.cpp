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
#include <vector>
#include <map>

// ITK includes
#include <itkImageFileWriter.h>

// CTK includes
#include "ctkCommandLineParser.h"

// MITK includes
#include <mitkBaseDataIOFactory.h>
#include <mitkConnectomicsObjectFactory.h>
#include <mitkConnectomicsStatisticsCalculator.h>
#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>

int NetworkStatistics(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("inputNetwork", "i", ctkCommandLineParser::String, "input connectomics network (.cnf)", us::Any(), false);
  parser.addArgument("outputFile", "o", ctkCommandLineParser::String, "name of output file", us::Any(), false);

  parser.addArgument("noGlobalStatistics", "g", ctkCommandLineParser::Bool, "Do not calculate global statistics");
  parser.addArgument("createConnectivityMatriximage", "I", ctkCommandLineParser::Bool, "Write connectivity matrix image");
  parser.addArgument("binaryConnectivity", "b", ctkCommandLineParser::Bool, "Whether to create a binary connectivity matrix");
  parser.addArgument("rescaleConnectivity", "r", ctkCommandLineParser::Bool, "Whether to rescale the connectivity matrix");
  parser.addArgument("localStatistics", "L", ctkCommandLineParser::StringList, "Provide a list of node labels for local statistics", us::Any());


  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  //default values
  bool noGlobalStatistics( false );
  bool binaryConnectivity( false );
  bool rescaleConnectivity( false );
  bool createConnectivityMatriximage( false );


  // parse command line arguments
  std::string networkName = us::any_cast<std::string>(parsedArgs["inputNetwork"]);
  std::string outName = us::any_cast<std::string>(parsedArgs["outputFile"]);

  ctkCommandLineParser::StringContainerType localLabels;

  if(parsedArgs.count("localStatistics"))
  {
    localLabels = us::any_cast<ctkCommandLineParser::StringContainerType>(parsedArgs["localStatistics"]);
  }

  if (parsedArgs.count("noGlobalStatistics"))
    noGlobalStatistics = us::any_cast<bool>(parsedArgs["noGlobalStatistics"]);
  if (parsedArgs.count("binaryConnectivity"))
    binaryConnectivity = us::any_cast<bool>(parsedArgs["binaryConnectivity"]);
  if (parsedArgs.count("rescaleConnectivity"))
    rescaleConnectivity = us::any_cast<bool>(parsedArgs["rescaleConnectivity"]);
  if (parsedArgs.count("createConnectivityMatriximage"))
    createConnectivityMatriximage = us::any_cast<bool>(parsedArgs["createConnectivityMatriximage"]);

  try
  {
    const std::string s1="", s2="";

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

    // global statistics
    if( !noGlobalStatistics )
    {
      std::string globalOutName = outName + "_global.txt";
      std::stringstream headerStream;
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

      std::ofstream outFile( globalOutName.c_str(), ios::out );

      if( ! outFile.is_open() )
      {
        std::string errorMessage = "Could not open " + globalOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }

      outFile << headerStream.str() << dataStream.str();
      outFile.close();
    } // end global statistics

    //create connectivity matrix png
    if( createConnectivityMatriximage )
    {
      std::string connectivity_png_postfix = "_connectivity";
      if( binaryConnectivity )
      {
        connectivity_png_postfix += "_binary";
      }
      else if( rescaleConnectivity )
      {
        connectivity_png_postfix += "_rescaled";
      }
      connectivity_png_postfix += ".png";

      /* File format
      * A png file depicting the binary connectivity matrix
      */
      itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::Pointer filter = itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::New();
      filter->SetInputNetwork( network );
      filter->SetBinaryConnectivity( binaryConnectivity );
      filter->SetRescaleConnectivity( rescaleConnectivity );
      filter->Update();

      typedef itk::ConnectomicsNetworkToConnectivityMatrixImageFilter::OutputImageType connectivityMatrixImageType;

      itk::ImageFileWriter< connectivityMatrixImageType >::Pointer connectivityWriter = itk::ImageFileWriter< connectivityMatrixImageType >::New();

      connectivityWriter->SetInput( filter->GetOutput() );
      connectivityWriter->SetFileName( outName + connectivity_png_postfix);
      connectivityWriter->Update();

      MITK_INFO << "Connectivity matrix image written.";
    } // end create connectivity matrix png

    // calculate local averages
    if( localLabels.size() > 0 )
    {
      std::string localOutName = outName + "_local.txt";

      // Create LabelToIndex translation
      std::map< std::string, int > labelToIdMap;
      std::vector< mitk::ConnectomicsNetwork::NetworkNode > nodeVector = network->GetVectorOfAllNodes();
      for(int loop(0); loop < nodeVector.size(); loop++)
      {
        labelToIdMap.insert( std::pair< std::string, int>(nodeVector.at(loop).label, nodeVector.at(loop).id) );
      }

      std::vector< int > degreeVector = network->GetDegreeOfNodes();
      std::vector< double > ccVector = network->GetLocalClusteringCoefficients( );
      std::vector< double > bcVector = network->GetNodeBetweennessVector( );

      double sumDegree( 0 );
      double sumCC( 0 );
      double sumBC( 0 );
      double count( 0 );

      for( int loop(0); loop < localLabels.size(); loop++ )
      {
        if( network->CheckForLabel(localLabels.at( loop )) )
        {
          sumDegree = sumDegree + degreeVector.at( labelToIdMap.find( localLabels.at( loop ) )->second );
          sumCC = sumCC + ccVector.at( labelToIdMap.find( localLabels.at( loop ) )->second );
          sumBC = sumBC + bcVector.at( labelToIdMap.find( localLabels.at( loop ) )->second );
          count = count + 1;
        }
        else
        {
          MITK_ERROR << "Illegal label. Label: \"" << localLabels.at( loop ) << "\" not found.";
        }
      }

      std::stringstream headerStream;
      headerStream  << "LocalAverageDegree "
        << "LocalAverageCC "
        << "LocalAverageBC "
        << "NumberOfNodes"
        << std::endl;

      std::stringstream dataStream;
      dataStream  << sumDegree / count << " "
        << sumCC / count << " "
        << sumBC / count << " "
        << count
        << std::endl;

      std::ofstream outFile( localOutName.c_str(), ios::out );

      if( ! outFile.is_open() )
      {
        std::string errorMessage = "Could not open " + localOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }

      outFile << headerStream.str() << dataStream.str();
      outFile.close();
    }// end calculate local averages

    return EXIT_SUCCESS;
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
