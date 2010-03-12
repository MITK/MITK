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

#include "mitkNodePredicateNOT.h"


mitk::NodePredicateNOT::NodePredicateNOT(const mitk::NodePredicateBase* p)
{
  m_ChildPredicates.push_back(p);
}

mitk::NodePredicateNOT::~NodePredicateNOT()
{
}

bool mitk::NodePredicateNOT::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateNOT: invalid node");


  // return the negation of the child predicate
  return !m_ChildPredicates.front()->CheckNode(node);
}

void mitk::NodePredicateNOT::AddPredicate( const mitk::NodePredicateBase* p )
{
  if(!m_ChildPredicates.empty())
    m_ChildPredicates.clear();

  NodePredicateCompositeBase::AddPredicate(p);
}
