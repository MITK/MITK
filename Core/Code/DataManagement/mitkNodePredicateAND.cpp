/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNodePredicateAND.h"


mitk::NodePredicateAND::NodePredicateAND()
: NodePredicateCompositeBase()
{
}


mitk::NodePredicateAND::NodePredicateAND(const NodePredicateBase* p1, const NodePredicateBase* p2)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
}


mitk::NodePredicateAND::NodePredicateAND(const NodePredicateBase* p1, const NodePredicateBase* p2, const NodePredicateBase* p3)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
  this->AddPredicate(p3);
}


mitk::NodePredicateAND::~NodePredicateAND()
{
}


bool mitk::NodePredicateAND::CheckNode(const mitk::DataTreeNode* node) const
{
  if (m_ChildPredicates.empty())
    throw std::invalid_argument("NodePredicateAND: no child predicates available");

  if (node == NULL)
    throw std::invalid_argument("NodePredicateAND: invalid node");

  // return the conjunction of the child predicate. If any predicate returns false, we return false too
  for (ChildPredicates::const_iterator it = m_ChildPredicates.begin();  ( it != m_ChildPredicates.end() ); ++it)
    if ((*it)->CheckNode(node) == false)
      return false;   // if one element of the conjunction is false, the whole conjunction gets false
  return true;  // none of the childs was false, so return true
}
