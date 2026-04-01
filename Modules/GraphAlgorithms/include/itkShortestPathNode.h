/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkShortestPathNode_h
#define itkShortestPathNode_h

#include <MitkGraphAlgorithmsExports.h>

namespace itk
{
  /** \brief Type used to represent path costs (distances). */
  typedef double DistanceType;

  /**
   * \brief Type used for node indexing.
   *
   * An unsigned int supports up to approximately 4.2 billion nodes
   * on 32-bit systems.
   */
  typedef unsigned int NodeNumType;

  /**
   * \brief A node in the shortest path graph used by ShortestPathImageFilter.
   *
   * Each ShortestPathNode corresponds to a single pixel in the image and stores
   * the state needed for the A*graph search algorithm: accumulated cost,
   * estimated total cost, predecessor link, and closed status.
   *
   * \sa ShortestPathImageFilter
   * \sa ShortestPathCostFunction
   */
  class MITKGRAPHALGORITHMS_EXPORT ShortestPathNode
  {
  public:
    DistanceType distance;     ///< Minimal cumulative cost from the start node to this node.
    DistanceType distAndEst;   ///< Sum of distance and estimated remaining cost to the target (used by A*).
    NodeNumType prevNode;      ///< Index of the predecessor node on the shortest path.
    NodeNumType mainListIndex; ///< Index of this node in the main node array (m_Nodes).
    bool closed;               ///< True if this node's optimal path to the start has been determined.
  };
}

#endif
