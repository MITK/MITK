
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


#ifndef _MITK_ConnectomicsNetwork_H
#define _MITK_ConnectomicsNetwork_H

#include "MitkDiffusionImagingExports.h"

#include "mitkBaseData.h"

#include <boost/graph/adjacency_list.hpp>

namespace mitk {

  /**
  * \brief Connectomics Network Class
  *
  * This class is designed to represent a connectomics network (brain network). It encapsulates a
  * boost adjacency list and provides additional capabilities. The information contained in the nodes and edges is:
  *
  * Network Node:
  * <ul>
  *  <li> int ID - The id of the node
  *  <li> string label - The label of the node, this can be any string, such as an anatomical label
  *  <li> vector<float> coordinates - The coordinates the node should be displayed at
  * </ul>
  *
  * Network Edge:
  * <ul>
  *  <li> int sourceId - The Id of the source node
  *  <li> int targetId - The Id of the target node
  *  <li> int weight - Weight of the edge as int (used for counting fibers)
  *  <li> double edge_weight - Used for boost and algorithms, should be between 0 and 1
  * </ul>
  */
  class MitkDiffusionImaging_EXPORT ConnectomicsNetwork : public BaseData
  {
  public:
    /** Structs for the graph */

    /** The Node */
    struct NetworkNode
    {
      int id;
      std::string label;
      std::vector< float > coordinates;
    };

    /** The Edge */
    struct NetworkEdge
    {
      int sourceId;
      int targetId;
      int weight; // For now the number of times it was present
      double edge_weight; // For boost, currently set to 1 by default for unweighted calculations
    };

    /** Typedefs **/
    //typedef boost::adjacency_list< boost::listS, boost::listS, boost::undirectedS, NetworkNode, NetworkEdge > NetworkType;
    typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS, NetworkNode, NetworkEdge > NetworkType;
    typedef boost::graph_traits<NetworkType>::vertex_descriptor VertexDescriptorType;
    typedef boost::graph_traits<NetworkType>::edge_descriptor EdgeDescriptorType;

    // virtual methods that need to be implemented
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );


    // Macros
    mitkClassMacro( ConnectomicsNetwork, BaseData );
    itkNewMacro( Self );

    ////////////////// Interface ///////////////////
    /** return whether an edge exists between the two given vertices */
    bool EdgeExists( VertexDescriptorType vertexA, VertexDescriptorType vertexB ) const;

    /** increase the weight of an edge between the two given vertices */
    void IncreaseEdgeWeight( VertexDescriptorType vertexA, VertexDescriptorType vertexB );

    /** add an edge between two given vertices */
    void AddEdge( VertexDescriptorType vertexA, VertexDescriptorType vertexB);

    /** add an edge between two given vertices ( with a specific weight ) */
    void AddEdge( VertexDescriptorType vertexA, VertexDescriptorType vertexB, int sourceID, int targetID, int weight = 1 );

    /** add a vertex with a specified id */
    VertexDescriptorType AddVertex( int id);

    /** set the label of a vertex */
    void SetLabel( VertexDescriptorType vertex, std::string inLabel );

    /** set the coordinates of a vertex */
    void SetCoordinates( VertexDescriptorType vertex, std::vector< float > inCoordinates );

    /** clear the graph */
    void clear();

    /** return the node struct for a given node descriptor */
    NetworkNode GetNode( VertexDescriptorType vertex ) const;

    /** return the edge struct for two given node descriptors */
    NetworkEdge GetEdge( VertexDescriptorType vertexA, VertexDescriptorType vertexB ) const;

    /** get vector containing all the nodes of the network */
    std::vector< NetworkNode > GetVectorOfAllNodes() const;

    /** get vector containing all the vertex descriptors of the network */
    std::vector< VertexDescriptorType > GetVectorOfAllVertexDescriptors() const;

    /** get vector containing the descriptors of nodes adjacent to the vertex denoted by the given descriptor  */
    std::vector< VertexDescriptorType > GetVectorOfAdjacentNodes( VertexDescriptorType vertex ) const;

    /** get vector containing all the edges of the network and the connected nodes */
    std::vector< std::pair< std::pair< NetworkNode, NetworkNode > , NetworkEdge > > GetVectorOfAllEdges() const;

    /** get overall number of vertices in the network */
    int GetNumberOfVertices() const;

    /** get overall number of edges in the network */
    int GetNumberOfEdges() const;

    /** get number of vertices, that are connected to themselves */
    int GetNumberOfSelfLoops();

    /** get number of vertices, that are connected to themselves */
    double GetAverageDegree();

    /** get number of edges divided by number of possible edges */
    double GetConnectionDensity();

    /** Get the maximum weight of all edges */
    int GetMaximumWeight() const;

    /** Get a vector in the format vector[ vertexID ] = degree */
    std::vector< int > GetDegreeOfNodes( ) const;

    /** Get the maximum degree of all nodes */
    int GetMaximumDegree() const;

    /** Get a vector in the format vector[ vertexID ] = clustering coefficient */
    std::vector< double > GetLocalClusteringCoefficients( ) const;

    /** Get a vector in the format vector[ degree ] = average clustering coefficient */
    std::vector< double > GetClusteringCoefficientsByDegree( );

    /** Get the global clustering coefficient */
    double GetGlobalClusteringCoefficient( );

    /** Get the betweenness centrality for each vertex in form of a vector of length (number vertices)*/
    std::vector< double > GetNodeBetweennessVector() const;

    /** Get the betweenness centrality for each edge in form of a vector of length (number edges)*/
    std::vector< double > GetEdgeBetweennessVector() const;

    /** Check whether a vertex with the specified label exists*/
    bool CheckForLabel( std::string targetLabel ) const;

    /** Get the shortest distance from a specified vertex to all other vertices in form of a vector of length (number vertices)*/
    std::vector< double > GetShortestDistanceVectorFromLabel( std::string targetLabel ) const;

    /** Access boost graph directly */
    NetworkType* GetBoostGraph();

    /** Get the modified flag */
    bool GetIsModified() const;

    /** Set the modified flag */
    void SetIsModified( bool );

    /** Update the bounds of the geometry to fit the network */
    void UpdateBounds( );

    /** Remove nodes not connected to any other node */
    void PruneUnconnectedSingleNodes();

    /** Remove edges below the specified weight
      *
      * targetWeight is the number of connecting fibers
      *
      * This will remove unconnected nodes after removal
      */
    void PruneEdgesBelowWeight( int targetWeight );

  protected:
    ConnectomicsNetwork();
    virtual ~ConnectomicsNetwork();

    /** This function will relabel all vertices and edges in a continuous manner
      *
      * Mainly important after removing vertices, to make sure that the ids run continuously from
      * 0 to number of vertices - 1 and edge target and source ids match the corresponding node.
      */
    void UpdateIDs();

    NetworkType m_Network;

    /// Flag which indicates whether the network has been modified since the last check
    ///
    /// mainly for rendering purposes

    bool m_IsModified;

  private:

  };

} // namespace mitk

#endif /*  _MITK_ConnectomicsNetwork_H */
