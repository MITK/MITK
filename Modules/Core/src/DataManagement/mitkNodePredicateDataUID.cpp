/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
