/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTubeGraph_h
#define mitkTubeGraph_h

#include <MitkTubeGraphExports.h>

#include "mitkTubeGraphEdge.h"
#include "mitkTubeGraphVertex.h"
#include "mitkUndirectedGraph.h"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4172) // boost/graph/named_function_params.hpp(240): returning address of local variable or temporary
#endif

#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/property.hpp>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

namespace mitk
{
  /**
  * \brief Base Class for Tube Graphs
  */
  class MITKTUBEGRAPH_EXPORT TubeGraph : public UndirectedGraph<TubeGraphVertex, TubeGraphEdge>
  {
  public:
    /**
    * Defines the type of the id of a single tube. It is allways the source and target descriptor: [uint, uint]
    */
    typedef std::pair<VertexDescriptorType, VertexDescriptorType> TubeDescriptorType;

    typedef boost::adjacency_list<boost::vecS,
                                  boost::vecS,
                                  boost::directedS // directed graph
                                  >
      DirectedGraphType;

    /**
    * Represents a tube id which may not occur. This may be used
    * for error states or for indicating that something went wrong.
    */
    static const TubeDescriptorType ErrorId;

    //--- Macros ---//
    mitkClassMacro(TubeGraph, Superclass);
    itkNewMacro(Self);

    /**
    * Search only the shortest path between two tubes.
    */
    std::vector<TubeDescriptorType> SearchShortestPath(
      const TubeDescriptorType &startTube, const TubeDescriptorType &endTube /*, std::vector<unsigned long> barrier*/);

    /**
    * Search every path between two tubes.
    */
    std::vector<TubeDescriptorType> SearchAllPathBetweenVertices(
      const TubeDescriptorType &startTube, const TubeDescriptorType &endTube /*, std::vector<unsigned long> barrier*/);
    std::vector<TubeDescriptorType> SearchPathToPeriphery(
      const TubeDescriptorType &startTube /*, std::vector<unsigned long> barrier*/);

    /**
    * Find thickest tube in the graph. This means the tube which has the element with the largest diameter.
    */
    TubeDescriptorType GetThickestTube();

    /**
    * Get this undirected graph as a directed graph. For this convertation you need a start point.
    */
    DirectedGraphType GetDirectedGraph(VertexDescriptorType startVertex);

    /**
    * Create a subgraph with all given tubes. The origin graph would not be changed.
    */
    TubeGraph::Pointer CreateSubGraph(std::vector<TubeDescriptorType> subGraphTubes);

    /**
    * Remove a part of the graph. All tubes, that means all edges and if necessary also the vertex (no other out-edge)
    * of the tube will be deleted.
    */
    void RemoveSubGraph(std::vector<TubeDescriptorType> deletedTubes);

    void SetRootTube(const TubeDescriptorType &root);
    void SetRoot(const VertexDescriptorType &root);
    TubeDescriptorType GetRootTube();
    VertexDescriptorType GetRootVertex();

  protected:
    TubeGraph();
    TubeGraph(const TubeGraph &graph);
    ~TubeGraph() override;

    TubeGraph &operator=(const TubeGraph &rhs);

  private:
    TubeDescriptorType m_RootTube;
    VertexDescriptorType m_Root;

    void GetOutEdgesOfAVertex(VertexDescriptorType vertex,
                              DirectedGraphType &directedGraph,
                              std::vector<TubeDescriptorType> &pathToPeriphery);
  };

  /**
  * Defines a own graph visitor for breadth-first-search.
  * This visitor should be create a new directed graph by passing the edges of a undirected graph.
  */
  class DirectedGraphBfsVisitor : public boost::default_bfs_visitor
  {
  public:
    DirectedGraphBfsVisitor(TubeGraph *oldGraph, TubeGraph::DirectedGraphType &newGraph)
      : m_OrignialGraph(oldGraph), m_DirectedGraph(newGraph)
    {
    }

    void tree_edge(TubeGraph::EdgeDescriptorType e, const TubeGraph::GraphType &g)
    {
      unsigned int numberSource = boost::source(e, g);
      unsigned int numberTarget = boost::target(e, g);
      boost::graph_traits<TubeGraph::DirectedGraphType>::vertex_descriptor source = numberSource;
      boost::graph_traits<TubeGraph::DirectedGraphType>::vertex_descriptor target = numberTarget;

      boost::add_edge(source, target, m_DirectedGraph);
    }

  private:
    TubeGraph::Pointer m_OrignialGraph;
    TubeGraph::DirectedGraphType &m_DirectedGraph;
  };

} // namespace mitk

#endif
