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

#include "mitkNodePredicateFirstLevel.h"

mitk::NodePredicateFirstLevel::NodePredicateFirstLevel(mitk::DataStorage* ds)
: NodePredicateBase(), m_DataStorage(ds)
{
}

mitk::NodePredicateFirstLevel::~NodePredicateFirstLevel()
{
}


bool mitk::NodePredicateFirstLevel::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateFirstLevel: invalid node");


  if(m_DataStorage.IsNull())
    throw std::invalid_argument("NodePredicateFirstLevel: DataStorage is invalid");

  mitk::DataStorage::SetOfObjects::ConstPointer list = m_DataStorage->GetSources(node, NULL, true);
  return (list->Size() == 0);
}
