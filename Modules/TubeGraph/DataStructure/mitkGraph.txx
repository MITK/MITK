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
#ifndef _mitkGraph_txx
#define _mitkGraph_txx

namespace mitk
{

template <class TVertex,class TEdge>
Graph<TVertex, TEdge>
::Graph()
: m_Graph(NULL)
{
}

template <class TVertex,class TEdge>
Graph<TVertex, TEdge>
::Graph(const Graph<TVertex, TEdge> & graph)
: m_Graph(graph.m_Graph)
{
}

template <class TVertex,class TEdge>
Graph<TVertex, TEdge>
::~Graph()
{
}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::VertexDescriptorType
Graph< TVertex, TEdge >
::AddVertex(const typename Graph<TVertex, TEdge>::VertexType& vertexData)
{
  VertexDescriptorType vertex = boost::add_vertex(m_Graph);
  this->properties(vertex) = vertexData;

  this->Modified();

  return vertex;
}

template <class TVertex,class TEdge>
void
Graph<TVertex, TEdge>
::RemoveVertex(const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertex)
{
  boost::clear_vertex(vertex, m_Graph);
  boost::remove_vertex(vertex, m_Graph);
  this->Modified();
}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::VertexType
Graph<TVertex, TEdge>
::GetVertex(const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertex)
{
  return this->properties(vertex);
}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::VertexDescriptorType
Graph<TVertex, TEdge>
::GetVertexDescriptor(const typename Graph<TVertex, TEdge>::VertexType& vertexData)
{
 typename Graph<TVertex, TEdge>::VertexIteratorType iterator, end;

  boost::tie(iterator, end) = boost::vertices( m_Graph );

  for ( ; iterator != end; ++iterator)
  {
    typename Graph<TVertex, TEdge>::VertexType tempVertex;

    // the value of an iterator is a descriptor
    tempVertex = this->properties(*iterator);

    if (vertexData == tempVertex)
      return *iterator;
  }
  MITK_ERROR << "Error in mitkGraph::GetVertexDescriptor(...): vertex could not be found." << std::endl;
   throw std::runtime_error("Exception thrown in mitkGraph::GetVertexDescriptor(...): vertex could not be found.");

}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::EdgeDescriptorType Graph<TVertex, TEdge>
::AddEdge(const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertexA, const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertexB, const typename Graph<TVertex, TEdge>::EdgeType& edgeData)
{
  EdgeDescriptorType edge;
  bool inserted = false;

  boost::tie(edge, inserted) = boost::add_edge(vertexA, vertexB, m_Graph);

  if (!inserted){
    MITK_ERROR << "Error in mitkGraph::addEdge(...): edge could not be inserted." << std::endl;
    throw std::runtime_error("Exception thrown in mitkGraph::addEdge(...): edge could not be inserted.");
  }
  this->properties(edge) = edgeData;

  this->Modified();

  return edge;
}

template <class TVertex,class TEdge>
void
Graph<TVertex, TEdge>
::RemoveEdge(const typename Graph<TVertex, TEdge>::EdgeDescriptorType& edge)
{
  boost::remove_edge(boost::source(edge), boost::target(edge), m_Graph);
  this->Modified();
}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::EdgeType
Graph<TVertex, TEdge>
::GetEdge(const typename Graph<TVertex, TEdge>::EdgeDescriptorType& edge)
{
  return this->properties(edge);
}

template <class TVertex,class TEdge>
typename Graph<TVertex, TEdge>::EdgeDescriptorType
Graph<TVertex, TEdge>
::GetEdgeDescriptor(const typename Graph<TVertex, TEdge>::EdgeType& edgeData)
{
  typename Graph<TVertex, TEdge>::EdgeIteratorType iterator, end;
  boost::tie(iterator, end) = boost::edges(m_Graph);

  for ( ; iterator != end; ++iterator)
  {
    typename Graph<TVertex, TEdge>::EdgeType tempEdge;

    // the value of an iterator is a descriptor
    tempEdge = this->properties(*iterator);

   if (edgeData == tempEdge)
      return iterator.dereference();
  }
  //if no edge match, throw error
    MITK_ERROR << "Error in mitkGraph::GetEdgeDescriptor(...): edge could not be found." << std::endl;
    throw std::runtime_error("Exception thrown in mitkGraph::GetEdgeDescriptor(...): edge could not be found.");

}

template <class TVertex,class TEdge>
std::pair<typename Graph<TVertex, TEdge>::VertexType, typename Graph<TVertex, TEdge>::VertexType >
Graph<TVertex, TEdge>
::GetVerticesOfAnEdge (const typename Graph<TVertex, TEdge>::EdgeDescriptorType& edge)const
{
 typename Graph<TVertex, TEdge>::VertexType sourceNode, targetNode;

 sourceNode = this->properties(boost::source(edge, m_Graph));
 targetNode = this->properties(boost::target(edge, m_Graph));
 std::pair<typename Graph<TVertex, TEdge>::VertexType,typename Graph<TVertex, TEdge>::VertexType > nodePair( sourceNode, targetNode );

 return nodePair;
}

template <class TVertex,class TEdge>
std::vector<typename Graph<TVertex, TEdge>::EdgeType>
Graph<TVertex, TEdge>
::GetAllEdgesOfAVertex (const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertex)const
{
 typename Graph<TVertex, TEdge>::OutEdgeIteratorType iterator, end;
 boost::tie(iterator, end) = boost::out_edges(vertex, m_Graph);

 std::vector< typename Graph<TVertex, TEdge>::EdgeType > vectorOfEdges;
 for ( ; iterator != end; ++iterator)
  {

    typename Graph<TVertex, TEdge>::EdgeType tempEdge;

    // the value of an iterator is a descriptor
    tempEdge = this->properties(*iterator);

    vectorOfEdges.push_back( tempEdge );
  }

  return vectorOfEdges;
}

template <class TVertex,class TEdge>
int
Graph<TVertex, TEdge>
::GetNumberOfVertices() const
{
  //Returns the number of vertices in the graph
  return boost::num_vertices( m_Graph );
}

template <class TVertex,class TEdge>
int
Graph<TVertex, TEdge>
::GetNumberOfEdges() const
{
  //Returns the number of edges in the graph
  return boost::num_edges( m_Graph );
}

template <class TVertex,class TEdge>
std::vector< typename Graph<TVertex, TEdge>::VertexType >
Graph<TVertex, TEdge>
::GetVectorOfAllVertices() const
{
  typename Graph<TVertex, TEdge>::VertexIteratorType iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::vertices( m_Graph );

  std::vector< typename Graph<TVertex, TEdge>::VertexType > vectorOfNodes;

  for ( ; iterator != end; ++iterator)
  {
    typename Graph<TVertex, TEdge>::VertexType tempVertex;

    // the value of an iterator is a descriptor
    tempVertex = this->properties(*iterator);

    vectorOfNodes.push_back( tempVertex  );
  }

  return vectorOfNodes;
}

template <class TVertex,class TEdge>
std::vector< typename Graph<TVertex, TEdge>::EdgeType >
Graph<TVertex, TEdge>
::GetVectorOfAllEdges() const
{
 typename Graph<TVertex, TEdge>::EdgeIteratorType iterator, end;

  // sets iterator to start end end to end
  boost::tie(iterator, end) = boost::edges(m_Graph);

  std::vector<typename Graph<TVertex, TEdge>::EdgeType> vectorOfEdges;

  for ( ; iterator != end; ++iterator)
  {

    typename Graph<TVertex, TEdge>::EdgeType tempEdge;

    // the value of an iterator is a descriptor
    tempEdge = this->properties(*iterator);

    vectorOfEdges.push_back( tempEdge );
  }

  return vectorOfEdges;
}

template <class TVertex,class TEdge>
void
Graph<TVertex, TEdge>
::Clear()
{
  m_Graph.clear();
  this->Modified();
}

template <class TVertex,class TEdge>
const typename Graph<TVertex, TEdge>::GraphType&
Graph<TVertex, TEdge>
::GetGraph() const
{
  return m_Graph;
}

/* operators */
template <class TVertex,class TEdge>
Graph<TVertex, TEdge>&
Graph<TVertex, TEdge>
::operator=(const Graph<TVertex, TEdge>& rhs)
{
  m_Graph = rhs.m_Graph;
  return *this;
}

template <class TVertex,class TEdge>
TVertex&
Graph<TVertex, TEdge>
::properties(const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertex)
{
  typename boost::property_map<GraphType, vertex_properties_t>::type param = boost::get(vertex_properties, m_Graph);
  return param[vertex];
}

template <class TVertex,class TEdge>
const TVertex&
Graph<TVertex, TEdge>
::properties(const typename Graph<TVertex, TEdge>::VertexDescriptorType& vertex) const
{
  typename boost::property_map<GraphType, vertex_properties_t>::const_type param = boost::get(vertex_properties, m_Graph);
  return param[vertex];
}

template <class TVertex,class TEdge>
TEdge&
Graph<TVertex, TEdge>
::properties(const typename Graph<TVertex, TEdge>::EdgeDescriptorType& edge)
{
  typename boost::property_map<GraphType, edge_properties_t>::type param = boost::get(edge_properties, m_Graph);
  return param[edge];
}

template <class TVertex,class TEdge>
const TEdge&
Graph<TVertex, TEdge>
::properties(const typename Graph<TVertex, TEdge>::EdgeDescriptorType& edge) const
{
  typename boost::property_map<GraphType, edge_properties_t>::const_type param = boost::get(edge_properties, m_Graph);
  return param[edge];
}

}//namespace mitk

#endif //_mitkGraph_txx
