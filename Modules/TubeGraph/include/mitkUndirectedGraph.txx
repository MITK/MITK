/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkUndirectedGraph_txx
#define _mitkUndirectedGraph_txx

namespace mitk
{
  template <class TVertex, class TEdge>
  UndirectedGraph<TVertex, TEdge>::UndirectedGraph()
  {
  }

  template <class TVertex, class TEdge>
  UndirectedGraph<TVertex, TEdge>::~UndirectedGraph()
  {
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType UndirectedGraph<TVertex, TEdge>::AddVertex(
    const typename UndirectedGraph<TVertex, TEdge>::VertexType &vertexData)
  {
    VertexDescriptorType vertex = boost::add_vertex(m_Graph);
    this->properties(vertex) = vertexData;

    this->Modified();

    return vertex;
  }

  template <class TVertex, class TEdge>
  void UndirectedGraph<TVertex, TEdge>::RemoveVertex(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex)
  {
    boost::clear_vertex(vertex, m_Graph);
    boost::remove_vertex(vertex, m_Graph);
    this->Modified();
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::VertexType UndirectedGraph<TVertex, TEdge>::GetVertex(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex)
  {
    return this->properties(vertex);
  }

  template <class TVertex, class TEdge>
  void UndirectedGraph<TVertex, TEdge>::SetVertex(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex,
    const typename UndirectedGraph<TVertex, TEdge>::VertexType &vertexData)
  {
    this->properties(vertex) = vertexData;

    this->Modified();
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType UndirectedGraph<TVertex, TEdge>::GetVertexDescriptor(
    const typename UndirectedGraph<TVertex, TEdge>::VertexType &vertexData) const
  {
    typename UndirectedGraph<TVertex, TEdge>::VertexIteratorType iterator, end;

    boost::tie(iterator, end) = boost::vertices(m_Graph);

    for (; iterator != end; ++iterator)
    {
      typename UndirectedGraph<TVertex, TEdge>::VertexType tempVertex;

      // the value of an iterator is a descriptor
      tempVertex = this->properties(*iterator);

      if (vertexData == tempVertex)
        return *iterator;
    }
    MITK_ERROR << "Error in mitkUndirectedGraph::GetVertexDescriptor(...): vertex could not be found." << std::endl;
    throw std::runtime_error(
      "Exception thrown in mitkUndirectedGraph::GetVertexDescriptor(...): vertex could not be found.");
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType UndirectedGraph<TVertex, TEdge>::AddEdge(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertexA,
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertexB,
    const typename UndirectedGraph<TVertex, TEdge>::EdgeType &edgeData)
  {
    EdgeDescriptorType edge;
    bool inserted = false;

    boost::tie(edge, inserted) = boost::add_edge(vertexA, vertexB, m_Graph);

    if (!inserted)
    {
      MITK_ERROR << "Error in mitkUndirectedGraph::addEdge(...): edge could not be inserted." << std::endl;
      throw std::runtime_error("Exception thrown in mitkUndirectedGraph::addEdge(...): edge could not be inserted.");
    }
    this->properties(edge) = edgeData;

    this->Modified();

    return edge;
  }

  template <class TVertex, class TEdge>
  void UndirectedGraph<TVertex, TEdge>::RemoveEdge(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge)
  {
    boost::remove_edge(edge, m_Graph);
    this->Modified();
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::EdgeType UndirectedGraph<TVertex, TEdge>::GetEdge(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge)
  {
    return this->properties(edge);
  }

  template <class TVertex, class TEdge>
  void UndirectedGraph<TVertex, TEdge>::SetEdge(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge,
    const typename UndirectedGraph<TVertex, TEdge>::EdgeType &edgeData)
  {
    this->properties(edge) = edgeData;

    this->Modified();
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType UndirectedGraph<TVertex, TEdge>::GetEdgeDescriptor(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeType &edgeData) const
  {
    typename UndirectedGraph<TVertex, TEdge>::EdgeIteratorType iterator, end;
    boost::tie(iterator, end) = boost::edges(m_Graph);

    for (; iterator != end; ++iterator)
    {
      typename UndirectedGraph<TVertex, TEdge>::EdgeType tempEdge;

      // the value of an iterator is a descriptor
      tempEdge = this->properties(*iterator);

      if (edgeData == tempEdge)
        return iterator.dereference();
    }
    // if no edge match, throw error
    MITK_ERROR << "Error in mitkUndirectedGraph::GetEdgeDescriptor(...): edge could not be found." << std::endl;
    throw std::runtime_error(
      "Exception thrown in mitkUndirectedGraph::GetEdgeDescriptor(...): edge could not be found.");
  }

  template <class TVertex, class TEdge>
  std::pair<typename UndirectedGraph<TVertex, TEdge>::VertexType, typename UndirectedGraph<TVertex, TEdge>::VertexType>
    UndirectedGraph<TVertex, TEdge>::GetVerticesOfAnEdge(
      const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge) const
  {
    typename UndirectedGraph<TVertex, TEdge>::VertexType sourceNode, targetNode;

    sourceNode = this->properties(boost::source(edge, m_Graph));
    targetNode = this->properties(boost::target(edge, m_Graph));
    std::pair<typename UndirectedGraph<TVertex, TEdge>::VertexType,
              typename UndirectedGraph<TVertex, TEdge>::VertexType>
      nodePair(sourceNode, targetNode);

    return nodePair;
  }

  template <class TVertex, class TEdge>
  typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType
    UndirectedGraph<TVertex, TEdge>::GetEdgeDescriptorByVerices(
      const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertexA,
      const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertexB) const
  {
    typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType edge;
    bool edgeExists(false);
    boost::tie(edge, edgeExists) = boost::edge(vertexA, vertexB, m_Graph);
    if (edgeExists)
      return edge;
    else
    {
      MITK_ERROR << "Error in mitkUndirectedGraph::GetEdgeDescriptorByVerices(...): edge could not be found."
                 << std::endl;
      throw std::runtime_error(
        "Exception thrown in mitkUndirectedGraph::GetEdgeDescriptorByVerices(...): edge could not be found.");
    }
  }

  template <class TVertex, class TEdge>
  std::vector<typename UndirectedGraph<TVertex, TEdge>::EdgeType> UndirectedGraph<TVertex, TEdge>::GetAllEdgesOfAVertex(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex) const
  {
    typename UndirectedGraph<TVertex, TEdge>::OutEdgeIteratorType iterator, end;
    boost::tie(iterator, end) = boost::out_edges(vertex, m_Graph);

    std::vector<typename UndirectedGraph<TVertex, TEdge>::EdgeType> vectorOfEdges;
    for (; iterator != end; ++iterator)
    {
      typename UndirectedGraph<TVertex, TEdge>::EdgeType tempEdge;

      // the value of an iterator is a descriptor
      tempEdge = this->properties(*iterator);

      vectorOfEdges.push_back(tempEdge);
    }

    return vectorOfEdges;
  }

  template <class TVertex, class TEdge>
  int UndirectedGraph<TVertex, TEdge>::GetNumberOfVertices() const
  {
    // Returns the number of vertices in the graph
    return boost::num_vertices(m_Graph);
  }

  template <class TVertex, class TEdge>
  int UndirectedGraph<TVertex, TEdge>::GetNumberOfEdges() const
  {
    // Returns the number of edges in the graph
    return boost::num_edges(m_Graph);
  }

  template <class TVertex, class TEdge>
  std::vector<typename UndirectedGraph<TVertex, TEdge>::VertexType>
    UndirectedGraph<TVertex, TEdge>::GetVectorOfAllVertices() const
  {
    typename UndirectedGraph<TVertex, TEdge>::VertexIteratorType iterator, end;

    // sets iterator to start end end to end
    boost::tie(iterator, end) = boost::vertices(m_Graph);

    std::vector<typename UndirectedGraph<TVertex, TEdge>::VertexType> vectorOfNodes;

    for (; iterator != end; ++iterator)
    {
      typename UndirectedGraph<TVertex, TEdge>::VertexType tempVertex;

      // the value of an iterator is a descriptor
      tempVertex = this->properties(*iterator);

      vectorOfNodes.push_back(tempVertex);
    }

    return vectorOfNodes;
  }

  template <class TVertex, class TEdge>
  std::vector<typename UndirectedGraph<TVertex, TEdge>::EdgeType> UndirectedGraph<TVertex, TEdge>::GetVectorOfAllEdges()
    const
  {
    typename UndirectedGraph<TVertex, TEdge>::EdgeIteratorType iterator, end;

    // sets iterator to start end end to end
    boost::tie(iterator, end) = boost::edges(m_Graph);

    std::vector<typename UndirectedGraph<TVertex, TEdge>::EdgeType> vectorOfEdges;

    for (; iterator != end; ++iterator)
    {
      typename UndirectedGraph<TVertex, TEdge>::EdgeType tempEdge;

      // the value of an iterator is a descriptor
      tempEdge = this->properties(*iterator);

      vectorOfEdges.push_back(tempEdge);
    }

    return vectorOfEdges;
  }

  template <class TVertex, class TEdge>
  void UndirectedGraph<TVertex, TEdge>::Clear()
  {
    m_Graph.clear();
    this->Modified();
  }

  template <class TVertex, class TEdge>
  const typename UndirectedGraph<TVertex, TEdge>::GraphType &UndirectedGraph<TVertex, TEdge>::GetGraph() const
  {
    return m_Graph;
  }

  /* operators */
  template <class TVertex, class TEdge>
  UndirectedGraph<TVertex, TEdge> &UndirectedGraph<TVertex, TEdge>::operator=(
    const UndirectedGraph<TVertex, TEdge> &rhs)
  {
    m_Graph = rhs.m_Graph;
    return *this;
  }

  template <class TVertex, class TEdge>
  TVertex &UndirectedGraph<TVertex, TEdge>::properties(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex)
  {
    typename boost::property_map<GraphType, vertex_properties_t>::type param = boost::get(vertex_properties, m_Graph);
    return param[vertex];
  }

  template <class TVertex, class TEdge>
  const TVertex &UndirectedGraph<TVertex, TEdge>::properties(
    const typename UndirectedGraph<TVertex, TEdge>::VertexDescriptorType &vertex) const
  {
    typename boost::property_map<GraphType, vertex_properties_t>::const_type param =
      boost::get(vertex_properties, m_Graph);
    return param[vertex];
  }

  template <class TVertex, class TEdge>
  TEdge &UndirectedGraph<TVertex, TEdge>::properties(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge)
  {
    typename boost::property_map<GraphType, edge_properties_t>::type param = boost::get(edge_properties, m_Graph);
    return param[edge];
  }

  template <class TVertex, class TEdge>
  const TEdge &UndirectedGraph<TVertex, TEdge>::properties(
    const typename UndirectedGraph<TVertex, TEdge>::EdgeDescriptorType &edge) const
  {
    typename boost::property_map<GraphType, edge_properties_t>::const_type param = boost::get(edge_properties, m_Graph);
    return param[edge];
  }

} // namespace mitk

#endif //_mitkUndirectedGraph_txx
