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

#include "mitkNodePredicateNot.h"


mitk::NodePredicateNot::NodePredicateNot(const mitk::NodePredicateBase* p)
{
  m_ChildPredicates.push_back(p);
}

mitk::NodePredicateNot::~NodePredicateNot()
{
}

bool mitk::NodePredicateNot::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateNot: invalid node");


  // return the negation of the child predicate
  return !m_ChildPredicates.front()->CheckNode(node);
}

void mitk::NodePredicateNot::AddPredicate( const mitk::NodePredicateBase* p )
{
  if(!m_ChildPredicates.empty())
    m_ChildPredicates.clear();

  NodePredicateCompositeBase::AddPredicate(p);
}
