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


#ifndef MITKNODEPREDICATEDIMENSIONONLY_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDIMENSIONONLY_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"

namespace mitk {
 class BaseData;

  //##Documentation
  //## @brief Predicate that evaluates if the given DataNodes data object
  //## has the specified dimension, for datasets where dimension is
  //## applicable.
  //##
  //## Evaluates to "false" for unsupported datasets.
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateDimensionOnly : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDimensionOnly, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDimensionOnly, unsigned int);

      //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateDimensionOnly();

    //##Documentation
    //## @brief Checks if the nodes data object is of the specified dimension
    virtual bool CheckNode(const mitk::DataNode* node) const override;

  protected:

    //##Documentation
    //## @brief Standard Constructor
    NodePredicateDimensionOnly(unsigned int dimension);

    unsigned int m_Dimension;
  };
} // namespace mitk

#endif /* MITKNodePredicateDimensionOnly_H_HEADER_INCLUDED_ */

