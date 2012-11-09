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

#include "mitkNodePredicateData.h"

#include "mitkDataNode.h"


mitk::NodePredicateData::NodePredicateData(mitk::BaseData* d)
: NodePredicateBase()
{
  m_DataObject = d;
}


mitk::NodePredicateData::~NodePredicateData()
{
}


bool mitk::NodePredicateData::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateData: invalid node");

  return (node->GetData() == m_DataObject);
}
