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

#include "mitkNodePredicateAtomic.h"

#include "mitkDataNode.h"


mitk::NodePredicateAtomic::NodePredicateAtomic(bool value)
: NodePredicateBase()
{
  m_Value = value;
}


mitk::NodePredicateAtomic::~NodePredicateAtomic()
{
}


bool mitk::NodePredicateAtomic::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateAtomic: invalid node");

  return m_Value;
}

mitk::NodePredicateAtomic* mitk::NodePredicateAtomic::False =
    new mitk::NodePredicateAtomic(false);

mitk::NodePredicateAtomic* mitk::NodePredicateAtomic::True =
    new mitk::NodePredicateAtomic(true);
