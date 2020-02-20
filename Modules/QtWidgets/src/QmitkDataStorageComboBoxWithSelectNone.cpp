/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataStorageComboBoxWithSelectNone.h"
#include <QDebug>

const QString QmitkDataStorageComboBoxWithSelectNone::ZERO_ENTRY_STRING = "--";

QmitkDataStorageComboBoxWithSelectNone::QmitkDataStorageComboBoxWithSelectNone(QWidget* parent, bool autoSelectNewNodes)
  : QmitkDataStorageComboBox(parent, autoSelectNewNodes)
  , m_CurrentPath("")
{
}

QmitkDataStorageComboBoxWithSelectNone::QmitkDataStorageComboBoxWithSelectNone(mitk::DataStorage* dataStorage,
                                                                               const mitk::NodePredicateBase* predicate,
                                                                               QWidget* parent,
                                                                               bool autoSelectNewNodes)
  : QmitkDataStorageComboBox(dataStorage, predicate, parent, autoSelectNewNodes)
{
}

QmitkDataStorageComboBoxWithSelectNone::~QmitkDataStorageComboBoxWithSelectNone()
{
}

int QmitkDataStorageComboBoxWithSelectNone::Find(const mitk::DataNode* dataNode) const
{
  int index = QmitkDataStorageComboBox::Find(dataNode);
  if (index != -1)
  {
    index += 1;
  }
  return index;
}

mitk::DataNode::Pointer QmitkDataStorageComboBoxWithSelectNone::GetNode(int index) const
{
  mitk::DataNode::Pointer result = nullptr;

  if (this->HasIndex(index))
  {
    if (index != 0)
    {
      result = m_Nodes.at(index - 1);
    }
  }
  return result;
}

mitk::DataNode::Pointer QmitkDataStorageComboBoxWithSelectNone::GetSelectedNode() const
{
  return this->GetNode(this->currentIndex());
}

void QmitkDataStorageComboBoxWithSelectNone::SetSelectedNode(const mitk::DataNode::Pointer& node)
{
  int index = Find(node);
  if (index == -1)
  {
    // didn't find it, so set the value to 0.
    index = 0;
  }

  this->setCurrentIndex(index);
}

void QmitkDataStorageComboBoxWithSelectNone::RemoveNode(int index)
{
  if(index > 0 && this->HasIndex(index))
  {
    RemoveNodeAndPropertyLists(index - 1);
    // remove node name from combobox
    this->removeItem(index);
  }
}

void QmitkDataStorageComboBoxWithSelectNone::SetNode(int index, const mitk::DataNode* dataNode)
{
  if (index > 0 && this->HasIndex(index))
  {
    // if node is identical, we only update the name in the QComboBoxItem
    if (dataNode == m_Nodes.at(index - 1))
    {
      this->setItemText(index, QString::fromStdString(dataNode->GetName()));
    }
    else
    {
      QmitkDataStorageComboBox::InsertNode(index - 1, dataNode);
    }
  }
}

bool QmitkDataStorageComboBoxWithSelectNone::HasIndex(unsigned int index) const
{
  return (m_Nodes.size() > 0 && index <= m_Nodes.size());
}

void QmitkDataStorageComboBoxWithSelectNone::InsertNode(int index, const mitk::DataNode* dataNode)
{
  if (index != 0)
  {
    QmitkDataStorageComboBox::InsertNode(index - 1, dataNode);
  }
}

void QmitkDataStorageComboBoxWithSelectNone::Reset()
{
  QmitkDataStorageComboBox::Reset();
  this->insertItem(0, ZERO_ENTRY_STRING);
}

void QmitkDataStorageComboBoxWithSelectNone::SetZeroEntryText(const QString& zeroEntryString)
{
  this->setItemText(0, zeroEntryString);
  this->setCurrentIndex(0);
}

QString QmitkDataStorageComboBoxWithSelectNone::currentValue() const
{
  return m_CurrentPath;
}

void QmitkDataStorageComboBoxWithSelectNone::setCurrentValue(const QString& path)
{
  m_CurrentPath = path;
}
