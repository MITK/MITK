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
#include "mitkConnectomicsNetworkConverter.h"

#include <numeric>

#include <boost/graph/clustering_coefficient.hpp>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4172)
#endif
#include <boost/graph/connected_components.hpp>
#include <boost/graph/betweenness_centrality.hpp>

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#include <boost/graph/visitors.hpp>

#include "vnl/algo/vnl_symmetric_eigensystem.h"

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
  : m_Network( nullptr )
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
  , m_NumberOfIsolatedPoints( 0 )
  , m_RatioOfIsolatedPoints( 0.0 )
  , m_NumberOfEndPoints( 0 )
  , m_RatioOfEndPoints( 0.0 )
  , m_VectorOfEccentrities( 0 )
  , m_VectorOfEccentrities90( 0 )
  , m_VectorOfAveragePathLengths( 0.0 )
  , m_Diameter( 0 )
  , m_Diameter90( 0 )
  , m_Radius( 0 )
  , m_Radius90( 0 )
  , m_AverageEccentricity( 0.0 )
  , m_AverageEccentricity90( 0.0 )
  , m_AveragePathLength( 0.0 )
  , m_NumberOfCentralPoints( 0 )
  , m_RatioOfCentralPoints( 0.0 )
  , m_VectorOfSortedEigenValues( 0 )
  , m_SpectralRadius( 0.0 )
  , m_SecondLargestEigenValue( 0.0 )
  , m_AdjacencyTrace( 0.0 )
  , m_AdjacencyEnergy( 0.0 )
  , m_VectorOfSortedLaplacianEigenValues( 0 )
  , m_LaplacianTrace( 0.0 )
  , m_LaplacianEnergy( 0.0 )
  , m_LaplacianSpectralGap( 0.0 )
  , m_VectorOfSortedNormalizedLaplacianEigenValues( 0 )
  , m_NormalizedLaplacianTrace( 0.0 )
  , m_NormalizedLaplacianEnergy( 0.0 )
  , m_NormalizedLaplacianNumberOf2s( 0 )
  , m_NormalizedLaplacianNumberOf1s( 0 )
  , m_NormalizedLaplacianNumberOf0s( 0 )
  , m_NormalizedLaplacianLowerSlope( 0.0 )
  , m_NormalizedLaplacianUpperSlope( 0.0 )
  , m_SmallWorldness( 0.0 )
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
  CalculateIsolatedAndEndPoints();
  CalculateShortestPathMetrics();
  CalculateSpectralMetrics();
  CalculateLaplacianMetrics();
  CalculateNormalizedLaplacianMetrics();
  CalculateSmallWorldness();
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
  double numberOfPossibleEdges = (double) m_NumberOfVertices * ( (double) m_NumberOfVertices - 1 ) / 2;

  m_ConnectionDensity = (double) m_NumberOfEdges / numberOfPossibleEdges;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateNumberOfConnectedComponents()
{
  m_Components.resize( m_NumberOfVertices );
  m_NumberOfConnectedComponents = boost::connected_components( *(m_Network->GetBoostGraph()), &m_Components[0] );
}

