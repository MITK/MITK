/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateOr.h"

mitk::NodePredicateOr::NodePredicateOr() : NodePredicateCompositeBase()
{
}

mitk::NodePredicateOr::NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2)
  : NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
}

mitk::NodePredicateOr::NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2, const NodePredicateBase *p3)
  : NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
  this->AddPredicate(p3);
}

mitk::NodePredicateOr::~NodePredicateOr()
{
}

bool mitk::NodePredicateOr::CheckNode(const DataNode *node) const
{
  if (m_ChildPredicates.empty())
    throw std::invalid_argument("NodePredicateOr: no child predicates available");

  if (node == nullptr)
    throw std::invalid_argument("NodePredicateOr: invalid node");

  /* return the disjunction of the child predicate. If any predicate returns true, we return true too. Return false only
   * if all child predicates return false */
  for (auto it = m_ChildPredicates.cbegin(); it != m_ChildPredicates.cend(); ++it)
    if ((*it)->CheckNode(node) == true)
      return true;
  return false; // none of the childs was true, so return false
}
