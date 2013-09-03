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

#include "mitkConnectomicsStatisticsCalculator.h"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/clustering_coefficient.hpp>
#include <boost/graph/betweenness_centrality.hpp>

mitk::ConnectomicsStatisticsCalculator::ConnectomicsStatisticsCalculator()
  : m_Network( 0 )
  , m_NumberOfVertices( 0 )
  , m_NumberOfEdges( 0 )
  , m_AverageDegree( 0.0 )
  , m_ConnectionDensity( 0.0 )
  , m_NumberOfConnectedComponents( 0 )
  , m_AverageComponentSize( 0.0 )
{
}

mitk::ConnectomicsStatisticsCalculator::~ConnectomicsStatisticsCalculator()
{
}

void mitk::ConnectomicsStatisticsCalculator::Update()
{
  CalculateNumberOfVertices();
  CalculateNumberOfEdges();
  CalculateAverageDegree();
  CalculateConnectionDensity();
  CalculateNumberOfConnectedComponents();
  CalculateAverageComponentSize();
}

void mitk::ConnectomicsStatisticsCalculator::CalculateNumberOfVertices()
{
  m_NumberOfVertices = boost::num_vertices( *(m_Network->GetBoostGraph()) );
}

void mitk::ConnectomicsStatisticsCalculator::CalculateNumberOfEdges()
{
  m_NumberOfEdges = boost::num_edges(  *(m_Network->GetBoostGraph()) );
}

void  mitk::ConnectomicsStatisticsCalculator::CalculateAverageDegree()
{
  m_AverageDegree = ( ( (double) m_NumberOfEdges * 2.0 ) / (double) m_NumberOfVertices );
}

void mitk::ConnectomicsStatisticsCalculator::CalculateConnectionDensity()
{
  double numberOfPossibleEdges = (double) m_NumberOfVertices * ( (double) m_NumberOfVertices - 1 ) / 2 ;

  m_ConnectionDensity = (double) m_NumberOfEdges / numberOfPossibleEdges;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateNumberOfConnectedComponents()
{
  std::vector<int> component( m_NumberOfVertices ); // TODO make member variable
  m_NumberOfConnectedComponents = boost::connected_components( *(m_Network->GetBoostGraph()), &component[0] );
}

void mitk::ConnectomicsStatisticsCalculator::CalculateAverageComponentSize()
{
  m_AverageComponentSize = (double) m_NumberOfConnectedComponents / (double) m_NumberOfVertices ;
}
