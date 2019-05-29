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

#include "QmitkDataStorageTreeModelInternalItem.h"

#include "QmitkNodeDescriptorManager.h"
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkMimeTypes.h>

QmitkDataStorageTreeModelInternalItem::QmitkDataStorageTreeModelInternalItem(mitk::DataNode *_DataNode, QmitkDataStorageTreeModelInternalItem *_Parent)
  : m_Parent(_Parent), m_DataNode(_DataNode)
{
  if (m_Parent)
    m_Parent->AddChild(this);
}

QmitkDataStorageTreeModelInternalItem::~QmitkDataStorageTreeModelInternalItem()
{
  if (m_Parent)
    m_Parent->RemoveChild(this);
}

void QmitkDataStorageTreeModelInternalItem::Delete()
{
  while (m_Children.size() > 0)
    delete m_Children.back();

  delete this;
}

QmitkDataStorageTreeModelInternalItem *QmitkDataStorageTreeModelInternalItem::Find(const mitk::DataNode *_DataNode) const
{
  QmitkDataStorageTreeModelInternalItem *item = nullptr;
  if (_DataNode)
  {
    if (m_DataNode == _DataNode)
      item = const_cast<QmitkDataStorageTreeModelInternalItem *>(this);
    else
    {
      for (std::vector<QmitkDataStorageTreeModelInternalItem *>::const_iterator it = m_Children.begin(); it != m_Children.end(); ++it)
      {
        if (item)
          break;
        item = (*it)->Find(_DataNode);
      }
    }
  }
  return item;
}

int QmitkDataStorageTreeModelInternalItem::IndexOfChild(const QmitkDataStorageTreeModelInternalItem *item) const
{
  std::vector<QmitkDataStorageTreeModelInternalItem *>::const_iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  return it != m_Children.end() ? std::distance(m_Children.begin(), it) : -1;
}

QmitkDataStorageTreeModelInternalItem *QmitkDataStorageTreeModelInternalItem::GetChild(int index) const
{
  return (m_Children.size() > 0 && index >= 0 && index < (int)m_Children.size()) ? m_Children.at(index) : 0;
}

void QmitkDataStorageTreeModelInternalItem::AddChild(QmitkDataStorageTreeModelInternalItem *item)
{
  this->InsertChild(item);
}

void QmitkDataStorageTreeModelInternalItem::RemoveChild(QmitkDataStorageTreeModelInternalItem *item)
{
  std::vector<QmitkDataStorageTreeModelInternalItem *>::iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  if (it != m_Children.end())
  {
    m_Children.erase(it);
    item->SetParent(0);
  }
}

int QmitkDataStorageTreeModelInternalItem::GetChildCount() const
{
  return m_Children.size();
}

int QmitkDataStorageTreeModelInternalItem::GetIndex() const
{
  if (m_Parent)
    return m_Parent->IndexOfChild(this);

  return 0;
}

QmitkDataStorageTreeModelInternalItem *QmitkDataStorageTreeModelInternalItem::GetParent() const
{
  return m_Parent;
}

mitk::DataNode::Pointer QmitkDataStorageTreeModelInternalItem::GetDataNode() const
{
  return m_DataNode.Lock();
}

void QmitkDataStorageTreeModelInternalItem::InsertChild(QmitkDataStorageTreeModelInternalItem *item, int index)
{
  std::vector<QmitkDataStorageTreeModelInternalItem *>::iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  if (it == m_Children.end())
  {
    if (m_Children.size() > 0 && index >= 0 && index < (int)m_Children.size())
    {
      it = m_Children.begin();
      std::advance(it, index);
      m_Children.insert(it, item);
    }
    else
      m_Children.push_back(item);

    // add parent if necessary
    if (item->GetParent() != this)
      item->SetParent(this);
  }
}

std::vector<QmitkDataStorageTreeModelInternalItem *> QmitkDataStorageTreeModelInternalItem::GetChildren() const
{
  return m_Children;
}

void QmitkDataStorageTreeModelInternalItem::SetParent(QmitkDataStorageTreeModelInternalItem *_Parent)
{
  m_Parent = _Parent;
  if (m_Parent)
    m_Parent->AddChild(this);
}
