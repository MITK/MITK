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

#include <mitkNodePredicateDataUID.h>
#include <mitkBaseData.h>
#include <mitkDataNode.h>

mitk::NodePredicateDataUID::NodePredicateDataUID(const Identifiable::UIDType &uid)
  : m_UID(uid)
{
}

mitk::NodePredicateDataUID::~NodePredicateDataUID()
{
}

bool mitk::NodePredicateDataUID::CheckNode(const mitk::DataNode *node) const
{
  if (nullptr != node)
  {
    auto data = node->GetData();

    if (nullptr != data)
    {
      return data->GetUID() == m_UID;
    }
  }

  return false;
}
