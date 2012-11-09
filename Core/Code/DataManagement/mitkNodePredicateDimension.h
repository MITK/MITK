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


#ifndef MITKNODEPREDICATEDIMENSION_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEDIMENSION_H_HEADER_INCLUDED_

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
  class MITK_CORE_EXPORT NodePredicateDimension : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDimension, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDimension, unsigned int);
    mitkNewMacro2Param(NodePredicateDimension, unsigned int, int);

      //##Documentation
    //## @brief Standard Destructor
    virtual ~NodePredicateDimension();

    //##Documentation
    //## @brief Checks if the nodes data object is of the specified dimension
    virtual bool CheckNode(const mitk::DataNode* node) const;

  protected:

    //##Documentation
    //## @brief Standard Constructor
    NodePredicateDimension(unsigned int dimension);

    //##Documentation
    //## @brief Standard Constructor
    NodePredicateDimension(unsigned int dimension, int pixelComponents);

    unsigned int m_Dimension;
    std::size_t m_PixelComponents;
  };
} // namespace mitk

#endif /* MITKNodePredicateDimension_H_HEADER_INCLUDED_ */

