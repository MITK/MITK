
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

#include<mitkConnectomicsShortestPathHistogram.h>

#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "mitkConnectomicsConstantsManager.h"

mitk::ConnectomicsShortestPathHistogram::ConnectomicsShortestPathHistogram()
: m_Mode( UnweightedUndirectedMode )
, m_EverythingConnected( true )
{
  m_Subject = "Shortest path";
}

mitk::ConnectomicsShortestPathHistogram::~ConnectomicsShortestPathHistogram()
{
}

void mitk::ConnectomicsShortestPathHistogram::SetShortestPathCalculationMode( const mitk::ConnectomicsShortestPathHistogram::ShortestPathCalculationMode & mode)
{
  m_Mode = mode;
}

mitk::ConnectomicsShortestPathHistogram::ShortestPathCalculationMode mitk::ConnectomicsShortestPathHistogram::GetShortestPathCalculationMode()
{
  return m_Mode;
}

void mitk::ConnectomicsShortestPathHistogram::ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source )
{

  NetworkType* boostGraph = source->GetBoostGraph();

  switch( m_Mode )
  {
  case UnweightedUndirectedMode:
    {
      CalculateUnweightedUndirectedShortestPaths( boostGraph );
      break;
    }
  case WeightedUndirectedMode:
    {
      CalculateWeightedUndirectedShortestPaths( boostGraph );
      break;
    }
  }

    ConvertDistanceMapToHistogram();
}

void mitk::ConnectomicsShortestPathHistogram::CalculateUnweightedUndirectedShortestPaths( NetworkType* boostGraph )
{
  std::vector< DescriptorType > predecessorMap( boost::num_vertices( *boostGraph ) );
  int numberOfNodes( boost::num_vertices( *boostGraph ) );

  m_DistanceMatrix.resize( numberOfNodes );
  for( int index(0); index < m_DistanceMatrix.size(); index++ )
  {
    m_DistanceMatrix[ index ].resize( numberOfNodes );
  }

  IteratorType iterator, end;
  boost::tie(iterator, end) = boost::vertices( *boostGraph );

  for ( int index(0) ; iterator != end; ++iterator, index++)
  {
          boost::dijkstra_shortest_paths(*boostGraph, *iterator, boost::predecessor_map(&predecessorMap[ 0 ]).distance_map(&m_DistanceMatrix[ index ][ 0 ]).weight_map( boost::get( &mitk::ConnectomicsNetwork::NetworkEdge::edge_weight ,*boostGraph ) ) ) ;
  }
}

void mitk::ConnectomicsShortestPathHistogram::CalculateWeightedUndirectedShortestPaths( NetworkType* boostGraph )
{
  MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_UNIMPLEMENTED_FEATURE;
}

void mitk::ConnectomicsShortestPathHistogram::ConvertDistanceMapToHistogram()
{
  // get the longest path between any two nodes in the network
  // we assume that no nodes are farther apart than there are nodes,
  // this is to filter unconnected nodes
  int longestPath( 0 );
  int numberOfNodes( m_DistanceMatrix.size() );
  m_EverythingConnected = true;

  for( int index(0); index < m_DistanceMatrix.size(); index++ )
  {
    for( int innerIndex(0); innerIndex < m_DistanceMatrix[ index ].size(); innerIndex++ )
    {
      if( m_DistanceMatrix[ index ][ innerIndex ] > longestPath )
      {
        if( m_DistanceMatrix[ index ][ innerIndex ] < numberOfNodes )
        {
          longestPath = m_DistanceMatrix[ index ][ innerIndex ];
        }
        else
        {
          // these nodes are not connected
          m_EverythingConnected = false;
        }
      }
    }
  }

  m_HistogramVector.resize( longestPath + 1 );

  for( int index(0); index < m_DistanceMatrix.size(); index++ )
  {
    for( int innerIndex(0); innerIndex < m_DistanceMatrix[ index ].size(); innerIndex++ )
    {
      if( m_DistanceMatrix[ index ][ innerIndex ] < numberOfNodes )
      {
        m_HistogramVector[ m_DistanceMatrix[ index ][ innerIndex ] ]++;
      }
    }
  }

  // correct for every path being counted twice

  for( int index(1); index < m_HistogramVector.size(); index++ )
  {
    m_HistogramVector[ index ] = m_HistogramVector[ index ] / 2;
  }

    // correct for every node being distance zero to itself
  if( m_HistogramVector[ 0 ] >= numberOfNodes )
  {
    m_HistogramVector[ 0 ] = m_HistogramVector[ 0 ] - numberOfNodes;
  }
  else
  {
    MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ZERO_DISTANCE_NODES;
  }

  UpdateYMax();

  this->m_Valid = true;
}

double mitk::ConnectomicsShortestPathHistogram::GetEfficiency()
{
  if( !this->m_Valid )
  {
    MBI_INFO << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_CAN_NOT_COMPUTE_EFFICIENCY << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NETWORK_NOT_VALID;
    return 0.0;
  }

  if( !m_EverythingConnected )
  { // efficiency of disconnected graphs is 0
        MBI_INFO << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NETWORK_DISCONNECTED;
    return 0.0;
  }

  double efficiency( 0.0 );

  double overallDistance( 0.0 );
  double numberOfPairs( 0.0 );
  // add up all distances
  for( int index(0); index < m_HistogramVector.size(); index++ )
  {
    overallDistance = overallDistance + m_HistogramVector[ index ] * index;
    numberOfPairs = numberOfPairs + m_HistogramVector[ index ];
  }

  // efficiency = 1 / averageDistance = 1 / ( overallDistance / numberofPairs )
  efficiency = numberOfPairs / overallDistance;

  return efficiency;
}
