/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNodePredicateProperty.h"
#include "mitkDataTreeNode.h"



mitk::NodePredicateProperty::NodePredicateProperty(const char* propertyName, mitk::BaseProperty* p)
: NodePredicateBase(), m_ValidProperty(p), m_ValidPropertyName(propertyName)
{
}

mitk::NodePredicateProperty::NodePredicateProperty(const char* propertyName)
: NodePredicateBase(), m_ValidProperty(NULL), m_ValidPropertyName(propertyName)
{
}

mitk::NodePredicateProperty::~NodePredicateProperty()
{
}


bool mitk::NodePredicateProperty::CheckNode(const mitk::DataTreeNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateProperty: invalid node");
  
  if (m_ValidPropertyName.empty())
    throw std::invalid_argument("NodePredicateProperty: invalid property name");

  // check, if any of the properties of node are equal to m_ValidProperty.
  if (m_ValidProperty.IsNull())
  //if (m_ValidProperty==NULL)
  {
    return (node->GetPropertyList()->GetProperty(m_ValidPropertyName.c_str()) != NULL); // search only for name
  }
  else
  {
    mitk::BaseProperty::Pointer p = node->GetPropertyList()->GetProperty(m_ValidPropertyName.c_str());
    if (p.IsNull())
      return false;
    return (*p == *m_ValidProperty); // search for name and property
  }
}
