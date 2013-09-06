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

#include <numeric>

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
  , m_VectorOfClusteringCoefficientsC( 0 )
  , m_VectorOfClusteringCoefficientsD( 0 )
  , m_VectorOfClusteringCoefficientsE( 0 )
  , m_AverageClusteringCoefficientsC( 0.0 )
  , m_AverageClusteringCoefficientsD( 0.0 )
  , m_AverageClusteringCoefficientsE( 0.0 )
  , m_VectorOfVertexBetweennessCentralities( 0 )
  , m_PropertyMapOfVertexBetweennessCentralities( )
  , m_AverageVertexBetweennessCentrality( 0.0 )
  , m_VectorOfEdgeBetweennessCentralities( 0 )
  , m_PropertyMapOfEdgeBetweennessCentralities( )
  , m_AverageEdgeBetweennessCentrality( 0.0 )
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
  CalculateClusteringCoefficients();
  CalculateBetweennessCentrality();
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

void mitk::ConnectomicsStatisticsCalculator::CalculateClusteringCoefficients()
{
  VertexIteratorType vi, vi_end;
  std::vector<double> m_VectorOfClusteringCoefficientsC;
  std::vector<double> m_VectorOfClusteringCoefficientsD;
  std::vector<double> m_VectorOfClusteringCoefficientsE;
  typedef std::set<VertexDescriptorType> NeighborSetType;
  typedef NetworkType::out_edge_iterator OutEdgeIterType;

  for(boost::tie(vi,vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi!=vi_end; ++vi)
  {
    // Get the list of vertices which are in the neighborhood of vi.
    std::pair<AdjacencyIteratorType, AdjacencyIteratorType> adjacent =
      boost::adjacent_vertices(*vi, *(m_Network->GetBoostGraph()) );

    //Populate a set with the neighbors of vi
    NeighborSetType neighbors;
    for(; adjacent.first!=adjacent.second; ++adjacent.first)
    {
      neighbors.insert(*adjacent.first);
    }

    // Now, count the edges between vertices in the neighborhood.
    unsigned int neighborhood_edge_count = 0;
    if(neighbors.size() > 0)
    {
      NeighborSetType::iterator iter;
      for(iter = neighbors.begin(); iter != neighbors.end(); ++iter)
      {
        std::pair<OutEdgeIterType, OutEdgeIterType> oe = out_edges(*iter, *(m_Network->GetBoostGraph()) );
        for(; oe.first != oe.second; ++oe.first)
        {
          if(neighbors.find(target(*oe.first, *(m_Network->GetBoostGraph()) )) != neighbors.end())
          {
            ++neighborhood_edge_count;
          }
        }
      }
      neighborhood_edge_count /= 2;
    }
    //Clustering Coefficienct C,E
    if(neighbors.size() > 1)
    {
      double num   = neighborhood_edge_count;
      double denum = neighbors.size() * (neighbors.size()-1)/2;
      m_VectorOfClusteringCoefficientsC.push_back( num / denum);
      m_VectorOfClusteringCoefficientsE.push_back( num / denum);
    }
    else
    {
      m_VectorOfClusteringCoefficientsC.push_back(0.0);
    }

    //Clustering Coefficienct D
    if(neighbors.size() > 0)
    {
      double num   = neighbors.size() + neighborhood_edge_count;
      double denum = ( (neighbors.size()+1) * neighbors.size()) / 2;
      m_VectorOfClusteringCoefficientsD.push_back( num / denum);
    }
    else
    {
      m_VectorOfClusteringCoefficientsD.push_back(0.0);
    }
  }

  // Average Clustering coefficienies:
  m_AverageClusteringCoefficientsC = std::accumulate(m_VectorOfClusteringCoefficientsC.begin(),
    m_VectorOfClusteringCoefficientsC.end(),
    0.0) / m_NumberOfVertices;

  m_AverageClusteringCoefficientsD = std::accumulate(m_VectorOfClusteringCoefficientsD.begin(),
    m_VectorOfClusteringCoefficientsD.end(),
    0.0) / m_NumberOfVertices;

  m_AverageClusteringCoefficientsE = std::accumulate(m_VectorOfClusteringCoefficientsE.begin(),
    m_VectorOfClusteringCoefficientsE.end(),
    0.0) / m_VectorOfClusteringCoefficientsE.size();
}

void mitk::ConnectomicsStatisticsCalculator::CalculateBetweennessCentrality()
{

  // std::map used for convenient initialization
  EdgeIndexStdMapType stdEdgeIndex;
  // associative property map needed for iterator property map-wrapper
  EdgeIndexMapType edgeIndex(stdEdgeIndex);

  boost::graph_traits<NetworkType>::edge_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::edges( *(m_Network->GetBoostGraph()) );

  int i(0);
  for ( ; iterator != end; ++iterator, ++i)
  {
    stdEdgeIndex.insert(std::pair< EdgeDescriptorType, int >( *iterator, i));
  }

  // Define EdgeCentralityMap
  m_VectorOfEdgeBetweennessCentralities.resize( m_NumberOfEdges, 0.0);
  // Create the external property map
  m_PropertyMapOfEdgeBetweennessCentralities = EdgeIteratorPropertyMapType(m_VectorOfEdgeBetweennessCentralities.begin(), edgeIndex);

  // Define VertexCentralityMap
  VertexIndexMapType vertexIndex = get(boost::vertex_index, *(m_Network->GetBoostGraph()) );
  m_VectorOfVertexBetweennessCentralities.resize( m_NumberOfVertices, 0.0);
    // Create the external property map
  m_PropertyMapOfVertexBetweennessCentralities = VertexIteratorPropertyMapType(m_VectorOfVertexBetweennessCentralities.begin(), vertexIndex);

  boost::brandes_betweenness_centrality( *(m_Network->GetBoostGraph()),
    m_PropertyMapOfVertexBetweennessCentralities, m_PropertyMapOfEdgeBetweennessCentralities );

  m_AverageVertexBetweennessCentrality = std::accumulate(m_VectorOfVertexBetweennessCentralities.begin(),
    m_VectorOfVertexBetweennessCentralities.end(),
    0.0) / (double) m_NumberOfVertices;

  m_AverageEdgeBetweennessCentrality = std::accumulate(m_VectorOfEdgeBetweennessCentralities.begin(),
    m_VectorOfEdgeBetweennessCentralities.end(),
    0.0) / (double) m_NumberOfEdges;
}
