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

#include "mitkNodePredicateDataType.h"

#include "mitkDataNode.h"
#include "mitkBaseData.h"

mitk::NodePredicateDataType::NodePredicateDataType(const char* datatype)
: NodePredicateBase()
{
  if (datatype == NULL)
    throw std::invalid_argument("NodePredicateDataType: invalid datatype");


  m_ValidDataType = datatype;
}

mitk::NodePredicateDataType::~NodePredicateDataType()
{
}


bool mitk::NodePredicateDataType::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateDataType: invalid node");


  mitk::BaseData* data = node->GetData();

  if (data == NULL)
    return false;  // or should we check if m_ValidDataType == "NULL" so that nodes without data can be requested?

  return ( m_ValidDataType.compare(data->GetNameOfClass()) == 0); // return true if data type matches
}
