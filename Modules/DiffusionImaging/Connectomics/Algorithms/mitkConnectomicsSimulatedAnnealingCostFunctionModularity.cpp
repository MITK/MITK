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

#include "mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h"

mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::ConnectomicsSimulatedAnnealingCostFunctionModularity()
{
}

mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::~ConnectomicsSimulatedAnnealingCostFunctionModularity()
{
}

double mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::Evaluate( mitk::ConnectomicsNetwork::Pointer network, ToModuleMapType* vertexToModuleMap ) const
{
  double cost( 0.0 );
  cost = 100.0 * ( 1.0 - CalculateModularity( network, vertexToModuleMap ) );
  return cost;
}

double mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::CalculateModularity( mitk::ConnectomicsNetwork::Pointer network, ToModuleMapType* vertexToModuleMap ) const
{
  double modularity( 0.0 );
  int numberOfModules = getNumberOfModules( vertexToModuleMap );

  if( network->GetNumberOfVertices() != vertexToModuleMap->size() )
  {
    MBI_ERROR << "Number of vertices and vertex to module map size do not match!";
    return modularity;
  }

  int numberOfLinksInNetwork( 0 );
  std::vector< int > numberOfLinksInModule, sumOfDegreesInModule;
  numberOfLinksInModule.resize( numberOfModules, 0 );
  sumOfDegreesInModule.resize( numberOfModules, 0 );
  // get vector of all vertex descriptors in the network

  const std::vector< VertexDescriptorType > allNodesVector
    = network->GetVectorOfAllVertexDescriptors();

  for( int nodeNumber( 0 ); nodeNumber < allNodesVector.size() ; nodeNumber++)
  {
    int correspondingModule = vertexToModuleMap->find( allNodesVector[ nodeNumber ] )->second;
    const std::vector< VertexDescriptorType > adjacentNodexVector
      = network->GetVectorOfAdjacentNodes( allNodesVector[ nodeNumber ] );
    numberOfLinksInNetwork += adjacentNodexVector.size();
    sumOfDegreesInModule[ correspondingModule ] += adjacentNodexVector.size();

    for( int adjacentNodeNumber( 0 ); adjacentNodeNumber < adjacentNodexVector.size() ; adjacentNodeNumber++)
    {
      if( correspondingModule == vertexToModuleMap->find( adjacentNodexVector[ adjacentNodeNumber ] )->second )
      {
        numberOfLinksInModule[ correspondingModule ]++;
      }
    }
  }

  // the numbers for links have to be halved, as each edge was counted twice
  numberOfLinksInNetwork = numberOfLinksInNetwork / 2;

  // if the network contains no links return 0
  if( numberOfLinksInNetwork < 1)
  {
    return 0;
  }

  for( int index( 0 ); index < numberOfModules ; index++)
  {
    numberOfLinksInModule[ index ] = numberOfLinksInModule[ index ] / 2;
  }

  //Calculate modularity M:
  //M = sum_{s=1}^{N_{M}} [ (l_{s} / L) - (d_{s} / ( 2L ))^2 ]
  //where N_{M} is the number of modules
  //L is the number of links in the network
  //l_{s} is the number of links between nodes in the module
  //s is the module
  //d_{s} is the sum of degrees of the nodes in the module
  //( taken from Guimera, R. AND Amaral, L. A. N.
  // Cartography of complex networks: modules and universal roles
  // Journal of Statistical Mechanics: Theory and Experiment, 2005, 2005, P02001 )

  for( int moduleID( 0 ); moduleID < numberOfModules; moduleID++ )
  {
    modularity += (((double) numberOfLinksInModule[ moduleID ]) / ((double) numberOfLinksInNetwork)) -
      (
      (((double) sumOfDegreesInModule[ moduleID ]) / ((double) 2 * numberOfLinksInNetwork) ) *
      (((double) sumOfDegreesInModule[ moduleID ]) / ((double) 2 * numberOfLinksInNetwork) )
      );
  }

  return modularity;
}

int mitk::ConnectomicsSimulatedAnnealingCostFunctionModularity::getNumberOfModules(
  ToModuleMapType *vertexToModuleMap ) const
{
  int maxModule( 0 );
  ToModuleMapType::iterator iter = vertexToModuleMap->begin();
  ToModuleMapType::iterator end =  vertexToModuleMap->end();
  while( iter != end )
  {
    if( iter->second > maxModule )
    {
      maxModule = iter->second;
    }

    iter++;
  }

  return maxModule + 1;
}
