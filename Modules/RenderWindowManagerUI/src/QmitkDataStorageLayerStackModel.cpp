/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageLayerStackModel.h>

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

QmitkDataStorageLayerStackModel::QmitkDataStorageLayerStackModel(QObject* parent/* = nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
{
  // nothing here
}

void QmitkDataStorageLayerStackModel::DataStorageChanged()
{
  UpdateModelData();
}

void QmitkDataStorageLayerStackModel::NodePredicateChanged()
{
  UpdateModelData();
}

void QmitkDataStorageLayerStackModel::NodeAdded(const mitk::DataNode* /*node*/)
{
  // nothing here; layers (nodes) are only added after button click
}

void QmitkDataStorageLayerStackModel::NodeChanged(const mitk::DataNode* /*node*/)
{
  UpdateModelData();
}

void QmitkDataStorageLayerStackModel::NodeRemoved(const mitk::DataNode* /*node*/)
{
  UpdateModelData();
}

void QmitkDataStorageLayerStackModel::SetCurrentRenderer(const std::string& renderWindowName)
{
  if (!m_DataStorage.IsExpired())
  {
    m_BaseRenderer = mitk::BaseRenderer::GetByName(renderWindowName);
    UpdateModelData();
  }
}

mitk::BaseRenderer* QmitkDataStorageLayerStackModel::GetCurrentRenderer() const
{
  return m_BaseRenderer.Lock();
}

QModelIndex QmitkDataStorageLayerStackModel::index(int row, int column, const QModelIndex& parent) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkDataStorageLayerStackModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

Qt::ItemFlags QmitkDataStorageLayerStackModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = Qt::NoItemFlags;
  if (index.isValid() && index.model() == this)
  {
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (0 == index.column())
    {
      flags |= Qt::ItemIsUserCheckable;
    }
  }

  return flags;
}

QVariant QmitkDataStorageLayerStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (0 == section)
    {
      return QVariant("Visibility");
    }
    else if (1 == section)
    {
      return QVariant("Data node");
    }
  }
  return QVariant();
}

int QmitkDataStorageLayerStackModel::rowCount(const QModelIndex& parent/* = QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_TempLayerStack.size());
}

int QmitkDataStorageLayerStackModel::columnCount(const QModelIndex& parent/* = QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 2;
}

QVariant QmitkDataStorageLayerStackModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.model() != this)
  {
    return QVariant();
  }

  if ((index.row()) < 0 || index.row() >= static_cast<int>(m_TempLayerStack.size()))
  {
    return QVariant();
  }

  RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_TempLayerStack.begin();
  std::advance(layerStackIt, index.row());
  mitk::DataNode* dataNode = layerStackIt->second;
  if (Qt::CheckStateRole == role && 0 == index.column())
  {
    bool visibility = false;
    dataNode->GetVisibility(visibility, m_BaseRenderer.Lock());
    if (visibility)
    {
      return Qt::Checked;
    }
    else
    {
      return Qt::Unchecked;
    }
  }
  else if (Qt::DisplayRole == role && 1 == index.column())
  {
    return QVariant(QString::fromStdString(dataNode->GetName()));
  }
  else if (Qt::ToolTipRole == role)
  {
    if (0 == index.column())
    {
      return QVariant("Show/hide data node.");
    }
    else if (1 == index.column())
    {
      return QVariant("Name of the data node.");
    }
  }
  else if (QmitkDataNodeRole == role)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }
  else if (QmitkDataNodeRawPointerRole == role)
  {
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
  }

  return QVariant();
}

bool QmitkDataStorageLayerStackModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
  if (!index.isValid() || index.model() != this)
  {
    return false;
  }

  if ((index.row()) < 0 || index.row() >= static_cast<int>(m_TempLayerStack.size()))
  {
    return false;
  }

  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNotNull())
  {
    RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_TempLayerStack.begin();
    std::advance(layerStackIt, index.row());
    mitk::DataNode* dataNode = layerStackIt->second;
    if (Qt::CheckStateRole == role)
    {
      Qt::CheckState newCheckState = static_cast<Qt::CheckState>(value.toInt());

      if (Qt::PartiallyChecked == newCheckState || Qt::Checked == newCheckState)
      {
        dataNode->SetVisibility(true, baseRenderer);
      }
      else
      {
        dataNode->SetVisibility(false, baseRenderer);
      }
      emit dataChanged(index, index);
      mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
      return true;
    }
  }

  return false;
}

void QmitkDataStorageLayerStackModel::UpdateModelData()
{
  // update the model, so that the table will be filled with the nodes according to the current
  // data storage and base renderer
  beginResetModel();
  // get the current layer stack of the given base renderer
  m_TempLayerStack = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage.Lock(), m_BaseRenderer.Lock());
  endResetModel();
}
