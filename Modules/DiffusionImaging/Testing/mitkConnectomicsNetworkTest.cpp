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

#include <mitkDiffusionImagingObjectFactory.h>
#include "mitkConnectomicsNetwork.h"
#include "mitkConnectomicsSyntheticNetworkGenerator.h"
#include <vector>
#include <string>
#include <utility>

/**Documentation
*  Test for synthetic connectomics generation and connectomics network functionality
*/
int mitkConnectomicsNetworkTest(int argc, char* argv[])
{
  // Test begins
  MITK_TEST_BEGIN("mitkConnectomicsNetworkTest");

  try
  {
    mitk::ConnectomicsSyntheticNetworkGenerator::Pointer generator = mitk::ConnectomicsSyntheticNetworkGenerator::New();
    MITK_TEST_CONDITION_REQUIRED(generator.IsNotNull(),"Synthetic network generator has been instantiated")

    // first and second parameter
    std::vector< std::pair<int, double> >  parameterVector;
    // name and isRandom
    std::vector< std::pair<std::string, bool> > messageVector;
    // order of results is # vertices, # edges, # self loops, average degree, connection density
    // global clustering coefficient
    std::vector< std::vector< double > > resultVector;

    int numberOfOptions = 3;

    parameterVector.resize(numberOfOptions);
    messageVector.resize(numberOfOptions);
    resultVector.resize(numberOfOptions);

    // Create regular lattice network in the form of a cube with sidelength 5 and distance 10 between nodes
    parameterVector[0] = std::pair< int, double>(5, 10);
    messageVector[0] = std::pair<std::string, bool>("Regular Lattice Network", false);
    resultVector[0].push_back( 5 * 5 * 5 );
    resultVector[0].push_back( 300 );
    resultVector[0].push_back( 0 );
    resultVector[0].push_back( 4.8 );
    resultVector[0].push_back(0.0387);
    resultVector[0].push_back(0);

    // Create a heterogeneous sphere network with 1 central node and 49 nodes on the surface and radius 10
    parameterVector[1] = std::pair< int, double>(50, 10);
    messageVector[1] = std::pair<std::string, bool>("Heterogeneous Sphere Network", false);
    resultVector[1].push_back( 50 );
    resultVector[1].push_back( 49 );
    resultVector[1].push_back( 0 );
    resultVector[1].push_back( 1.96 );
    resultVector[1].push_back(0.0400);
    resultVector[1].push_back(0);

    // Create random network with 50 nodes and edges between them generated with a 0.1 likelihood
    parameterVector[2] = std::pair< int, double>(50, 0.1);
    messageVector[2] = std::pair<std::string, bool>("Random Network", true);
    resultVector[2].push_back( 50 );

    for(int loop(0); loop < numberOfOptions; loop++)
    {
      mitk::ConnectomicsNetwork::Pointer network =
        generator->CreateSyntheticNetwork(loop, parameterVector[loop].first, parameterVector[loop].second);
      bool generationWorked = generator->WasGenerationSuccessfull() && network.IsNotNull();

      std::string message = messageVector[loop].first + " has been instantiated";
      MITK_TEST_CONDITION_REQUIRED(generationWorked,message)

      double eps(0.001);

      bool verticesCloseEnough( std::abs(resultVector[loop][0] - network->GetNumberOfVertices()) < eps);
      MITK_TEST_CONDITION_REQUIRED( verticesCloseEnough, "Expected number of vertices")

      if(!messageVector[loop].second)
      {
        bool edgesCloseEnough( std::abs(resultVector[loop][1] - network->GetNumberOfEdges()) < eps);
        MITK_TEST_CONDITION_REQUIRED( edgesCloseEnough, "Expected number of edges")

        bool selfLoopsCloseEnough( std::abs(resultVector[loop][2] - network->GetNumberOfSelfLoops()) < eps);
        MITK_TEST_CONDITION_REQUIRED( selfLoopsCloseEnough, "Expected number of self loops")

        bool avDegCloseEnough( std::abs(resultVector[loop][3] - network->GetAverageDegree()) < eps);
        MITK_TEST_CONDITION_REQUIRED( avDegCloseEnough, "Expected average degree")

        bool conDensCloseEnough( std::abs(resultVector[loop][4] - network->GetConnectionDensity()) < eps);
        MITK_TEST_CONDITION_REQUIRED( conDensCloseEnough, "Expected connection density")

        bool clustCoeffCloseEnough( std::abs(resultVector[loop][5] - network->GetGlobalClusteringCoefficient()) < eps);
        MITK_TEST_CONDITION_REQUIRED( clustCoeffCloseEnough, "Expected global clustering coefficient")
      }
    }
  }
  catch (...)
  {
    MITK_ERROR << "Unhandled exception caught [FAILED]" ;
    return EXIT_FAILURE;
  }
  // Test ends
  MITK_TEST_END();
}
