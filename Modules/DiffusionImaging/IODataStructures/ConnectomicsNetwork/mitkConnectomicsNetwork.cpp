
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

#include "mitkConnectomicsNetwork.h"
#include <boost/graph/clustering_coefficient.hpp>
#include <boost/graph/betweenness_centrality.hpp>

/* Constructor and Destructor */
mitk::ConnectomicsNetwork::ConnectomicsNetwork()
: m_IsModified( false )
{
}

mitk::ConnectomicsNetwork::~ConnectomicsNetwork()
{
}

/* Wrapper methods */

bool mitk::ConnectomicsNetwork::EdgeExists(
  mitk::ConnectomicsNetwork::VertexDescriptorType vertexA, mitk::ConnectomicsNetwork::VertexDescriptorType vertexB ) const
{
  return boost::edge(vertexA, vertexB, m_Network ).second;
}

void mitk::ConnectomicsNetwork::IncreaseEdgeWeight(
  mitk::ConnectomicsNetwork::VertexDescriptorType vertexA, mitk::ConnectomicsNetwork::VertexDescriptorType vertexB )
{
  m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ].weight++;

  SetIsModified( true );
}

void mitk::ConnectomicsNetwork::AddEdge(
                                        mitk::ConnectomicsNetwork::VertexDescriptorType vertexA,
                                        mitk::ConnectomicsNetwork::VertexDescriptorType vertexB
                                        )
{
  AddEdge(vertexA, vertexB, m_Network[ vertexA ].id, m_Network[ vertexB ].id );
}

void mitk::ConnectomicsNetwork::AddEdge(
                                        mitk::ConnectomicsNetwork::VertexDescriptorType vertexA,
                                        mitk::ConnectomicsNetwork::VertexDescriptorType vertexB,
                                        int sourceID, int targetID, int weight )
{
  boost::add_edge( vertexA, vertexB, m_Network );
  m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ].sourceId = sourceID;
  m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ].targetId = targetID;
  m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ].weight = weight;
  m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ].edge_weight = 1.0;

  SetIsModified( true );
}

mitk::ConnectomicsNetwork::VertexDescriptorType mitk::ConnectomicsNetwork::AddVertex( int id )
{
  VertexDescriptorType vertex = boost::add_vertex( m_Network );
  m_Network[vertex].id = id;

  SetIsModified( true );

  return vertex;
}

void mitk::ConnectomicsNetwork::SetLabel(
  mitk::ConnectomicsNetwork::VertexDescriptorType vertex, std::string inLabel )
{
  m_Network[vertex].label = inLabel;

  SetIsModified( true );
}

void mitk::ConnectomicsNetwork::SetCoordinates(
  mitk::ConnectomicsNetwork::VertexDescriptorType vertex, std::vector< float > inCoordinates )
{
  m_Network[vertex].coordinates = inCoordinates;

  SetIsModified( true );
}

void mitk::ConnectomicsNetwork::clear()
{
  m_Network.clear();

  SetIsModified( true );
}

/* Superclass methods, that need to be implemented */
void mitk::ConnectomicsNetwork::UpdateOutputInformation()
{

}
void mitk::ConnectomicsNetwork::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::ConnectomicsNetwork::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool mitk::ConnectomicsNetwork::VerifyRequestedRegion()
{
  return true;
}
void mitk::ConnectomicsNetwork::SetRequestedRegion( itk::DataObject *data )
{

}



std::vector< mitk::ConnectomicsNetwork::NetworkNode >
mitk::ConnectomicsNetwork::GetVectorOfAllNodes() const
{
  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::vertices( m_Network );

  std::vector< NetworkNode > vectorOfNodes;

  for ( ; iterator != end; ++iterator)
  {
    NetworkNode tempNode;

    // the value of an iterator is a descriptor
    tempNode = m_Network[ *iterator ];

    vectorOfNodes.push_back( tempNode );
  }

  return vectorOfNodes;
}

std::vector< mitk::ConnectomicsNetwork::VertexDescriptorType >
mitk::ConnectomicsNetwork::GetVectorOfAllVertexDescriptors() const
{
  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::vertices( m_Network );

  std::vector< VertexDescriptorType > vectorOfDescriptors;

  for ( ; iterator != end; ++iterator)
  {
    vectorOfDescriptors.push_back( *iterator );
  }

  return vectorOfDescriptors;
}

