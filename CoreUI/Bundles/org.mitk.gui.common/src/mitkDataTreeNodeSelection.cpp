/*=========================================================================

 Program:   BlueBerry Platform
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

#include "mitkDataTreeNodeSelection.h"

#include "mitkDataTreeNodeObject.h"

namespace mitk
{

DataTreeNodeSelection::DataTreeNodeSelection() :
  m_Selection(new ContainerType())
{

}

DataTreeNodeSelection::DataTreeNodeSelection(DataTreeNode::Pointer node) :
  m_Selection(new ContainerType())
{
  DataTreeNodeObject::Pointer obj(new DataTreeNodeObject(node));
  m_Selection->push_back(obj);
}

DataTreeNodeSelection::DataTreeNodeSelection(const std::vector<DataTreeNode::Pointer>& nodes) :
  m_Selection(new ContainerType())
{
  for (std::vector<DataTreeNode::Pointer>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
  {
    DataTreeNodeObject::Pointer obj(new DataTreeNodeObject(*i));
    m_Selection->push_back(obj);
  }
}

berry::Object::Pointer DataTreeNodeSelection::GetFirstElement() const
{
  if (m_Selection->empty())
    return berry::Object::Pointer();

  return *(m_Selection->begin());
}

berry::IStructuredSelection::iterator DataTreeNodeSelection::Begin() const
{
  return m_Selection->begin();
}

berry::IStructuredSelection::iterator DataTreeNodeSelection::End() const
{
  return m_Selection->end();
}

int DataTreeNodeSelection::Size() const
{
  return m_Selection->size();
}

berry::IStructuredSelection::ContainerType::Pointer DataTreeNodeSelection::ToVector() const
{
  return m_Selection;
}

bool DataTreeNodeSelection::IsEmpty() const
{
  return m_Selection->empty();
}

bool DataTreeNodeSelection::operator==(const berry::Object* obj) const
{
  if (const berry::IStructuredSelection* other = dynamic_cast<const berry::IStructuredSelection*>(obj))
  {
    return m_Selection == other->ToVector();
  }

  return false;
}

}
