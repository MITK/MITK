/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateDimension_h
#define mitkNodePredicateDimension_h

#include "mitkNodePredicateBase.h"

namespace mitk
{
  class BaseData;

  //##Documentation
  //## @brief Predicate that evaluates if the given DataNodes data object
  //## has the specified dimension, for datasets where dimension is
  //## applicable.
  //##
  //## Evaluates to "false" for unsupported datasets.
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateDimension : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDimension, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDimension, unsigned int);
    mitkNewMacro2Param(NodePredicateDimension, unsigned int, int);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateDimension() override;

    //##Documentation
    //## @brief Checks if the nodes data object is of the specified dimension
    bool CheckNode(const mitk::DataNode *node) const override;

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

#endif
