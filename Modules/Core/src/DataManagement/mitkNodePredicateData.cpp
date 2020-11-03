/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateData.h"

#include "mitkDataNode.h"

mitk::NodePredicateData::NodePredicateData(mitk::BaseData *d) : NodePredicateBase()
{
  m_DataObject = d;
}

mitk::NodePredicateData::~NodePredicateData()
{
}

bool mitk::NodePredicateData::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateData: invalid node");

  return (node->GetData() == m_DataObject);
}
