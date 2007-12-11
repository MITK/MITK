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


mitk::NodePredicateNOT::NodePredicateNOT(mitk::NodePredicateBase& p)
: NodePredicateBase(), m_ChildPredicate(p)
{
}

mitk::NodePredicateNOT::~NodePredicateNOT()
{
}


bool mitk::NodePredicateNOT::CheckNode(const mitk::DataTreeNode* node) const
{
  if (!node)
    throw 1;  // Insert Exception Handling here

  // return the negation of the child predicate
  return !m_ChildPredicate.CheckNode(node);
}

