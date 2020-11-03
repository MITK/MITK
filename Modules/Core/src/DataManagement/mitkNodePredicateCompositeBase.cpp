/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateCompositeBase.h"

// for std::find
#include <algorithm>

mitk::NodePredicateCompositeBase::~NodePredicateCompositeBase()
{
}

void mitk::NodePredicateCompositeBase::AddPredicate(const NodePredicateBase *p)
{
  m_ChildPredicates.push_back(p);
}

void mitk::NodePredicateCompositeBase::RemovePredicate(const NodePredicateBase *p)
{
  m_ChildPredicates.remove(p);
}

mitk::NodePredicateCompositeBase::ChildPredicates mitk::NodePredicateCompositeBase::GetPredicates() const
{
  return m_ChildPredicates;
}
