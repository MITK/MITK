
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

#include<mitkConnectomicsBetweennessHistogram.h>

#include <boost/numeric/conversion/converter.hpp>

#include <mitkConnectomicsConstantsManager.h>

mitk::ConnectomicsBetweennessHistogram::ConnectomicsBetweennessHistogram()
: m_Mode( UnweightedUndirectedMode )
, m_CentralityMap()
{
  m_Subject = "Node Betweenness";
}

mitk::ConnectomicsBetweennessHistogram::~ConnectomicsBetweennessHistogram()
{
}

void mitk::ConnectomicsBetweennessHistogram::SetBetweennessCalculationMode(
  const mitk::ConnectomicsBetweennessHistogram::BetweennessCalculationMode & mode )
{
  m_Mode = mode;
}

mitk::ConnectomicsBetweennessHistogram::BetweennessCalculationMode mitk::ConnectomicsBetweennessHistogram::GetBetweennessCalculationMode()
{
  return m_Mode;
}

void mitk::ConnectomicsBetweennessHistogram::ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source )
{
  NetworkType* boostGraph = source->GetBoostGraph();
  IteratorType vertex_iterator_begin, vertex_iterator_end;

  m_CentralityMap.clear();
  m_CentralityMap.resize( source->GetNumberOfVertices() );

  switch( m_Mode )
  {
  case UnweightedUndirectedMode:
    {
      CalculateUnweightedUndirectedBetweennessCentrality( boostGraph, vertex_iterator_begin, vertex_iterator_end );
      break;
    }
  case WeightedUndirectedMode:
    {
      CalculateWeightedUndirectedBetweennessCentrality( boostGraph, vertex_iterator_begin, vertex_iterator_end );
      break;
    }
  }

  ConvertCentralityMapToHistogram();
}

void mitk::ConnectomicsBetweennessHistogram::CalculateUnweightedUndirectedBetweennessCentrality(
  NetworkType* boostGraph, IteratorType vertex_iterator_begin, IteratorType vertex_iterator_end )
{
  boost::brandes_betweenness_centrality(
    *boostGraph,
    boost::centrality_map(
    boost::make_iterator_property_map( m_CentralityMap.begin(), boost::get( &mitk::ConnectomicsNetwork::NetworkNode::id, *boostGraph ), double() )
    ).vertex_index_map( boost::get( &mitk::ConnectomicsNetwork::NetworkNode::id, *boostGraph ) )
    );

}

void mitk::ConnectomicsBetweennessHistogram::CalculateWeightedUndirectedBetweennessCentrality(
  NetworkType* boostGraph, IteratorType vertex_iterator_begin, IteratorType vertex_iterator_end )
{
  MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_UNIMPLEMENTED_FEATURE;
}

void mitk::ConnectomicsBetweennessHistogram::ConvertCentralityMapToHistogram()
{
  double maximumFloat( 0.0 );

  for ( int index( 0 ); index < m_CentralityMap.size(); index++ )
  {
    if( m_CentralityMap[ index ] > maximumFloat )
    {
      maximumFloat = m_CentralityMap[ index ];
    }
   }

  // use the boost double to int converter
  // it defaults to trunc
  typedef boost::numeric::converter<int,double> Double2Int ;

  // for rounding, reduces the number of nodes classed as zero
  maximumFloat += 0.5;

  int maximumInt( 0 );
  try
  {
    maximumInt = Double2Int::convert( maximumFloat );
  }
  catch ( boost::numeric::positive_overflow const& )
  {
    MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_OUTSIDE_INTEGER_RANGE;
  }

  m_HistogramVector.resize( maximumInt + 1 );

  for ( int index( 0 ); index < m_CentralityMap.size(); index++ )
  {
    int value( 0 );
    value = Double2Int::convert( ( m_CentralityMap[index ] + 0.5 ) );
    m_HistogramVector[ value ]++;
  }

  UpdateYMax();

  m_Valid = true;
}