std::vector< std::pair<
std::pair< mitk::ConnectomicsNetwork::NetworkNode, mitk::ConnectomicsNetwork::NetworkNode >
, mitk::ConnectomicsNetwork::NetworkEdge > >
mitk::ConnectomicsNetwork::GetVectorOfAllEdges() const
{
  boost::graph_traits<NetworkType>::edge_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::edges( m_Network );

  std::vector<
    std::pair<
    std::pair< NetworkNode, NetworkNode >
    , NetworkEdge
    >
  > vectorOfEdges;

  for ( ; iterator != end; ++iterator)
  {
    NetworkNode sourceNode, targetNode;
    NetworkEdge tempEdge;

    // the value of an iterator is a descriptor
    tempEdge = m_Network[ *iterator ];
    sourceNode = m_Network[ boost::source( *iterator, m_Network ) ];
    targetNode = m_Network[ boost::target( *iterator, m_Network ) ];

    std::pair< NetworkNode, NetworkNode > nodePair( sourceNode, targetNode );
    std::pair< std::pair< NetworkNode, NetworkNode > , NetworkEdge > edgePair( nodePair, tempEdge);
    vectorOfEdges.push_back( edgePair );
  }

  return vectorOfEdges;
}

int mitk::ConnectomicsNetwork::GetNumberOfVertices() const
{
  return boost::num_vertices( m_Network );
}

int mitk::ConnectomicsNetwork::GetNumberOfEdges() const
{
  return boost::num_edges( m_Network );
}

int mitk::ConnectomicsNetwork::GetMaximumWeight() const
{
  int maxWeight( 0 );

  boost::graph_traits<NetworkType>::edge_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::edges( m_Network );

  for ( ; iterator != end; ++iterator)
  {
    int tempWeight;

    // the value of an iterator is a descriptor
    tempWeight = m_Network[ *iterator ].weight;

    if( tempWeight > maxWeight )
    {
      maxWeight = tempWeight;
    }
  }

  return maxWeight;
}

int mitk::ConnectomicsNetwork::GetNumberOfSelfLoops()
{
  int noOfSelfLoops( 0 );

  std::vector< std::pair< std::pair< NetworkNode, NetworkNode > , NetworkEdge > >
    edgeVector =  GetVectorOfAllEdges();

  for( int index = 0; index < edgeVector.size() ; index++ )
  {
    double sourceX, sourceY, sourceZ, targetX, targetY, targetZ;

    sourceX = edgeVector[ index ].first.first.coordinates[0] ;
    sourceY = edgeVector[ index ].first.first.coordinates[1] ;
    sourceZ = edgeVector[ index ].first.first.coordinates[2] ;
    targetX = edgeVector[ index ].first.second.coordinates[0] ;
    targetY = edgeVector[ index ].first.second.coordinates[1] ;
    targetZ = edgeVector[ index ].first.second.coordinates[2] ;

    // if the coordinates are the same
    if(
      sourceX > ( targetX - 0.01 ) &&
      sourceX < ( targetX + 0.01 ) &&
      sourceY > ( targetY - 0.01 ) &&
      sourceY < ( targetY + 0.01 ) &&
      sourceZ > ( targetZ - 0.01 ) &&
      sourceZ < ( targetZ + 0.01 )
      )
    {
      noOfSelfLoops++;
    }
  }

  return noOfSelfLoops;
}

double mitk::ConnectomicsNetwork::GetAverageDegree()
{
  double vertices = (double) GetNumberOfVertices();
  double edges = (double) GetNumberOfEdges();

  return ( ( edges * 2.0 ) / vertices );
}

double mitk::ConnectomicsNetwork::GetConnectionDensity()
{
  double vertices = (double) GetNumberOfVertices();
  double edges = (double) GetNumberOfEdges();
  double numberOfPossibleEdges = vertices * ( vertices - 1 ) / 2 ;

  return ( edges / numberOfPossibleEdges );
}

std::vector< int > mitk::ConnectomicsNetwork::GetDegreeOfNodes( ) const
{
  std::vector< int > vectorOfDegree;

  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie( iterator, end ) = boost::vertices( m_Network );

  vectorOfDegree.resize( this->GetNumberOfVertices() );

  for ( ; iterator != end; ++iterator)
  {
    // the value of an iterator is a descriptor
    vectorOfDegree[ m_Network[ *iterator ].id ] = GetVectorOfAdjacentNodes( *iterator ).size();
  }
  return vectorOfDegree;
}

