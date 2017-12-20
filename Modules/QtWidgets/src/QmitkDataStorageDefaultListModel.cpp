/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <QmitkDataStorageDefaultListModel.h>

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

QmitkDataStorageDefaultListModel::QmitkDataStorageDefaultListModel(QObject *parent)
  : m_DataStorage(nullptr)
  , m_NodePredicate(nullptr)
{
  // nothing here
}

void QmitkDataStorageDefaultListModel::SetDataStorage(mitk::DataStorage* dataStorage)
{
  /* TEMPORARY
  if (m_DataStorage == dataStorage)
  {
    return;
  }
  */
  m_DataStorage = dataStorage;

  mitk::DataStorage::SetOfObjects::ConstPointer dataNodes;
  if (m_DataStorage != nullptr)
  {
    if (m_NodePredicate != nullptr)
    {
      dataNodes = m_DataStorage->GetSubset(m_NodePredicate);
    }
    else
    {
      dataNodes = m_DataStorage->GetAll();
    }
  }

  // update the model, so that it will be filled with the nodes of the new data storage
  beginResetModel();
  m_DataNodes.clear();

  // add all (filtered) nodes to the vector of nodes
  if (dataNodes != nullptr)
  {
    for (auto& node : *dataNodes)
    {
      m_DataNodes.push_back(node);
    }
  }
  endResetModel();
}

void QmitkDataStorageDefaultListModel::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  m_NodePredicate = nodePredicate;
}

QModelIndex QmitkDataStorageDefaultListModel::index(int row, int column, const QModelIndex &parent) const
{
  bool hastIndex = hasIndex(row, column, parent);
  if (hastIndex)
  {
    return createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkDataStorageDefaultListModel::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

int QmitkDataStorageDefaultListModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_DataNodes.size();
}

int QmitkDataStorageDefaultListModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 1;
}

QVariant QmitkDataStorageDefaultListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if(index.row() < 0 || index.row() >= m_DataNodes.size())
  {
    return QVariant();
  }

  mitk::DataNode::Pointer dataNode = m_DataNodes.at(index.row());
  if (Qt::DisplayRole == role)
  {
    return QVariant(QString::fromStdString(dataNode->GetName()));
  }
  else if (QmitkDataNodeRole == role)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(dataNode);
  }

  return QVariant();
}

QVariant QmitkDataStorageDefaultListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant(tr("Nodes"));
}

Qt::ItemFlags QmitkDataStorageDefaultListModel::flags(const QModelIndex &index) const
{
  if (index.isValid())
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return Qt::NoItemFlags;
}
