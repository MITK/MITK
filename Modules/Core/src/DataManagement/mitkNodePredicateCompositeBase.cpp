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

#include "mitkNodePredicateCompositeBase.h"

// for std::find
#include <algorithm>

mitk::NodePredicateCompositeBase::~NodePredicateCompositeBase()
{
}

void mitk::NodePredicateCompositeBase::AddPredicate( const NodePredicateBase* p )
{
  m_ChildPredicates.push_back(p);
}

void mitk::NodePredicateCompositeBase::RemovePredicate( const NodePredicateBase* p )
{
  m_ChildPredicates.remove(p);
}

mitk::NodePredicateCompositeBase::ChildPredicates mitk::NodePredicateCompositeBase::GetPredicates() const
{
  return m_ChildPredicates;
}
