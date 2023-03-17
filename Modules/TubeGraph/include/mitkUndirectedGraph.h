/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUndirectedGraph_h
#define mitkUndirectedGraph_h

#include <mitkBaseData.h>

#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#endif

/* definition of basic boost::graph properties */
enum vertex_properties_t
{
  vertex_properties
};
enum edge_properties_t
{
  edge_properties
};

namespace boost
{
  BOOST_INSTALL_PROPERTY(vertex, properties);
  BOOST_INSTALL_PROPERTY(edge, properties);
}

namespace mitk
{
  /**
  * \brief Template class for undirected graphs.Paramters should be the vertex and edge classes, which contains the
  * information.
  */
  template <class TVertex, class TEdge>
  class UndirectedGraph : public BaseData
  {
  public:
    //--- Typedefs ---//

    typedef TVertex VertexType;
    typedef TEdge EdgeType;
    /**
    * Creating the graph type
    * listS:          Represents the OutEdgeList as a std::list
    * vecS:           Represents the VertexList as a std::vector
    * undirectedS:    Representation for an undirected graph
    * VertexProperty: Defines that all vertex are represented by VertexType
    * EdgeProperty:   Defines that all edges  are represented by EdgeType
    */
    typedef boost::adjacency_list<boost::listS,
                                  boost::vecS,
                                  boost::undirectedS, // undirected graph
                                  boost::property<vertex_properties_t, VertexType>,
                                  boost::property<edge_properties_t, EdgeType>>
      GraphType;

    /* a bunch of graph-specific typedefs */
    typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexDescriptorType;
    typedef typename boost::graph_traits<GraphType>::edge_descriptor EdgeDescriptorType;

    typedef typename boost::graph_traits<GraphType>::vertex_iterator VertexIteratorType;
    typedef typename boost::graph_traits<GraphType>::edge_iterator EdgeIteratorType;
    typedef typename boost::graph_traits<GraphType>::adjacency_iterator AdjacenyIteratorType;
    typedef typename boost::graph_traits<GraphType>::out_edge_iterator OutEdgeIteratorType;
    typedef typename boost::graph_traits<GraphType>::in_edge_iterator InEdgeIteratorType;

    //--- Macros ---//
    mitkClassMacro(UndirectedGraph, BaseData);
    itkNewMacro(Self);

    // virtual methods that need to be implemented
    void UpdateOutputInformation() override
    {
      if (this->GetSource())
        this->GetSource()->UpdateOutputInformation();
    }
    void SetRequestedRegionToLargestPossibleRegion() override {}
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return false; }
    bool VerifyRequestedRegion() override { return true; }
    void SetRequestedRegion(const itk::DataObject *) override {}
    /** Add a new vertex to the graph */
    VertexDescriptorType AddVertex(const VertexType &vertexData);

    /** Remove the vertex from the graph */
    void RemoveVertex(const VertexDescriptorType &vertex);

    /** Get the vertex data of the given vertex descriptor */
    VertexType GetVertex(const VertexDescriptorType &vertex);

    /** Set the vertex data of the given vertex descriptor */
    void SetVertex(const VertexDescriptorType &vertex, const VertexType &vertexData);

    /**Returns the descriptor if the vertex exist in the graph, otherwise undefined*/
    VertexDescriptorType GetVertexDescriptor(const VertexType &vertexData) const;

    /** Add a new edge between the given vertices to the graph */
    EdgeDescriptorType AddEdge(const VertexDescriptorType &vertexA,
                               const VertexDescriptorType &vertexB,
                               const EdgeType &edgeData);

    /** Remove the edge from the graph */
    void RemoveEdge(const EdgeDescriptorType &edge);

    /** Get the edge data of the given edge descriptor */
    EdgeType GetEdge(const EdgeDescriptorType &edge);

    /** Set the edge data of the given edge descriptor */
    void SetEdge(const EdgeDescriptorType &edge, const EdgeType &edgeData);

    /**Returns the descriptor if the edge exist in the graph, otherwise undefined*/
    EdgeDescriptorType GetEdgeDescriptor(const EdgeType &edgeData) const;

    /** Get parent and target vertex of the given edge*/
    std::pair<VertexType, VertexType> GetVerticesOfAnEdge(const EdgeDescriptorType &edge) const;

    /** Returns the edge descriptor from the edge which has the given vertices as source and target */
    EdgeDescriptorType GetEdgeDescriptorByVerices(const VertexDescriptorType &vertexA,
                                                  const VertexDescriptorType &vertexB) const;

    /** Get all edges of the given vertex */
    std::vector<EdgeType> GetAllEdgesOfAVertex(const VertexDescriptorType &vertex) const;

    /** Get overall number of vertices in the graph */
    int GetNumberOfVertices() const;

    /** get overall number of edges in the graph */
    int GetNumberOfEdges() const;

    /** get vector containing all the vertices of the graph*/
    std::vector<VertexType> GetVectorOfAllVertices() const;

    /** get vector containing all the edges of the network  */
    std::vector<EdgeType> GetVectorOfAllEdges() const;

    /** clear the graph */
    void Clear() override;

    /** get the graph */
    const GraphType &GetGraph() const;

    UndirectedGraph<VertexType, EdgeType> &operator=(const UndirectedGraph<VertexType, EdgeType> &rhs);

  protected:
    UndirectedGraph();
    ~UndirectedGraph() override;

    GraphType m_Graph;

  private:
    /** property access for vertices */
    VertexType &properties(const VertexDescriptorType &vertex);
    /** property access for vertices */
    const VertexType &properties(const VertexDescriptorType &vertex) const;
    /** property access for edges */
    EdgeType &properties(const EdgeDescriptorType &edge);
    /** property access for edges */
    const EdgeType &properties(const EdgeDescriptorType &edge) const;
  };

} // namespace mitk

#include "mitkUndirectedGraph.txx"

#endif
