/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkDataStorageComboBoxWithSelectNone.h"

const QString QmitkDataStorageComboBoxWithSelectNone::ZERO_ENTRY_STRING = "please select";

//-----------------------------------------------------------------------------
QmitkDataStorageComboBoxWithSelectNone::QmitkDataStorageComboBoxWithSelectNone(
    QWidget* parent,
    bool autoSelectNewNodes )
: QmitkDataStorageComboBox(parent, autoSelectNewNodes)
{
}


//-----------------------------------------------------------------------------
QmitkDataStorageComboBoxWithSelectNone::QmitkDataStorageComboBoxWithSelectNone(
    mitk::DataStorage* dataStorage,
    const mitk::NodePredicateBase* predicate,
    QWidget* parent, bool autoSelectNewNodes )
: QmitkDataStorageComboBox(dataStorage, predicate, parent, autoSelectNewNodes)
{
}


//-----------------------------------------------------------------------------
QmitkDataStorageComboBoxWithSelectNone::~QmitkDataStorageComboBoxWithSelectNone()
{
}


//-----------------------------------------------------------------------------
int QmitkDataStorageComboBoxWithSelectNone::Find( const mitk::DataNode* dataNode ) const
{
  int index = QmitkDataStorageComboBox::Find(dataNode);
  if (index != -1)
  {
    index += 1;
  }
  return index;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer QmitkDataStorageComboBoxWithSelectNone::GetNode( int index ) const
{
  mitk::DataNode::Pointer result = NULL;

  if (this->HasIndex(index))
  {
    if (index != 0)
    {
      result = m_Nodes.at(index - 1);
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer QmitkDataStorageComboBoxWithSelectNone::GetSelectedNode() const
{
  return this->GetNode(this->currentIndex());
}


//-----------------------------------------------------------------------------
void QmitkDataStorageComboBoxWithSelectNone::RemoveNode( int index )
{
  if(index > 0 && this->HasIndex(index))
  {

    // remove itk::Event observer
    mitk::DataNode* dataNode = m_Nodes.at(index - 1);

    // get name property first
    mitk::BaseProperty* nameProperty = dataNode->GetProperty("name");

    // if prop exists remove modified listener
    if(nameProperty)
    {
      nameProperty->RemoveObserver(m_NodesModifiedObserverTags[index-1]);

      // remove name property map
      m_PropertyToNode.erase(dataNode);
    }

    // then remove delete listener on the node itself
    dataNode->RemoveObserver(m_NodesDeleteObserverTags[index-1]);

    // remove observer tags from lists
    m_NodesModifiedObserverTags.erase(m_NodesModifiedObserverTags.begin()+index-1);
    m_NodesDeleteObserverTags.erase(m_NodesDeleteObserverTags.begin()+index-1);

    // remove node name from combobox
    this->removeItem(index);

    // remove node from node vector
    m_Nodes.erase(m_Nodes.begin()+index-1);
  }
}


//-----------------------------------------------------------------------------
void QmitkDataStorageComboBoxWithSelectNone::SetNode(int index, const mitk::DataNode* dataNode)
{
  if(index > 0 && this->HasIndex(index))
  {
    QmitkDataStorageComboBox::InsertNode(index - 1, dataNode);
  }
}


//-----------------------------------------------------------------------------
bool QmitkDataStorageComboBoxWithSelectNone::HasIndex(unsigned int index) const
{
  return (m_Nodes.size() > 0 && index <= m_Nodes.size());
}


//-----------------------------------------------------------------------------
void QmitkDataStorageComboBoxWithSelectNone::InsertNode(int index, const mitk::DataNode* dataNode)
{
  if (index != 0)
  {
    QmitkDataStorageComboBox::InsertNode(index - 1, dataNode);
  }
}


//-----------------------------------------------------------------------------
void QmitkDataStorageComboBoxWithSelectNone::Reset()
{
  QmitkDataStorageComboBox::Reset();
  this->insertItem(0, ZERO_ENTRY_STRING);
}
