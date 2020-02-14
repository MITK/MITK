/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataNodeObject.h"

namespace mitk
{

DataNodeObject::DataNodeObject() :
  m_Node(nullptr)
{

}

DataNodeObject::DataNodeObject(DataNode::Pointer node) :
  m_Node(node)
{

}

DataNode::Pointer DataNodeObject::GetDataNode() const
{
  return m_Node;
}

bool DataNodeObject::operator==(const berry::Object* obj) const
{
  if (const DataNodeObject* other = dynamic_cast<const DataNodeObject*>(obj))
  {
    return m_Node == other->m_Node;
  }

  return false;
}

}
