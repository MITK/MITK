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

#include <mitkNodePredicateUID.h>
#include <mitkDataNode.h>

mitk::NodePredicateUID::NodePredicateUID(const Identifiable::UIDType &uid)
  : m_UID(uid)
{
}

mitk::NodePredicateUID::~NodePredicateUID()
{
}

bool mitk::NodePredicateUID::CheckNode(const mitk::DataNode *node) const
{
  return nullptr != node
    ? node->GetUID() == m_UID
    : false;
}
