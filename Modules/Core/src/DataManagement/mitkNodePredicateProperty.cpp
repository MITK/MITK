/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateProperty.h"
#include "mitkDataNode.h"

mitk::NodePredicateProperty::NodePredicateProperty(const char *propertyName,
                                                   mitk::BaseProperty *p,
                                                   const mitk::BaseRenderer *renderer)
  : NodePredicateBase(), m_ValidProperty(p), m_ValidPropertyName(propertyName), m_Renderer(renderer)
{
}

mitk::NodePredicateProperty::~NodePredicateProperty()
{
}

bool mitk::NodePredicateProperty::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    throw std::invalid_argument("NodePredicateProperty: invalid node");

  if (m_ValidPropertyName.empty())
    throw std::invalid_argument("NodePredicateProperty: invalid property name");

  // check, if any of the properties of node are equal to m_ValidProperty.
  if (m_ValidProperty.IsNull())
  // if (m_ValidProperty==nullptr)
  {
    return (node->GetProperty(m_ValidPropertyName.c_str(), m_Renderer) != nullptr); // search only for name
  }
  else
  {
    mitk::BaseProperty::Pointer p = node->GetProperty(m_ValidPropertyName.c_str(), m_Renderer);
    if (p.IsNull())
      return false;
    return (*p == *m_ValidProperty); // search for name and property
  }
}