std::vector< mitk::ConnectomicsNetwork::VertexDescriptorType >
mitk::ConnectomicsNetwork::GetVectorOfAdjacentNodes( mitk::ConnectomicsNetwork::VertexDescriptorType vertex ) const
{
  std::vector< mitk::ConnectomicsNetwork::VertexDescriptorType > vectorOfAdjacentNodes;

  boost::graph_traits<NetworkType>::adjacency_iterator adjIter, adjEnd;

  boost::tie( adjIter, adjEnd ) = boost::adjacent_vertices( vertex, m_Network);

  for ( ; adjIter != adjEnd; ++adjIter)
  {
    vectorOfAdjacentNodes.push_back( *adjIter );
  }

  return vectorOfAdjacentNodes;
}

int mitk::ConnectomicsNetwork::GetMaximumDegree() const
{
  int maximumDegree( 0 );

  std::vector< int > vectorOfDegree = GetDegreeOfNodes();

  for( int index( 0 ); index < vectorOfDegree.size(); ++index )
  {
    if( maximumDegree < vectorOfDegree[ index ] )
    {
      maximumDegree = vectorOfDegree[ index ];
    }
  }

  return maximumDegree;
}

std::vector< double > mitk::ConnectomicsNetwork::GetLocalClusteringCoefficients( ) const
{
  std::vector< double > vectorOfClusteringCoefficients;

  typedef boost::graph_traits<NetworkType>::vertex_iterator vertexIter;

  vectorOfClusteringCoefficients.resize( this->GetNumberOfVertices() );

  std::pair<vertexIter, vertexIter> vertexPair;

  //for every vertex calculate the clustering coefficient
  for (vertexPair = vertices(m_Network); vertexPair.first != vertexPair.second; ++vertexPair.first)
  {
    vectorOfClusteringCoefficients[ m_Network[ *vertexPair.first ].id ] =
      boost::clustering_coefficient(m_Network,*vertexPair.first) ;
  }

  return vectorOfClusteringCoefficients;
}

std::vector< double > mitk::ConnectomicsNetwork::GetClusteringCoefficientsByDegree( )
{
  std::vector< double > vectorOfClusteringCoefficients = GetLocalClusteringCoefficients();
  std::vector< int > vectorOfDegree = GetDegreeOfNodes();

  std::vector< double > vectorOfClusteringCoefficientsByDegree;
  vectorOfClusteringCoefficientsByDegree.resize( GetMaximumDegree() + 1, 0 );

  // c_{mean}(k) = frac{1}_{N_{k}} sum_{i in Y(k)} c_{i}
  // where N_{k} is the number of vertices of degree k
  // Y(k) is the set of vertices of degree k
  // c_{i} is the local clustering coefficient of vertex i
  for( int degree( 0 ); degree < vectorOfClusteringCoefficientsByDegree.size(); ++degree )
  {
    vectorOfClusteringCoefficientsByDegree[ degree ] = 0;
    int n_k( 0 );
    for( int index( 0 ); index < vectorOfDegree.size(); ++index )
    {
      if( degree == vectorOfDegree[ index ] )
      {// if in Y( degree )
        vectorOfClusteringCoefficientsByDegree[ degree ] += vectorOfClusteringCoefficients[ index ];
        n_k++;
      }
    }
    if( n_k != 0 )
    {
      vectorOfClusteringCoefficientsByDegree[ degree ] =
        vectorOfClusteringCoefficientsByDegree[ degree ] / n_k;
    }
  }

  return vectorOfClusteringCoefficientsByDegree;
}

double mitk::ConnectomicsNetwork::GetGlobalClusteringCoefficient( )
{
  double globalClusteringCoefficient( 0.0 );

  std::vector< double > vectorOfClusteringCoefficientsByDegree = GetClusteringCoefficientsByDegree();
  std::vector< int > vectorOfDegree = GetDegreeOfNodes();
  std::vector< int > degreeDistribution;
  degreeDistribution.resize( vectorOfClusteringCoefficientsByDegree.size(), 0 );

  int normalizationParameter( 0 );

  for( int index( 0 ); index < vectorOfDegree.size(); ++index )
  {
    degreeDistribution[ vectorOfDegree[ index ] ]++;
    normalizationParameter++;
  }
  // c_{mean} = sum_{k} P_{k} c_{mean}(k)
  // where P_{k} is the degree distribution
  // k is the degree
  for( int degree( 0 ); degree < degreeDistribution.size(); ++degree )
  {
    globalClusteringCoefficient +=
      degreeDistribution[ degree ] / ( (double) normalizationParameter)
      * vectorOfClusteringCoefficientsByDegree[ degree ];
  }

  return globalClusteringCoefficient;
}

