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

// std includes
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <utility>

// boost includes
#include <boost/algorithm/string.hpp>

// ITK includes
#include <itkImageFileWriter.h>

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include <mitkConnectomicsStatisticsCalculator.h>
#include <mitkConnectomicsNetworkThresholder.h>
#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>
#include <mitkIOUtil.h>
#include <mitkLocaleSwitch.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Network Creation");
  parser.setCategory("Connectomics");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");

  parser.addArgument("", "i", mitkCommandLineParser::String, "Input network", "input connectomics network (.cnf)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output file", "name of output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("noGlobalStatistics", "g", mitkCommandLineParser::Bool, "No global statistics", "Do not calculate global statistics");
  parser.addArgument("createConnectivityMatriximage", "I", mitkCommandLineParser::Bool, "Write connectivity matrix image", "Write connectivity matrix image");
  parser.addArgument("binaryConnectivity", "b", mitkCommandLineParser::Bool, "Binary connectivity", "Whether to create a binary connectivity matrix");
  parser.addArgument("rescaleConnectivity", "r", mitkCommandLineParser::Bool, "Rescale connectivity", "Whether to rescale the connectivity matrix");
  parser.addArgument("localStatistics", "L", mitkCommandLineParser::StringList, "Local statistics", "Provide a list of node labels for local statistics", us::Any());
  parser.addArgument("regionList", "R", mitkCommandLineParser::StringList, "Region list", "A space separated list of regions. Each region has the format\n regionname;label1;label2;...;labelN", us::Any());
  parser.addArgument("granularity", "gr", mitkCommandLineParser::Int, "Granularity", "How finely to test the density range and how many thresholds to consider",1);
  parser.addArgument("startDensity", "d", mitkCommandLineParser::Float, "Start Density", "Largest density for the range",1.0);
  parser.addArgument("thresholdStepSize", "t", mitkCommandLineParser::Int, "Step size threshold", "Distance of two adjacent thresholds",3);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  //default values
  bool noGlobalStatistics( false );
  bool binaryConnectivity( false );
  bool rescaleConnectivity( false );
  bool createConnectivityMatriximage( false );
  int granularity( 1 );
  double startDensity( 1.0 );
  int thresholdStepSize( 3 );


  // parse command line arguments
  std::string networkName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outName = us::any_cast<std::string>(parsedArgs["o"]);

  mitkCommandLineParser::StringContainerType localLabels;

  if(parsedArgs.count("localStatistics"))
  {
    localLabels = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["localStatistics"]);
  }

  mitkCommandLineParser::StringContainerType unparsedRegions;
  std::map< std::string, std::vector<std::string> > parsedRegions;
  std::map< std::string, std::vector<std::string> >::iterator parsedRegionsIterator;

  if(parsedArgs.count("regionList"))
  {
    unparsedRegions = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["regionList"]);

    for(unsigned int index(0); index < unparsedRegions.size(); index++ )
    {
      std::vector< std::string > tempRegionVector;

      boost::split(tempRegionVector, unparsedRegions.at(index), boost::is_any_of(";"));

      std::vector< std::string >::const_iterator begin = tempRegionVector.begin();
      std::vector< std::string >::const_iterator last = tempRegionVector.begin() + tempRegionVector.size();
      std::vector< std::string > insertRegionVector(begin + 1, last);

      if( parsedRegions.count( tempRegionVector.at(0) ) == 0 )
      {
        parsedRegions.insert( std::pair< std::string, std::vector<std::string>  >( tempRegionVector.at(0), insertRegionVector) );
      }
      else
      {
        MITK_ERROR << "Region already exists. Skipping second occurrence.";
      }
    }
  }

  if (parsedArgs.count("noGlobalStatistics"))
    noGlobalStatistics = us::any_cast<bool>(parsedArgs["noGlobalStatistics"]);
  if (parsedArgs.count("binaryConnectivity"))
    binaryConnectivity = us::any_cast<bool>(parsedArgs["binaryConnectivity"]);
  if (parsedArgs.count("rescaleConnectivity"))
    rescaleConnectivity = us::any_cast<bool>(parsedArgs["rescaleConnectivity"]);
  if (parsedArgs.count("createConnectivityMatriximage"))
    createConnectivityMatriximage = us::any_cast<bool>(parsedArgs["createConnectivityMatriximage"]);
  if (parsedArgs.count("granularity"))
    granularity = us::any_cast<int>(parsedArgs["granularity"]);
  if (parsedArgs.count("startDensity"))
    startDensity = us::any_cast<float>(parsedArgs["startDensity"]);
  if (parsedArgs.count("thresholdStepSize"))
    thresholdStepSize = us::any_cast<int>(parsedArgs["thresholdStepSize"]);

  try
  {
    // load network
    std::vector<mitk::BaseData::Pointer> networkFile =
      mitk::IOUtil::Load( networkName);
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

    // streams
    std::stringstream globalHeaderStream;
    globalHeaderStream  << "NumberOfVertices "
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
      << "NormalizedLaplacianUpperSlope "
      << "SmallWorldness"
      << std::endl;

    std::stringstream localHeaderStream;
    std::stringstream regionalHeaderStream;

    std::stringstream globalDataStream;
    std::stringstream localDataStream;
    std::stringstream regionalDataStream;

    std::string globalOutName = outName + "_global.txt";
    std::string localOutName = outName + "_local.txt";
    std::string regionalOutName = outName + "_regional.txt";

    bool firstRun( true );
    // iterate over all three possible methods
    for(unsigned int method( 0 ); method < 3; method++)
    {
      // 0 - Random removal threshold
      // 1 - Largest density below threshold
      // 2 - Threshold based

      // iterate over possible targets
      for( int step = 0; step < granularity; ++step )
      {
        double targetValue( 0.0 );

        switch ( method )
        {
        case mitk::ConnectomicsNetworkThresholder::RandomRemovalOfWeakest :
        case mitk::ConnectomicsNetworkThresholder::LargestLowerThanDensity :
          targetValue = startDensity * (1 - static_cast<double>( step ) / ( granularity + 0.5 ) );
          break;
        case mitk::ConnectomicsNetworkThresholder::ThresholdBased :
          targetValue = static_cast<double>( thresholdStepSize * step );
          break;
        default:
          MITK_ERROR << "Invalid thresholding method called, aborting.";
          return EXIT_FAILURE;
          break;
        }

        mitk::ConnectomicsNetworkThresholder::Pointer thresholder = mitk::ConnectomicsNetworkThresholder::New();
        thresholder->SetNetwork( network );
        thresholder->SetTargetThreshold( targetValue );
        thresholder->SetTargetDensity( targetValue );
        thresholder->SetThresholdingScheme( static_cast<mitk::ConnectomicsNetworkThresholder::ThresholdingSchemes>(method) );
        mitk::ConnectomicsNetwork::Pointer thresholdedNetwork = thresholder->GetThresholdedNetwork();

        mitk::ConnectomicsStatisticsCalculator::Pointer statisticsCalculator = mitk::ConnectomicsStatisticsCalculator::New();
        statisticsCalculator->SetNetwork( thresholdedNetwork );
        statisticsCalculator->Update();

        // global statistics
        if( !noGlobalStatistics )
        {
          globalDataStream << statisticsCalculator->GetNumberOfVertices() << " "
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

          mitk::LocaleSwitch localeSwitch("C");
          itk::ImageFileWriter< connectivityMatrixImageType >::Pointer connectivityWriter = itk::ImageFileWriter< connectivityMatrixImageType >::New();

          connectivityWriter->SetInput( filter->GetOutput() );
          connectivityWriter->SetFileName( outName + connectivity_png_postfix);
          connectivityWriter->Update();

          std::cout << "Connectivity matrix image written.";
        } // end create connectivity matrix png

        /*
         * We can either calculate local indices for specific nodes, or specific regions
         */

        // Create LabelToIndex translation
        std::map< std::string, int > labelToIdMap;
        std::vector< mitk::ConnectomicsNetwork::NetworkNode > nodeVector = thresholdedNetwork->GetVectorOfAllNodes();
        for(std::size_t loop(0); loop < nodeVector.size(); loop++)
        {
          labelToIdMap.insert( std::pair< std::string, int>(nodeVector.at(loop).label, nodeVector.at(loop).id) );
        }

        std::vector< int > degreeVector = thresholdedNetwork->GetDegreeOfNodes();
        std::vector< double > ccVector = thresholdedNetwork->GetLocalClusteringCoefficients( );
        std::vector< double > bcVector = thresholdedNetwork->GetNodeBetweennessVector( );


        // calculate local indices

        {
          // only add to header for the first step of the first method
          if( firstRun )
          {
            localHeaderStream << "Th_method " << "Th_target " << "density";
          }

          double density = statisticsCalculator->GetConnectionDensity();

          localDataStream << "\n"
            << method << " "
            << targetValue << " "
            << density;

          for(unsigned int loop(0); loop < localLabels.size(); loop++ )
          {
            if( network->CheckForLabel(localLabels.at( loop )) )
            {
              if( firstRun )
              {
                localHeaderStream  << " "
                  << localLabels.at( loop ) << "_Degree "
                  << localLabels.at( loop ) << "_CC "
                  << localLabels.at( loop ) << "_BC";
              }

          localDataStream  << " " << degreeVector.at( labelToIdMap.find( localLabels.at( loop ) )->second )
            << " " << ccVector.at( labelToIdMap.find( localLabels.at( loop ) )->second )
            << " " << bcVector.at( labelToIdMap.find( localLabels.at( loop ) )->second );
            }
            else
            {
              MITK_ERROR << "Illegal label. Label: \"" << localLabels.at( loop ) << "\" not found.";
            }
          }
        }

        // calculate regional indices

        {
          // only add to header for the first step of the first method
          if( firstRun )
          {
            regionalHeaderStream << "Th_method " << "Th_target " << "density";
          }

          double density = statisticsCalculator->GetConnectionDensity();

          regionalDataStream << "\n"
            << method << " "
            << targetValue << " "
            << density;

          for( parsedRegionsIterator = parsedRegions.begin(); parsedRegionsIterator != parsedRegions.end(); parsedRegionsIterator++ )
          {
            std::vector<std::string> regionLabelsVector = parsedRegionsIterator->second;
            std::string regionName = parsedRegionsIterator->first;

            double sumDegree( 0 );
            double sumCC( 0 );
            double sumBC( 0 );
            double count( 0 );

            for( std::size_t loop(0); loop < regionLabelsVector.size(); loop++ )
            {
              if( thresholdedNetwork->CheckForLabel(regionLabelsVector.at( loop )) )
              {
                sumDegree = sumDegree + degreeVector.at( labelToIdMap.find( regionLabelsVector.at( loop ) )->second );
                sumCC = sumCC + ccVector.at( labelToIdMap.find( regionLabelsVector.at( loop ) )->second );
                sumBC = sumBC + bcVector.at( labelToIdMap.find( regionLabelsVector.at( loop ) )->second );
                count = count + 1;
              }
              else
              {
                MITK_ERROR << "Illegal label. Label: \"" << regionLabelsVector.at( loop ) << "\" not found.";
              }
            }

            // only add to header for the first step of the first method
            if( firstRun )
            {
              regionalHeaderStream  << " " << regionName << "_LocalAverageDegree "
                << regionName << "_LocalAverageCC "
                << regionName << "_LocalAverageBC "
                << regionName << "_NumberOfNodes";
            }

            regionalDataStream  << " " << sumDegree / count
              << " " << sumCC / count
              << " " << sumBC / count
              << " " << count;

            // count number of connections and fibers between regions
            std::map< std::string, std::vector<std::string> >::iterator loopRegionsIterator;
            for (loopRegionsIterator = parsedRegionsIterator; loopRegionsIterator != parsedRegions.end(); loopRegionsIterator++)
            {
              int numberConnections(0), possibleConnections(0);
              double summedFiberCount(0.0);
              std::vector<std::string> loopLabelsVector = loopRegionsIterator->second;
              std::string loopName = loopRegionsIterator->first;

              for (std::size_t loop(0); loop < regionLabelsVector.size(); loop++)
              {
                if (thresholdedNetwork->CheckForLabel(regionLabelsVector.at(loop)))
                {
                  for (std::size_t innerLoop(0); innerLoop < loopLabelsVector.size(); innerLoop++)
                  {
                    if (thresholdedNetwork->CheckForLabel(loopLabelsVector.at(loop)))
                    {
                      bool exists = thresholdedNetwork->EdgeExists(
                        labelToIdMap.find(regionLabelsVector.at(loop))->second,
                        labelToIdMap.find(loopLabelsVector.at(innerLoop))->second);
                      possibleConnections++;
                      if (exists)
                      {
                        numberConnections++;
                        summedFiberCount += thresholdedNetwork->GetEdge(
                          labelToIdMap.find(regionLabelsVector.at(loop))->second,
                          labelToIdMap.find(loopLabelsVector.at(innerLoop))->second).fiber_count;
                      }
                    }
                    else
                    {
                      MITK_ERROR << "Illegal label. Label: \"" << loopLabelsVector.at(loop) << "\" not found.";
                    }
                  }
                }
                else
                {
                  MITK_ERROR << "Illegal label. Label: \"" << regionLabelsVector.at(loop) << "\" not found.";
                }
              }

              if (firstRun)
              {
                regionalHeaderStream << " " << regionName << "_" << loopName << "_Connections "
                  << " " << regionName << "_" << loopName << "_possibleConnections "
                  << " " << regionName << "_" << loopName << "_ConnectingFibers";
              }

              regionalDataStream << " " << numberConnections
                << " " << possibleConnections
                << " " << summedFiberCount;

            }
          }
        }

        firstRun = false;
      }

    }// end calculate local averages

    if( !noGlobalStatistics )
    {
      std::cout << "Writing to " << globalOutName;
      std::ofstream glocalOutFile( globalOutName.c_str(), ios::out );
      if( ! glocalOutFile.is_open() )
      {
        std::string errorMessage = "Could not open " + globalOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }
      glocalOutFile << globalHeaderStream.str() << globalDataStream.str();
      glocalOutFile.close();
    }

    if( localLabels.size() > 0 )
    {
      std::cout << "Writing to " << localOutName;
      std::ofstream localOutFile( localOutName.c_str(), ios::out );
      if( ! localOutFile.is_open() )
      {
        std::string errorMessage = "Could not open " + localOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }
      localOutFile << localHeaderStream.str() << localDataStream.str();
      localOutFile.close();
    }

    if( parsedRegions.size() > 0 )
    {
      std::cout << "Writing to " << regionalOutName;
      std::ofstream regionalOutFile( regionalOutName.c_str(), ios::out );
      if( ! regionalOutFile.is_open() )
      {
        std::string errorMessage = "Could not open " + regionalOutName + " for writing.";
        MITK_ERROR << errorMessage;
        return EXIT_FAILURE;
      }
      regionalOutFile << regionalHeaderStream.str() << regionalDataStream.str();
      regionalOutFile.close();
    }

    return EXIT_SUCCESS;
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  std::cout << "DONE";
  return EXIT_SUCCESS;
}
