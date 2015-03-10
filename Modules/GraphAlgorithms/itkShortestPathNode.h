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
#ifndef __itkShortestPathNode_h_
#define __itkShortestPathNode_h_

#include "MitkGraphAlgorithmsExports.h"

namespace itk
{
  typedef double                 DistanceType; // Type to declare the costs
    typedef unsigned int           NodeNumType;  // Type for Node Numeration: unsignend int for up to 4.2 billion pixel in 32Bit system

  class MITKGRAPHALGORITHMS_EXPORT ShortestPathNode
  {
  public:
      DistanceType distance;      // minimal costs from StartPoint to this pixel
     DistanceType distAndEst;    // Distance+Estimated Distnace to target
      NodeNumType prevNode;       // previous node. Important to find the Shortest Path
      NodeNumType mainListIndex;  // Indexnumber of this node in m_Nodes
      bool closed; // determines if this node is closes, so its optimal path to startNode is known
  };

  //bool operator<(const ShortestPathNode &a) const;
  //bool operator==(const ShortestPathNode &a) const;

}



#endif
