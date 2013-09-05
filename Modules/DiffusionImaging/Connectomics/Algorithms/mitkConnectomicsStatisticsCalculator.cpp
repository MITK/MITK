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
#include <boost/graph/visitors.hpp>

template<typename GraphType>
class all_pairs_shortest_recorder : public boost::default_bfs_visitor
{
public:
  typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexType;
  typedef typename boost::graph_traits<GraphType>::edge_descriptor EdgeType;

  all_pairs_shortest_recorder(int* dist, int &max, unsigned int &size)
  {
    d = dist;
    ecc = &max;
    component_size = &size;
  }

  void tree_edge(EdgeType edge, const GraphType& graph) {
    VertexType u = boost::source(edge, graph);
    VertexType v = boost::target(edge, graph);
    d[v] = d[u] + 1;
    *ecc = d[v];
    *component_size = *component_size + 1;
  }
private:
  int* d;
  int* ecc;
  unsigned int* component_size;
};

mitk::ConnectomicsStatisticsCalculator::ConnectomicsStatisticsCalculator()
  : m_Network( 0 )
  , m_NumberOfVertices( 0 )
  , m_NumberOfEdges( 0 )
  , m_AverageDegree( 0.0 )
  , m_ConnectionDensity( 0.0 )
  , m_NumberOfConnectedComponents( 0 )
  , m_AverageComponentSize( 0.0 )
  , m_Components(0)
  , m_LargestComponentSize( 0 )
  , m_HopPlotExponent( 0.0 )
  , m_EffectiveHopDiameter( 0.0 )
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
  CalculateLargestComponentSize();
  CalculateRatioOfNodesInLargestComponent();
  CalculateHopPlotValues();
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
  m_Components.resize( m_NumberOfVertices );
  m_NumberOfConnectedComponents = boost::connected_components( *(m_Network->GetBoostGraph()), &m_Components[0] );
}

void mitk::ConnectomicsStatisticsCalculator::CalculateAverageComponentSize()
{
  m_AverageComponentSize = (double) m_NumberOfConnectedComponents / (double) m_NumberOfVertices ;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateLargestComponentSize()
{
  m_LargestComponentSize = 0;
  std::vector<int> bins( m_NumberOfConnectedComponents );

  for(unsigned int i=0; i < m_NumberOfVertices; i++)
  {
    bins[ m_Components[i] ]++;
  }

  for(unsigned int i=0; i < m_NumberOfConnectedComponents; i++)
  {
    if (bins[i] > m_LargestComponentSize )
    {
      m_LargestComponentSize = bins[i];
    }
  }
}

void mitk::ConnectomicsStatisticsCalculator::CalculateRatioOfNodesInLargestComponent()
{
  m_RatioOfNodesInLargestComponent = (double) m_LargestComponentSize / (double) m_NumberOfVertices ;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateHopPlotValues()
{
  std::vector<int> bins( m_NumberOfVertices );

  VertexIteratorType vi, vi_end;
  unsigned int index( 0 );

  for( boost::tie( vi, vi_end ) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi != vi_end; ++vi)
  {
    std::vector<int> distances(m_NumberOfVertices, 0);
    int max_distance = 0;
    VertexDescriptorType src = *vi;
    distances[src] = 0;
    unsigned int size = 0;

    boost::breadth_first_search(*(m_Network->GetBoostGraph()), src,
      visitor(all_pairs_shortest_recorder< NetworkType >
      (&distances[0], max_distance, size)));

    for(index=0; index < distances.size(); index++)
    {
      if(distances[index] > 0)
      {
        bins[distances[index]]++;
      }
    }
  }

  bins[0] = m_NumberOfVertices;
  for(index=1; index < bins.size(); index++)
  {
    bins[index] = bins[index] + bins[index-1];
  }

  int counter = 0;
  double C=0, D=0, E=0, F=0;

  for (unsigned int i=1; i<bins.size()-1; i++)
  {
    counter ++;
    if(fabs(log(double(bins[i+1])) - log(double(bins[i]))) < 0.0000001)
    {
      break;
    }
  }

  for (int i=1; i<=counter; i++)
  {
    double x = log(double(i));
    double y = log(double(bins[i]));
    C += x;
    D += y;
    E += x * y;
    F += x * x;
  }
  double b = (D*F - C*E)/(F*counter - C*C);
  m_HopPlotExponent = (E - b*C)/F;

  m_EffectiveHopDiameter =
    std::pow( ( m_NumberOfVertices * m_NumberOfVertices )
    / ( m_NumberOfVertices + 2 * m_NumberOfEdges ), 1.0 / m_HopPlotExponent );
}
