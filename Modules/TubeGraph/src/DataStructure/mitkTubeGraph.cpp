/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraph.h"
#include "mitkGeometry3D.h"

const mitk::TubeGraph::TubeDescriptorType mitk::TubeGraph::ErrorId =
  std::pair<VertexDescriptorType, VertexDescriptorType>(boost::graph_traits<GraphType>::null_vertex(),
                                                        boost::graph_traits<GraphType>::null_vertex());

mitk::TubeGraph::TubeGraph()
{
}

mitk::TubeGraph::TubeGraph(const mitk::TubeGraph &graph) : UndirectedGraph<TubeGraphVertex, TubeGraphEdge>(graph)
{
}

mitk::TubeGraph::~TubeGraph()
{
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraph::SearchShortestPath(
  const TubeDescriptorType &, const TubeDescriptorType &)
{
  std::vector<TubeDescriptorType> shortestPath;
  return shortestPath;
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraph::SearchAllPathBetweenVertices(
  const mitk::TubeGraph::TubeDescriptorType &startTube,
  const mitk::TubeGraph::TubeDescriptorType &endTube /*, std::vector<unsigned long> barrier*/)
{ // http://lists.boost.org/boost-users/att-9001/maze.cpp
  // http://www.boost.org/doc/libs/1_49_0/libs/graph/example/bfs.cpp

  typedef std::map<VertexDescriptorType, EdgeDescriptorType> EdgeMap;
  typedef boost::associative_property_map<EdgeMap> PredecessorMap;
  typedef boost::edge_predecessor_recorder<PredecessorMap, boost::on_tree_edge> PredecessorVisitor;
  typedef boost::dfs_visitor<std::pair<PredecessorVisitor, boost::null_visitor>> DFSVisitor;

  EdgeMap edgesMap;
  PredecessorMap predecessorMap(edgesMap);

  PredecessorVisitor predecessorVisitor(predecessorMap);
  boost::null_visitor nullVisitor;
  DFSVisitor visitor = boost::make_dfs_visitor(std::make_pair(predecessorVisitor, nullVisitor));

  std::map<VertexDescriptorType, boost::default_color_type> vertexColorMap;
  std::map<EdgeDescriptorType, boost::default_color_type> edgeColorMap;

  boost::undirected_dfs(
    m_Graph, visitor, make_assoc_property_map(vertexColorMap), make_assoc_property_map(edgeColorMap), startTube.second);

  std::vector<TubeDescriptorType> solutionPath;
  solutionPath.push_back(endTube);
  VertexDescriptorType pathEdgeSource = endTube.first;
  VertexDescriptorType pathEdgeTarget;

  MITK_INFO << "Source: [" << startTube.first << "," << startTube.second << "] Target: [" << endTube.first << ","
            << endTube.second << "]";
  MITK_INFO << "tube [" << endTube.first << "," << endTube.second << "]";
  do
  {
    if (pathEdgeSource == 10393696)
      break;
    EdgeDescriptorType edge = get(predecessorMap, pathEdgeSource);
    pathEdgeSource = boost::source(edge, m_Graph);
    pathEdgeTarget = boost::target(edge, m_Graph);
    TubeDescriptorType tube(pathEdgeSource, pathEdgeTarget);
    MITK_INFO << "tube [" << tube.first << "," << tube.second << "]";
    solutionPath.push_back(tube);
  } while (pathEdgeSource != startTube.second);

  return solutionPath;
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraph::SearchPathToPeriphery(
  const mitk::TubeGraph::TubeDescriptorType &startTube /*, std::vector<unsigned long> barrier*/)
{
  std::vector<mitk::TubeGraph::TubeDescriptorType> pathToPeriphery;

  if (m_RootTube == ErrorId)
  {
    m_RootTube = this->GetThickestTube();
    if (m_Root == m_RootTube.first)
      m_Root = m_RootTube.second;
    else
      m_Root = m_RootTube.first;
  }

  // Attention!! No check which one is the right one
  DirectedGraphType directedGraph = this->GetDirectedGraph(m_Root);

  // Only the target vertex: it's a directed Graph, and we want only the "after tube" tubes and the clicked ones
  // this->GetOutEdgesOfAVertex(startTube.first, directedGraph, pathToPeriphery);
  pathToPeriphery.push_back(startTube);
  this->GetOutEdgesOfAVertex(startTube.second, directedGraph, pathToPeriphery);

  return pathToPeriphery;
}

void mitk::TubeGraph::GetOutEdgesOfAVertex(mitk::TubeGraph::VertexDescriptorType vertex,
                                           mitk::TubeGraph::DirectedGraphType &directedGraph,
                                           std::vector<mitk::TubeGraph::TubeDescriptorType> &pathToPeriphery)
{
  typedef boost::graph_traits<DirectedGraphType>::out_edge_iterator OutEdgeIteratorType;
  std::pair<OutEdgeIteratorType, OutEdgeIteratorType> outEdges = boost::out_edges(vertex, directedGraph);
  for (; outEdges.first != outEdges.second; ++outEdges.first)
  {
    TubeDescriptorType tube;
    tube.first = boost::source(*outEdges.first, directedGraph);
    tube.second = boost::target(*outEdges.first, directedGraph);

    if (std::find(pathToPeriphery.begin(), pathToPeriphery.end(), tube) == pathToPeriphery.end())
    {
      pathToPeriphery.push_back(tube);
      this->GetOutEdgesOfAVertex(tube.second, directedGraph, pathToPeriphery);
    }
  }
}

mitk::TubeGraph::TubeDescriptorType mitk::TubeGraph::GetThickestTube()
{
  TubeDescriptorType thickestTube;
  float tubeDiameter = 0.0;

  EdgeIteratorType iterator, end;

  boost::tie(iterator, end) = boost::edges(m_Graph);

  for (; iterator != end; ++iterator)
  {
    TubeGraphEdge edge = this->GetEdge(*iterator);

    std::pair<TubeGraphVertex, TubeGraphVertex> soureTargetPair = this->GetVerticesOfAnEdge(*iterator);

    float tempDiameter = edge.GetEdgeAverageDiameter(soureTargetPair.first, soureTargetPair.second);
    if (tempDiameter > tubeDiameter)
    {
      tubeDiameter = tempDiameter;
      thickestTube.first = this->GetVertexDescriptor(soureTargetPair.first);
      thickestTube.second = this->GetVertexDescriptor(soureTargetPair.second);
    }
  }
  return thickestTube;
}

mitk::TubeGraph::DirectedGraphType mitk::TubeGraph::GetDirectedGraph(VertexDescriptorType startVertex)
{
  DirectedGraphType directedGraph(boost::num_vertices(m_Graph));
  DirectedGraphBfsVisitor vis(this, directedGraph);
  boost::breadth_first_search(m_Graph, startVertex, visitor(vis));
  return directedGraph;
}

mitk::TubeGraph::Pointer mitk::TubeGraph::CreateSubGraph(std::vector<TubeDescriptorType> subGraphTubes)
{
  TubeGraph::Pointer subGraph = new TubeGraph();
  // store the descriptor from origin graph to the correspondent new descriptors
  std::map<VertexDescriptorType, VertexDescriptorType> vertexDescriptorOldToNewMap;

  // add a new edge and if necessary also the vertices of each tube to the new sub graph
  for (auto it = subGraphTubes.begin(); it != subGraphTubes.end(); it++)
  {
    // search for the source vertex in the subgraph; if it is already added continue, otherwise add it
    if (vertexDescriptorOldToNewMap.find(it->first) == vertexDescriptorOldToNewMap.end())
    {
      // add the vertex to the subgraph
      VertexDescriptorType newVertexDescriptor = subGraph->AddVertex(this->GetVertex(it->first));
      // add the pair of descriptor from the origin graph to the descriptor from the subgraph
      vertexDescriptorOldToNewMap.insert(
        std::pair<VertexDescriptorType, VertexDescriptorType>(it->first, newVertexDescriptor));
    }
    // and now...search for the target vertex...
    if (vertexDescriptorOldToNewMap.find(it->second) == vertexDescriptorOldToNewMap.end())
    {
      VertexDescriptorType newVertexDescriptor = subGraph->AddVertex(this->GetVertex(it->second));
      vertexDescriptorOldToNewMap.insert(
        std::pair<VertexDescriptorType, VertexDescriptorType>(it->second, newVertexDescriptor));
    }
    // Get the EdgeDescriptor from origin graph
    EdgeDescriptorType edgeDescriptor = this->GetEdgeDescriptorByVerices(it->first, it->second);

    TubeGraphEdge oldEdge = this->GetEdge(edgeDescriptor);

    // AddEdge needs the source vertex, the target vertex and the edge data
    // source Vertex: get the subgraph VertexDescriptor  by the origin descriptor (tubeDescriptor->first)from the
    // assigning map
    // target Vertex: get the subgraph VertexDescriptor  by the origin descriptor (tubeDescriptor->second)from the
    // assigning map
    // edge data: copy the TubeGraphEdge object using the origin edge desciptor and the origin graph
    VertexDescriptorType sourceVertex = vertexDescriptorOldToNewMap[it->first];
    VertexDescriptorType targetVertex = vertexDescriptorOldToNewMap[it->second];
    subGraph->AddEdge(sourceVertex, targetVertex, this->GetEdge(edgeDescriptor));
  }
  subGraph->CopyInformation(this);

  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  geometry->Initialize();
  subGraph->SetGeometry(geometry);

  this->Modified();

  return subGraph;
}

void mitk::TubeGraph::RemoveSubGraph(std::vector<TubeDescriptorType> deletedTubes)
{
  for (auto it = deletedTubes.begin(); it != deletedTubes.end(); it++)
  {
    VertexDescriptorType source = it->first;
    VertexDescriptorType target = it->second;

    EdgeDescriptorType edge = this->GetEdgeDescriptorByVerices(source, target);
    this->RemoveEdge(edge);

    if (this->GetAllEdgesOfAVertex(source).size() == 0)
    {
      this->RemoveVertex(source);
    }
    if (this->GetAllEdgesOfAVertex(target).size() == 0)
    {
      this->RemoveVertex(target);
    }
  }
  this->Modified();
}

void mitk::TubeGraph::SetRootTube(const TubeDescriptorType &root)
{
  if (root != TubeGraph::ErrorId)
  {
    m_RootTube = root;
    if (m_Root == root.first)
      m_Root = root.second;
    else
      m_Root = root.first;
    this->Modified();
  }
}

void mitk::TubeGraph::SetRoot(const VertexDescriptorType &root)
{
  if (root != TubeGraph::ErrorId.first)
  {
    m_Root = root;
  }
}

mitk::TubeGraph::TubeDescriptorType mitk::TubeGraph::GetRootTube()
{
  return m_RootTube;
}

mitk::TubeGraph::VertexDescriptorType mitk::TubeGraph::GetRootVertex()
{
  return m_Root;
}

mitk::TubeGraph &mitk::TubeGraph::operator=(const mitk::TubeGraph &rhs)
{
  UndirectedGraph<TubeGraphVertex, TubeGraphEdge>::operator=(rhs);
  return *this;
}
