/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateOr_h
#define mitkNodePredicateOr_h

#include "mitkNodePredicateCompositeBase.h"

namespace mitk
{
  //##Documentation
  //## @brief Composite predicate that forms a logical OR relation from its child predicates
  //##
  //##
  //##
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateOr : public NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateOr, NodePredicateCompositeBase);
    itkFactorylessNewMacro(NodePredicateOr);
    mitkNewMacro2Param(NodePredicateOr, const NodePredicateBase *, const NodePredicateBase *);
    mitkNewMacro3Param(NodePredicateOr, const NodePredicateBase *, const NodePredicateBase *, const NodePredicateBase *);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateOr() override;

    //##Documentation
    //## @brief Checks, if the node fulfills any of the subpredicates conditions
    bool CheckNode(const DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor
    NodePredicateOr();
    //##Documentation
    //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
    NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2);
    //##Documentation
    //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
    NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2, const NodePredicateBase *p3);
  };

} // namespace mitk

#endif
