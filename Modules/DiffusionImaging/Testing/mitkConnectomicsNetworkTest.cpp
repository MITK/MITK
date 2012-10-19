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

  double eps(0.001);

  try
  {
    // Testing synthetic network generation
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
    MITK_ERROR << "Unhandled exception caught while testing synthetic network generation [FAILED]" ;
    return EXIT_FAILURE;
  }

  try
  {
    // Testing network interface

    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexType;
    typedef mitk::ConnectomicsNetwork::EdgeDescriptorType EdgeType;
    typedef mitk::ConnectomicsNetwork::NetworkNode NodeType;

    // The test network

    std::vector< NodeType > inNodes;

    NodeType node;

    node.id = 0;
    node.label = "1-1";
    node.coordinates.clear();
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 0 );
    inNodes.push_back(node);

    node.id = 1;
    node.label = "2-1";
    node.coordinates.clear();
    node.coordinates.push_back( 10 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 0 );
    inNodes.push_back(node);

    node.id = 2;
    node.label = "3-1";
    node.coordinates.clear();
    node.coordinates.push_back( 20 );
    node.coordinates.push_back( 10 );
    node.coordinates.push_back( 0 );
    inNodes.push_back(node);

    node.id = 3;
    node.label = "4-1";
    node.coordinates.clear();
    node.coordinates.push_back( 30 );
    node.coordinates.push_back( 20 );
    node.coordinates.push_back( 0 );
    inNodes.push_back(node);

    node.id = 4;
    node.label = "5-1";
    node.coordinates.clear();
    node.coordinates.push_back( 40 );
    node.coordinates.push_back( 50 );
    node.coordinates.push_back( 0 );
    inNodes.push_back(node);

    node.id = 5;
    node.label = "6-2";
    node.coordinates.clear();
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 10 );
    inNodes.push_back(node);

    node.id = 6;
    node.label = "7-2";
    node.coordinates.clear();
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 10 );
    node.coordinates.push_back( 20 );
    inNodes.push_back(node);

    node.id = 7;
    node.label = "8-2";
    node.coordinates.clear();
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 20 );
    node.coordinates.push_back( 30 );
    inNodes.push_back(node);

    node.id = 8;
    node.label = "9-2";
    node.coordinates.clear();
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 30 );
    node.coordinates.push_back( 40 );
    inNodes.push_back(node);

    node.id = 9;
    node.label = "10-3";
    node.coordinates.clear();
    node.coordinates.push_back( 20 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 10 );
    inNodes.push_back(node);

    node.id = 10;
    node.label = "11-3";
    node.coordinates.clear();
    node.coordinates.push_back( 30 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 20 );
    inNodes.push_back(node);

    node.id = 11;
    node.label = "12-3";
    node.coordinates.clear();
    node.coordinates.push_back( 40 );
    node.coordinates.push_back( 0 );
    node.coordinates.push_back( 30 );
    inNodes.push_back(node);

    std::vector< mitk::ConnectomicsNetwork::NetworkEdge > inEdges;

    mitk::ConnectomicsNetwork::NetworkEdge edge;

    edge.sourceId = 0;
    edge.targetId = 1;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 0;
    edge.targetId = 2;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 0;
    edge.targetId = 4;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 1;
    edge.targetId = 4;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 1;
    edge.targetId = 3;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 2;
    edge.targetId = 3;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 3;
    edge.targetId = 4;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 4;
    edge.targetId = 5;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 5;
    edge.targetId = 6;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 5;
    edge.targetId = 7;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 5;
    edge.targetId = 8;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 7;
    edge.targetId = 8;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 6;
    edge.targetId = 8;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 6;
    edge.targetId = 9;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 3;
    edge.targetId = 10;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 9;
    edge.targetId = 10;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 10;
    edge.targetId = 11;
    edge.weight = 2;
    inEdges.push_back( edge );

    edge.sourceId = 9;
    edge.targetId = 11;
    edge.weight = 2;
    inEdges.push_back( edge );

    // Create network

    mitk::ConnectomicsNetwork::Pointer network = mitk::ConnectomicsNetwork::New();

    std::vector< VertexType > vertexVector;
    vertexVector.resize( inNodes.size() );

    for(int loop(0); loop < inNodes.size(); loop++)
    {
      VertexType newVertex = network->AddVertex( inNodes[loop].id );
      vertexVector[ inNodes[loop].id ] = newVertex;
      network->SetLabel( newVertex, inNodes[loop].label );
      network->SetCoordinates( newVertex, inNodes[loop].coordinates );
    }

    for(int loop(0); loop < inEdges.size(); loop++)
    {
      int sourceId = inEdges[loop].sourceId;
      int targetId = inEdges[loop].targetId;
      VertexType sourceVertex = vertexVector[ sourceId ];
      VertexType targetVertex = vertexVector[ targetId ];

      // there are two methods to add nodes
      if( loop % 2 == 0 )
      {
        network->AddEdge(sourceVertex, targetVertex);
        for( int remaining( inEdges[loop].weight ); remaining > 1; remaining-- )
        {
          network->IncreaseEdgeWeight( sourceVertex, targetVertex );
        }
      }
      else
      {
        network->AddEdge(sourceVertex, targetVertex, sourceId, targetId, inEdges[loop].weight );
      }
    }

    // Test whether network parameters are as expected
    MITK_TEST_CONDITION_REQUIRED( inNodes.size() == network->GetNumberOfVertices(), "Expected number of vertices")

    MITK_TEST_CONDITION_REQUIRED( inEdges.size() == network->GetNumberOfEdges(), "Expected number of edges")

    MITK_TEST_CONDITION_REQUIRED( 0 == network->GetNumberOfSelfLoops(), "Expected number of self loops")

    bool avDegCloseEnough( std::abs(3 - network->GetAverageDegree()) < eps);
    MITK_TEST_CONDITION_REQUIRED( avDegCloseEnough, "Expected average degree")

    bool conDensCloseEnough( std::abs(0.2727 - network->GetConnectionDensity()) < eps);
    MITK_TEST_CONDITION_REQUIRED( conDensCloseEnough, "Expected connection density")

    bool clustCoeffCloseEnough( std::abs(0.4583 - network->GetGlobalClusteringCoefficient()) < eps);
    MITK_TEST_CONDITION_REQUIRED( clustCoeffCloseEnough, "Expected global clustering coefficient")

    MITK_TEST_CONDITION_REQUIRED( network->GetMaximumWeight() == 2, "Expected maximum weight")

    MITK_TEST_CONDITION_REQUIRED( network->GetVectorOfAllVertexDescriptors().size() == vertexVector.size(), "Expected number of vertex descriptors")



  }
  catch (...)
  {
    MITK_ERROR << "Unhandled exception caught while testing network interface [FAILED]" ;
    return EXIT_FAILURE;
  }
  // Test ends
  MITK_TEST_END();
}
