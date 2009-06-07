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

#include "mitkDataTreeNodeObject.h"

namespace mitk
{

DataTreeNodeObject::DataTreeNodeObject() :
  m_Node(0)
{

}

DataTreeNodeObject::DataTreeNodeObject(DataTreeNode::Pointer node) :
  m_Node(node)
{

}

DataTreeNode::Pointer DataTreeNodeObject::GetDataTreeNode() const
{
  return m_Node;
}

bool DataTreeNodeObject::operator==(const cherry::Object* obj) const
{
  if (const DataTreeNodeObject* other = dynamic_cast<const DataTreeNodeObject*>(obj))
  {
    return m_Node == other->m_Node;
  }

  return false;
}

}
