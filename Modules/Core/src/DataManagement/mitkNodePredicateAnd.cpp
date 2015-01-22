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

#include "mitkNodePredicateAnd.h"


mitk::NodePredicateAnd::NodePredicateAnd()
: NodePredicateCompositeBase()
{
}


mitk::NodePredicateAnd::NodePredicateAnd(const NodePredicateBase* p1, const NodePredicateBase* p2)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
}


mitk::NodePredicateAnd::NodePredicateAnd(const NodePredicateBase* p1, const NodePredicateBase* p2, const NodePredicateBase* p3)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
  this->AddPredicate(p3);
}


mitk::NodePredicateAnd::~NodePredicateAnd()
{
}


bool mitk::NodePredicateAnd::CheckNode(const mitk::DataNode* node) const
{
  if (m_ChildPredicates.empty())
    throw std::invalid_argument("NodePredicateAnd: no child predicates available");

  if (node == NULL)
    throw std::invalid_argument("NodePredicateAnd: invalid node");

  // return the conjunction of the child predicate. If any predicate returns false, we return false too
  for (ChildPredicates::const_iterator it = m_ChildPredicates.begin();  ( it != m_ChildPredicates.end() ); ++it)
    if ((*it)->CheckNode(node) == false)
      return false;   // if one element of the conjunction is false, the whole conjunction gets false
  return true;  // none of the childs was false, so return true
}
