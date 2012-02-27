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

#include "mitkDataNodeObject.h"

namespace mitk
{

DataNodeObject::DataNodeObject() :
  m_Node(0)
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
