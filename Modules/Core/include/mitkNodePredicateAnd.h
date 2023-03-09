/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateAnd_h
#define mitkNodePredicateAnd_h

#include "mitkNodePredicateCompositeBase.h"

namespace mitk
{
  //##Documentation
  //## @brief Composite predicate that forms a logical AND relation from its child predicates
  //##
  //##
  //##
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateAnd : public NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateAnd, NodePredicateCompositeBase);

    itkFactorylessNewMacro(NodePredicateAnd);
    mitkNewMacro2Param(NodePredicateAnd, const NodePredicateBase *, const NodePredicateBase *);
    mitkNewMacro3Param(NodePredicateAnd,
                       const NodePredicateBase *,
                       const NodePredicateBase *,
                       const NodePredicateBase *);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateAnd() override;

    //##Documentation
    //## @brief Checks, if the node fulfills all of the subpredicates conditions
    bool CheckNode(const DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    NodePredicateAnd();

    //##Documentation
    //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
    //## Protected constructor, use static instantiation functions instead
    NodePredicateAnd(const NodePredicateBase *p1, const NodePredicateBase *p2);

    //##Documentation
    //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
    //## Protected constructor, use static instantiation functions instead
    NodePredicateAnd(const NodePredicateBase *p1, const NodePredicateBase *p2, const NodePredicateBase *p3);
  };

} // namespace mitk

#endif
