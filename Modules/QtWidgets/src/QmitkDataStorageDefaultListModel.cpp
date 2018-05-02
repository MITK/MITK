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
#include "QmitkNodeDescriptorManager.h"

QmitkDataStorageDefaultListModel::QmitkDataStorageDefaultListModel(QObject *parent) : QmitkAbstractDataStorageModel(parent)
{
}

void QmitkDataStorageDefaultListModel::DataStorageChanged()
{
  UpdateModelData();
}

void QmitkDataStorageDefaultListModel::NodePredicateChanged()
{
  UpdateModelData();
}

void QmitkDataStorageDefaultListModel::NodeAdded(const mitk::DataNode* /*node*/)
{
  UpdateModelData();
}

void QmitkDataStorageDefaultListModel::NodeChanged(const mitk::DataNode* /*node*/)
{
  // nothing here, since the "'NodeChanged'-event is currently sent far too often
  //UpdateModelData();
}

void QmitkDataStorageDefaultListModel::NodeRemoved(const mitk::DataNode* /*node*/)
{
  UpdateModelData();
}

QModelIndex QmitkDataStorageDefaultListModel::index(int row, int column, const QModelIndex &parent) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkDataStorageDefaultListModel::parent(const QModelIndex &/*child*/) const
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
  if (!index.isValid() || index.model() != this)
  {
    return QVariant();
  }

  if(index.row() < 0 || index.row() >= static_cast<int>(m_DataNodes.size()))
  {
    return QVariant();
  }

  mitk::DataNode::Pointer dataNode = m_DataNodes.at(index.row());
  QString nodeName = QString::fromStdString(dataNode->GetName());
  if (nodeName.isEmpty())
    nodeName = "unnamed";

  if (role == Qt::DisplayRole)
    return nodeName;
  else if (role == Qt::ToolTipRole)
    return nodeName;
  else if (role == Qt::DecorationRole)
  {
    QmitkNodeDescriptor *nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(dataNode);
    return nodeDescriptor->GetIcon(dataNode);
  }
  else if (role == QmitkDataNodeRole)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }
  else if (role == QmitkDataNodeRawPointerRole)
  {
    return QVariant::fromValue<mitk::DataNode *>(dataNode);
  }

  return QVariant();
}

QVariant QmitkDataStorageDefaultListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
  return QVariant(tr("Nodes"));
}

Qt::ItemFlags QmitkDataStorageDefaultListModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.model() == this)
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return Qt::NoItemFlags;
}

void QmitkDataStorageDefaultListModel::UpdateModelData()
{
  mitk::DataStorage::SetOfObjects::ConstPointer dataNodes;
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    if (m_NodePredicate.IsNotNull())
    {
      dataNodes = dataStorage->GetSubset(m_NodePredicate);
    }
    else
    {
      dataNodes = dataStorage->GetAll();
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