mitk::ConnectomicsNetwork::NetworkType* mitk::ConnectomicsNetwork::GetBoostGraph()
{
  return &m_Network;
}


bool mitk::ConnectomicsNetwork::GetIsModified() const
{
  return m_IsModified;
}


void mitk::ConnectomicsNetwork::SetIsModified( bool value)
{
  m_IsModified = value;
}


mitk::ConnectomicsNetwork::NetworkNode mitk::ConnectomicsNetwork::GetNode( VertexDescriptorType vertex ) const
{
  return m_Network[ vertex ];
}


mitk::ConnectomicsNetwork::NetworkEdge mitk::ConnectomicsNetwork::GetEdge( VertexDescriptorType vertexA, VertexDescriptorType vertexB ) const
{
  return m_Network[ boost::edge(vertexA, vertexB, m_Network ).first ];
}

void mitk::ConnectomicsNetwork::UpdateBounds( )
{
  float min = itk::NumericTraits<float>::min();
  float max = itk::NumericTraits<float>::max();
  float bounds[] = {max, min, max, min, max, min};

  std::vector< mitk::ConnectomicsNetwork::NetworkNode > nodeVector = this->GetVectorOfAllNodes();

  if( nodeVector.size() == 0 )
  {
    bounds[0] = 0;
    bounds[1] = 1;
    bounds[2] = 0;
    bounds[3] = 1;
    bounds[4] = 0;
    bounds[5] = 1;
  }

  // for each direction, make certain the point is in between
  for( int index(0), end(nodeVector.size()) ; index < end; index++ )
  {
    for( int direction(0); direction < nodeVector.at( index ).coordinates.size(); direction++ )
    {
      if( nodeVector.at( index ).coordinates.at(direction) < bounds[ 2 * direction ]  )
      {
        bounds[ 2 * direction ] = nodeVector.at( index ).coordinates.at(direction);
      }

      if( nodeVector.at( index ).coordinates.at(direction) > bounds[ 2 * direction + 1]  )
      {
        bounds[ 2 * direction + 1] = nodeVector.at( index ).coordinates.at(direction);
      }
    }
  }


  // provide some border margin
  for(int i=0; i<=4; i+=2)
  {
    bounds[i] -=10;
  }

  for(int i=1; i<=5; i+=2)
  {
    bounds[i] +=10;
  }

  this->GetGeometry()->SetFloatBounds(bounds);
  this->GetTimeSlicedGeometry()->UpdateInformation();
}

void mitk::ConnectomicsNetwork::PruneUnconnectedSingleNodes()
{
  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;

  // set to true if iterators are invalidated by deleting a vertex
  bool vertexHasBeenRemoved( true );

  // if no vertex has been removed in the last loop, we are done
  while( vertexHasBeenRemoved )
  {
    vertexHasBeenRemoved = false;
    // sets iterator to start and end to end
    boost::tie(iterator, end) = boost::vertices( m_Network );

    for ( ; iterator != end && !vertexHasBeenRemoved; ++iterator)
    {
      // If the node has no adjacent vertices it should be deleted
      if( GetVectorOfAdjacentNodes( *iterator ).size() == 0 )
      {
        vertexHasBeenRemoved = true;
        // this invalidates all iterators
        boost::remove_vertex( *iterator, m_Network );
      }
    }
  }

  UpdateIDs();
}

void mitk::ConnectomicsNetwork::UpdateIDs()
{
  boost::graph_traits<NetworkType>::vertex_iterator v_i, v_end;
  boost::graph_traits<NetworkType>::edge_iterator e_i, e_end;

  // update node ids
  boost::tie( v_i, v_end ) = boost::vertices( m_Network );

  for ( ; v_i != v_end; ++v_i)
  {
    m_Network[*v_i].id = *v_i;
  }

  // update edge information
  boost::tie(e_i, e_end) = boost::edges( m_Network );

  for ( ; e_i != e_end; ++e_i)
  {
    m_Network[ *e_i ].sourceId = m_Network[ boost::source( *e_i, m_Network ) ].id;
    m_Network[ *e_i ].targetId = m_Network[ boost::target( *e_i, m_Network ) ].id;
  }
  this->SetIsModified( true );
}

