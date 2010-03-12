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

#include "mitkNodePredicateOR.h"




mitk::NodePredicateOR::NodePredicateOR()
: NodePredicateCompositeBase()
{
}

mitk::NodePredicateOR::NodePredicateOR(const NodePredicateBase* p1, const NodePredicateBase* p2)
: NodePredicateCompositeBase()
{
  this->AddPredicate(p1);
  this->AddPredicate(p2);
}

mitk::NodePredicateOR::~NodePredicateOR()
{
}


bool mitk::NodePredicateOR::CheckNode(const DataNode* node) const
{
  if (m_ChildPredicates.empty())
    throw std::invalid_argument("NodePredicateOR: no child predicates available");

  if (node == NULL)
    throw std::invalid_argument("NodePredicateOR: invalid node");

  /* return the disjunction of the child predicate. If any predicate returns true, we return true too. Return false only if all child predicates return false */
  for (ChildPredicates::const_iterator it = m_ChildPredicates.begin(); it != m_ChildPredicates.end(); ++it)
    if ((*it)->CheckNode(node) == true)
      return true;
  return false; // none of the childs was true, so return false
}