void mitk::ConnectomicsStatisticsCalculator::CalculateAverageComponentSize()
{
  m_AverageComponentSize = (double) m_NumberOfVertices / (double) m_NumberOfConnectedComponents ;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateLargestComponentSize()
{
  m_LargestComponentSize = 0;
  std::vector<unsigned int> bins( m_NumberOfConnectedComponents );

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

  EdgeIteratorType iterator, end;

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

void mitk::ConnectomicsStatisticsCalculator::CalculateIsolatedAndEndPoints()
{
  m_NumberOfIsolatedPoints = 0;
  m_NumberOfEndPoints = 0;

  VertexIteratorType vi, vi_end;
  for( boost::tie(vi,vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi!=vi_end; ++vi)
  {
    int degree = boost::out_degree(*vi, *(m_Network->GetBoostGraph()) );
    if(degree == 0)
    {
      m_NumberOfIsolatedPoints++;
    }
    else if (degree == 1)
    {
      m_NumberOfEndPoints++;
    }
  }

  m_RatioOfEndPoints = (double) m_NumberOfEndPoints / (double) m_NumberOfVertices;
  m_RatioOfIsolatedPoints = (double) m_NumberOfIsolatedPoints / (double) m_NumberOfVertices;
}


/**
* Calculates Shortest Path Related metrics of the graph.  The
* function runs a BFS from each node to find out the shortest
* distances to other nodes in the graph. The maximum of this distance
* is called the eccentricity of that node. The maximum eccentricity
* in the graph is called diameter and the minimum eccentricity is
* called the radius of the graph.  Central points are those nodes
* having eccentricity equals to radius.
*/
void mitk::ConnectomicsStatisticsCalculator::CalculateShortestPathMetrics()
{
  //for all vertices:
  VertexIteratorType vi, vi_end;

  //store the eccentricities in a vector.
  m_VectorOfEccentrities.resize( m_NumberOfVertices );
  m_VectorOfEccentrities90.resize( m_NumberOfVertices );
  m_VectorOfAveragePathLengths.resize( m_NumberOfVertices );

  //assign diameter and radius while iterating over the ecccencirities.
  m_Diameter              = 0;
  m_Diameter90            = 0;
  m_Radius                = std::numeric_limits<unsigned int>::max();
  m_Radius90              = std::numeric_limits<unsigned int>::max();
  m_AverageEccentricity   = 0.0;
  m_AverageEccentricity90 = 0.0;
  m_AveragePathLength     = 0.0;

  //The size of the giant connected component so far.
  unsigned int giant_component_size = 0;
  VertexDescriptorType radius_src(0);

  //Loop over the vertices
  for( boost::tie(vi, vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi!=vi_end; ++vi)
  {
    //We are going to start a BFS, initialize the neccessary
    //structures for that.  Store the distances of nodes from the
    //source in distance vector. The maximum distance is stored in
    //max. The BFS will start from the node that vi is pointing, that
    //is the src is *vi. We also init the distance of the src node to
    //itself to 0. size gives the number of nodes discovered during
    //this BFS.
    std::vector<int> distances( m_NumberOfVertices );
    int max_distance = 0;
    VertexDescriptorType src = *vi;
    distances[src] = 0;
    unsigned int size = 0;

    breadth_first_search(*(m_Network->GetBoostGraph()), src,
      visitor(all_pairs_shortest_recorder<NetworkType>
      (&distances[0], max_distance, size)));
    // vertex vi has eccentricity equal to max_distance
    m_VectorOfEccentrities[src] = max_distance;

    //check whether there is any change in the diameter or the radius.
    //note that the diameter we are calculating here is also the
    //diameter of the giant connected component!
    if(m_VectorOfEccentrities[src] > m_Diameter)
    {
      m_Diameter = m_VectorOfEccentrities[src];
    }

    //The radius should be calculated on the largest connected
    //component, otherwise it is very likely that radius will be 1.
    //We change the value of the radius only if this is the giant
    //connected component so far. After all the eccentricities are
    //found we should loop over this connected component and find the
    //minimum eccentricity which is the radius. So we keep the src
    //node, so that we can find the connected component later on.
    if(size > giant_component_size)
    {
      giant_component_size = size;
      radius_src = src;
    }

    //Calculate in how many hops we can reach 90 percent of the
    //nodes. We store the number of hops we can reach in h hops in the
    //bucket vector. That is bucket[h] gives the number of nodes
    //reachable in exactly h hops. sum of bucket[i<h] gives the number
    //of nodes that are reachable in less than h hops. We also
    //calculate sum of the distances from this node to every single
    //other node in the graph.
    int reachable90 = std::ceil((double)size * 0.9);
    std::vector <int> bucket (max_distance+1);
    int counter = 0;
    for(unsigned int i=0; i<distances.size(); i++)
    {
      if(distances[i]>0)
      {
        bucket[distances[i]]++;
        m_VectorOfAveragePathLengths[src] += distances[i];
        counter ++;
      }
    }
    if(counter > 0)
    {
      m_VectorOfAveragePathLengths[src] = m_VectorOfAveragePathLengths[src] / counter;
    }

    int eccentricity90 = 0;
    while(reachable90 > 0)
    {
      eccentricity90 ++;
      reachable90 = reachable90 - bucket[eccentricity90];
    }
    // vertex vi has eccentricity90 equal to eccentricity90
    m_VectorOfEccentrities90[src] = eccentricity90;
    if(m_VectorOfEccentrities90[src] > m_Diameter90)
    {
      m_Diameter90 = m_VectorOfEccentrities90[src];
    }
  }

  //We are going to calculate the radius now. We stored the src node
  //that when we start a BFS gives the giant connected component, and
  //we have the eccentricities calculated. Iterate over the nodes of
  //this giant component and find the minimum eccentricity.
  std::vector<int> component( m_NumberOfVertices );
  boost::connected_components( *(m_Network->GetBoostGraph()), &component[0]);
  for (unsigned int i=0; i<component.size(); i++)
  {
    //If we are in the same component and the radius is not the
    //minimum so far store the eccentricity as the radius.
    if( component[i] == component[radius_src])
    {
      if(m_Radius > m_VectorOfEccentrities[i])
      {
        m_Radius = m_VectorOfEccentrities[i];
      }
      if(m_Radius90 > m_VectorOfEccentrities90[i])
      {
        m_Radius90 = m_VectorOfEccentrities90[i];
      }
    }
  }

  m_AverageEccentricity = std::accumulate(m_VectorOfEccentrities.begin(),
    m_VectorOfEccentrities.end(), 0.0) / m_NumberOfVertices;

  m_AverageEccentricity90 = std::accumulate(m_VectorOfEccentrities90.begin(),
    m_VectorOfEccentrities90.end(), 0.0) / m_NumberOfVertices;

  m_AveragePathLength = std::accumulate(m_VectorOfAveragePathLengths.begin(),
    m_VectorOfAveragePathLengths.end(), 0.0) / m_NumberOfVertices;

  //calculate Number of Central Points, nodes having eccentricity = radius.
  m_NumberOfCentralPoints = 0;
  for (boost::tie(vi, vi_end) = boost::vertices( *(m_Network->GetBoostGraph()) ); vi != vi_end; ++vi)
  {
    if(m_VectorOfEccentrities[*vi] == m_Radius)
    {
      m_NumberOfCentralPoints++;
    }
  }
  m_RatioOfCentralPoints = (double)m_NumberOfCentralPoints / m_NumberOfVertices;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateSpectralMetrics()
{
  mitk::ConnectomicsNetworkConverter::Pointer converter = mitk::ConnectomicsNetworkConverter::New();
  converter->SetNetwork( m_Network );
  vnl_matrix<double> adjacencyMatrix = converter->GetNetworkAsVNLAdjacencyMatrix();

  vnl_symmetric_eigensystem<double> eigenSystem(adjacencyMatrix);

  m_AdjacencyTrace = 0;
  m_AdjacencyEnergy = 0;
  m_VectorOfSortedEigenValues.clear();

  for(unsigned int i=0; i < m_NumberOfVertices; ++i)
  {
    double value = std::fabs(eigenSystem.get_eigenvalue(i));
    m_VectorOfSortedEigenValues.push_back(value);
    m_AdjacencyTrace += value;
    m_AdjacencyEnergy += value * value;
  }

  std::sort(m_VectorOfSortedEigenValues.begin(), m_VectorOfSortedEigenValues.end());

  m_SpectralRadius = m_VectorOfSortedEigenValues[ m_NumberOfVertices - 1];
  m_SecondLargestEigenValue  = m_VectorOfSortedEigenValues[ m_NumberOfVertices - 2];
}

void mitk::ConnectomicsStatisticsCalculator::CalculateLaplacianMetrics()
{
  mitk::ConnectomicsNetworkConverter::Pointer converter = mitk::ConnectomicsNetworkConverter::New();
  converter->SetNetwork( m_Network );
  vnl_matrix<double> adjacencyMatrix = converter->GetNetworkAsVNLAdjacencyMatrix();
  vnl_matrix<double> laplacianMatrix ( m_NumberOfVertices, m_NumberOfVertices, 0);
  vnl_matrix<double> degreeMatrix = converter->GetNetworkAsVNLDegreeMatrix();

  m_VectorOfSortedLaplacianEigenValues.clear();
  laplacianMatrix = degreeMatrix - adjacencyMatrix;
  int numberOfConnectedComponents = 0;
  vnl_symmetric_eigensystem <double> laplacianEigenSystem( laplacianMatrix );
  m_LaplacianEnergy = 0;
  m_LaplacianTrace  = 0;
  for(unsigned int i(0); i < m_NumberOfVertices; ++i)
  {
    double value = std::fabs( laplacianEigenSystem.get_eigenvalue(i) );
    m_VectorOfSortedLaplacianEigenValues.push_back( value );
    m_LaplacianTrace += value;
    m_LaplacianEnergy += value * value;
    if ( std::fabs( value ) < mitk::eps )
    {
      numberOfConnectedComponents++;
    }
  }

  std::sort(m_VectorOfSortedLaplacianEigenValues.begin(), m_VectorOfSortedLaplacianEigenValues.end());
  for(unsigned int i(0); i < m_VectorOfSortedLaplacianEigenValues.size(); ++i)
  {
    if(m_VectorOfSortedLaplacianEigenValues[i] > mitk::eps )
    {
      m_LaplacianSpectralGap = m_VectorOfSortedLaplacianEigenValues[i];
      break;
    }
  }
}

void  mitk::ConnectomicsStatisticsCalculator::CalculateNormalizedLaplacianMetrics()
{
  vnl_matrix<double> normalizedLaplacianMatrix(m_NumberOfVertices, m_NumberOfVertices, 0);
  EdgeIteratorType ei, ei_end;
  VertexDescriptorType sourceVertex, destinationVertex;
  int sourceIndex, destinationIndex;
  VertexIndexMapType vertexIndexMap = boost::get(boost::vertex_index, *(m_Network->GetBoostGraph()) );
  m_VectorOfSortedNormalizedLaplacianEigenValues.clear();

  // Normalized laplacian matrix
  for( boost::tie(ei, ei_end) = boost::edges( *(m_Network->GetBoostGraph()) ); ei != ei_end; ++ei)
  {
    sourceVertex = boost::source(*ei, *(m_Network->GetBoostGraph()) );
    sourceIndex = vertexIndexMap[sourceVertex];

    destinationVertex = boost::target(*ei, *(m_Network->GetBoostGraph()) );
    destinationIndex = vertexIndexMap[destinationVertex];
    int sourceDegree = boost::out_degree(sourceVertex, *(m_Network->GetBoostGraph()) );
    int destinationDegree = boost::out_degree(destinationVertex, *(m_Network->GetBoostGraph()) );

    normalizedLaplacianMatrix.put(
      sourceIndex, destinationIndex, -1 / (sqrt(double(sourceDegree * destinationDegree))));
    normalizedLaplacianMatrix.put(
      destinationIndex, sourceIndex, -1 / (sqrt(double(sourceDegree * destinationDegree))));
  }

  VertexIteratorType vi, vi_end;
  for(boost::tie(vi, vi_end)=boost::vertices( *(m_Network->GetBoostGraph()) ); vi!=vi_end; ++vi)
  {
    if(boost::out_degree(*vi, *(m_Network->GetBoostGraph()) ) > 0)
    {
      normalizedLaplacianMatrix.put(vertexIndexMap[*vi], vertexIndexMap[*vi], 1);
    }
  }
  //End of normalized laplacian matrix definition

  vnl_symmetric_eigensystem <double>
    normalizedLaplacianEigensystem(normalizedLaplacianMatrix);

  double N1=0, C1=0, D1=0, E1=0, F1=0, b1=0;
  double N2=0, C2=0, D2=0, E2=0, F2=0, b2=0;
  m_NormalizedLaplacianNumberOf2s = 0;
  m_NormalizedLaplacianNumberOf1s = 0;
  m_NormalizedLaplacianNumberOf0s = 0;
  m_NormalizedLaplacianTrace = 0;
  m_NormalizedLaplacianEnergy = 0;

  for(unsigned int i(0); i< m_NumberOfVertices; ++i)
  {
    double eigenValue = std::fabs(normalizedLaplacianEigensystem.get_eigenvalue(i));
    m_VectorOfSortedNormalizedLaplacianEigenValues.push_back(eigenValue);
    m_NormalizedLaplacianTrace  += eigenValue;
    m_NormalizedLaplacianEnergy += eigenValue * eigenValue;

    //0
    if(eigenValue < mitk::eps)
    {
      m_NormalizedLaplacianNumberOf0s++;
    }

    //Between 0 and 1.
    else if(eigenValue > mitk::eps && eigenValue< 1 - mitk::eps)
    {
      C1 += i;
      D1 += eigenValue;
      E1 += i * eigenValue;
      F1 += i * i;
      N1 ++;
    }

    //1
    else if(std::fabs( std::fabs(eigenValue) - 1) < mitk::eps)
    {
      m_NormalizedLaplacianNumberOf1s++;
    }

    //Between 1 and 2
    else if(std::fabs(eigenValue) > 1+mitk::eps && std::fabs(eigenValue)< 2 - mitk::eps)
    {
      C2 += i;
      D2 += eigenValue;
      E2 += i * eigenValue;
      F2 += i * i;
      N2 ++;
    }

    //2
    else if(std::fabs( std::fabs(eigenValue) - 2) < mitk::eps)
    {
      m_NormalizedLaplacianNumberOf2s++;
    }
  }

  b1 = (D1*F1 - C1*E1)/(F1*N1 - C1*C1);
  m_NormalizedLaplacianLowerSlope = (E1 - b1*C1)/F1;

  b2 = (D2*F2 - C2*E2)/(F2*N2 - C2*C2);
  m_NormalizedLaplacianUpperSlope = (E2 - b2*C2)/F2;
}

void mitk::ConnectomicsStatisticsCalculator::CalculateSmallWorldness()
{
  double k( this->GetAverageDegree() );
  double N( this->GetNumberOfVertices() );
  // The clustering coefficient of an Erdos-Reny network is equivalent to
  // the likelihood two random nodes are connected
  double gamma = this->GetAverageClusteringCoefficientsC() / ( k / N );
  //The mean path length of an Erdos-Reny network is approximately
  // ln( #vertices ) / ln( average degree )
  double lambda = this->GetAveragePathLength() / ( std::log( N ) / std::log( k ) );

  m_SmallWorldness = gamma / lambda;
}
