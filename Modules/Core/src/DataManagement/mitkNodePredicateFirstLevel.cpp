/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateFirstLevel.h"

mitk::NodePredicateFirstLevel::NodePredicateFirstLevel(mitk::DataStorage *ds) : NodePredicateBase(), m_DataStorage(ds)
{
}

mitk::NodePredicateFirstLevel::~NodePredicateFirstLevel()
{
}

bool mitk::NodePredicateFirstLevel::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateFirstLevel: invalid node");

  if (m_DataStorage.IsExpired())
    throw std::invalid_argument("NodePredicateFirstLevel: DataStorage is invalid");

  mitk::DataStorage::SetOfObjects::ConstPointer list = m_DataStorage.Lock()->GetSources(node, nullptr, true);
  return (list->Size() == 0);
}
