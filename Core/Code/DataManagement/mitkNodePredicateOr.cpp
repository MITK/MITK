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

#include "mitkNodePredicateOr.h"




mitk::NodePredicateOr::NodePredicateOr()
: NodePredicateCompositeBase()
{
}

mitk::NodePredicateOr::NodePredicateOr(const NodePredicateBase* p1, const NodePredicateBase* p2)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
}

mitk::NodePredicateOr::~NodePredicateOr()
{
}


bool mitk::NodePredicateOr::CheckNode(const DataNode* node) const
{
  if (m_ChildPredicates.empty())
    throw std::invalid_argument("NodePredicateOr: no child predicates available");

  if (node == NULL)
    throw std::invalid_argument("NodePredicateOr: invalid node");

  /* return the disjunction of the child predicate. If any predicate returns true, we return true too. Return false only if all child predicates return false */
  for (ChildPredicates::const_iterator it = m_ChildPredicates.begin(); it != m_ChildPredicates.end(); ++it)
    if ((*it)->CheckNode(node) == true)
      return true;
  return false; // none of the childs was true, so return false
}