void mitk::ConnectomicsNetwork::PruneEdgesBelowWeight( int targetWeight )
{
  boost::graph_traits<NetworkType>::edge_iterator iterator, end;

  // set to true if iterators are invalidated by deleting a vertex
  bool edgeHasBeenRemoved( true );

  // if no vertex has been removed in the last loop, we are done
  while( edgeHasBeenRemoved )
  {
    edgeHasBeenRemoved = false;
    // sets iterator to start and end to end
    boost::tie(iterator, end) = boost::edges( m_Network );

    for ( ; iterator != end && !edgeHasBeenRemoved; ++iterator)
    {
      // If the node has no adjacent edges it should be deleted
      if( m_Network[ *iterator ].weight < targetWeight )
      {
        edgeHasBeenRemoved = true;
        // this invalidates all iterators
        boost::remove_edge( *iterator, m_Network );
      }
    }
  }

  // this will remove any nodes which, after deleting edges are now
  // unconnected, also this calls UpdateIDs()
  PruneUnconnectedSingleNodes();
}

std::vector< double > mitk::ConnectomicsNetwork::GetNodeBetweennessVector() const
{
  std::vector< double > betweennessVector;

  betweennessVector.clear();
  betweennessVector.resize( this->GetNumberOfVertices() );

  boost::brandes_betweenness_centrality(
    m_Network,
    boost::centrality_map(
    boost::make_iterator_property_map( betweennessVector.begin(), boost::get( &NetworkNode::id, m_Network ), double() )
    ).vertex_index_map( boost::get( &NetworkNode::id, m_Network ) )
    );

  return betweennessVector;
}

std::vector< double > mitk::ConnectomicsNetwork::GetEdgeBetweennessVector() const
{
  // std::map used for convenient initialization
  typedef std::map<EdgeDescriptorType, int> EdgeIndexStdMap;
  EdgeIndexStdMap stdEdgeIndex;
  // associative property map needed for iterator property map-wrapper
  typedef boost::associative_property_map< EdgeIndexStdMap > EdgeIndexMap;
  EdgeIndexMap edgeIndex(stdEdgeIndex);

  boost::graph_traits<NetworkType>::edge_iterator iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::edges( m_Network );

  int i(0);
  for ( ; iterator != end; ++iterator, ++i)
  {
    stdEdgeIndex.insert(std::pair< EdgeDescriptorType, int >( *iterator, i));
  }

  // Define EdgeCentralityMap
  std::vector< double > edgeBetweennessVector(boost::num_edges( m_Network ), 0.0);
  // Create the external property map
  boost::iterator_property_map< std::vector< double >::iterator, EdgeIndexMap >
    e_centrality_map(edgeBetweennessVector.begin(), edgeIndex);

  // Define VertexCentralityMap
  typedef boost::property_map< NetworkType, boost::vertex_index_t>::type VertexIndexMap;
  VertexIndexMap vertexIndex = get(boost::vertex_index, m_Network );
  std::vector< double > betweennessVector(boost::num_vertices( m_Network ), 0.0);
  // Create the external property map
  boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMap >
    v_centrality_map(betweennessVector.begin(), vertexIndex);

  boost::brandes_betweenness_centrality( m_Network, v_centrality_map, e_centrality_map );

  return edgeBetweennessVector;
}

std::vector< double > mitk::ConnectomicsNetwork::GetShortestDistanceVectorFromLabel( std::string targetLabel ) const
{
  std::vector< VertexDescriptorType > predecessorMap( boost::num_vertices( m_Network ) );
  int numberOfNodes( boost::num_vertices( m_Network ) );

  std::vector< double > distanceMatrix;
  distanceMatrix.resize( numberOfNodes );

  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;
  boost::tie(iterator, end) = boost::vertices( m_Network );

  while( (iterator != end) && (m_Network[ *iterator ].label != targetLabel) )
  {
    ++iterator;
  }

  if( iterator == end )
  {
    MITK_WARN << "Label not found";
    return distanceMatrix;
  }

  boost::dijkstra_shortest_paths(m_Network, *iterator, boost::predecessor_map(&predecessorMap[ 0 ]).distance_map(&distanceMatrix[ 0 ]).weight_map( boost::get( &NetworkEdge::edge_weight ,m_Network ) ) ) ;

  return distanceMatrix;
}

bool mitk::ConnectomicsNetwork::CheckForLabel( std::string targetLabel ) const
{
  boost::graph_traits<NetworkType>::vertex_iterator iterator, end;
  boost::tie(iterator, end) = boost::vertices( m_Network );

  while( (iterator != end) && (m_Network[ *iterator ].label != targetLabel) )
  {
    ++iterator;
  }

  if( iterator == end )
  {
    return false;
  }

  return true;
}
