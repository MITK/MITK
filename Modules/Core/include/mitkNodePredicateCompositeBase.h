/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include <list>

namespace mitk
{
  //##Documentation
  //## @brief Base class for all predicates that can have child predicates (e.g. AND/OR)
  //##
  //## This class provides methods to add and remove child predicates. It is used for
  //## predicates that are compositions of other predicates like AND and OR.
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateCompositeBase : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateCompositeBase, NodePredicateBase);

    typedef std::list<NodePredicateBase::ConstPointer> ChildPredicates;

    //##Documentation
    //## @brief Pure virtual (but implemented) Destructor makes NodePredicateCompositeBase an abstract class
    ~NodePredicateCompositeBase() override = 0;

    //##Documentation
    //## @brief Adds a child predicate
    virtual void AddPredicate(const NodePredicateBase *p);

    //##Documentation
    //## @brief Removes a child predicate
    virtual void RemovePredicate(const NodePredicateBase *p);

    //##Documentation
    //## @brief Return all child predicates (immutable).
    virtual ChildPredicates GetPredicates() const;

  protected:
    //##Documentation
    //## @brief list of child predicates
    ChildPredicates m_ChildPredicates;
  };

} // namespace mitk

#endif /* MITKNODEPREDICATECOMPOSITEBASE_H_HEADER_INCLUDED_ */
