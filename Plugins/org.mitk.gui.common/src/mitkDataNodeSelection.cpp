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

#include "mitkDataNodeSelection.h"

#include "mitkDataNodeObject.h"

namespace mitk
{

DataNodeSelection::DataNodeSelection() :
  m_Selection(new ContainerType())
{

}

DataNodeSelection::DataNodeSelection(DataNode::Pointer node) :
  m_Selection(new ContainerType())
{
  DataNodeObject::Pointer obj(new DataNodeObject(node));
  m_Selection->push_back(obj);
}

DataNodeSelection::DataNodeSelection(const std::vector<DataNode::Pointer>& nodes) :
  m_Selection(new ContainerType())
{
  for (std::vector<DataNode::Pointer>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
  {
    DataNodeObject::Pointer obj(new DataNodeObject(*i));
    m_Selection->push_back(obj);
  }
}

berry::Object::Pointer DataNodeSelection::GetFirstElement() const
{
  if (m_Selection->empty())
    return berry::Object::Pointer();

  return *(m_Selection->begin());
}

berry::IStructuredSelection::iterator DataNodeSelection::Begin() const
{
  return m_Selection->begin();
}

berry::IStructuredSelection::iterator DataNodeSelection::End() const
{
  return m_Selection->end();
}

int DataNodeSelection::Size() const
{
  return m_Selection->size();
}

berry::IStructuredSelection::ContainerType::Pointer DataNodeSelection::ToVector() const
{
  return m_Selection;
}

std::list<DataNode::Pointer> DataNodeSelection::GetSelectedDataNodes() const
{
  std::list<DataNode::Pointer> selectedNodes;
  if(IsEmpty())
    return selectedNodes;

  DataNodeObject::Pointer dataNodeObject;
  DataNode::Pointer dataNode;

  for(iterator it = Begin(); it != End(); ++it)
  {
    dataNodeObject = it->Cast<DataNodeObject>();
    if(dataNodeObject.IsNotNull())
    {
      dataNode = dataNodeObject->GetDataNode();
      if(dataNode.IsNotNull())
        selectedNodes.push_back(dataNode);
    }
  }
  return selectedNodes;
}

bool DataNodeSelection::IsEmpty() const
{
  return m_Selection->empty();
}

bool DataNodeSelection::operator==(const berry::Object* obj) const
{
  if (const berry::IStructuredSelection* other = dynamic_cast<const berry::IStructuredSelection*>(obj))
  {
    return m_Selection == other->ToVector();
  }

  return false;
}

}
