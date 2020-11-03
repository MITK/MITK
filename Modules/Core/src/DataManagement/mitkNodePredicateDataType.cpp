/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateDataType.h"

#include "mitkBaseData.h"
#include "mitkDataNode.h"

mitk::NodePredicateDataType::NodePredicateDataType(const char *datatype) : NodePredicateBase()
{
  if (datatype == nullptr)
    throw std::invalid_argument("NodePredicateDataType: invalid datatype");

  m_ValidDataType = datatype;
}

mitk::NodePredicateDataType::~NodePredicateDataType()
{
}

bool mitk::NodePredicateDataType::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateDataType: invalid node");

  mitk::BaseData *data = node->GetData();

  if (data == nullptr)
    return false; // or should we check if m_ValidDataType == "nullptr" so that nodes without data can be requested?

  return (m_ValidDataType.compare(data->GetNameOfClass()) == 0); // return true if data type matches
}
