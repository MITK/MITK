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

// render window manager UI module
#include "QmitkRenderWindowDataModel.h"

#include "QmitkCustomVariants.h"

QmitkRenderWindowDataModel::QmitkRenderWindowDataModel(QObject* parent /*= nullptr*/)
  : QAbstractTableModel(parent)
{
  // nothing here
}

QmitkRenderWindowDataModel::~QmitkRenderWindowDataModel()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->ChangedNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
    m_DataStorage->RemoveNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
  }
}

int QmitkRenderWindowDataModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_TempLayerStack.size());
}

int QmitkRenderWindowDataModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 2;
}

QVariant QmitkRenderWindowDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if ((index.row()) < static_cast<int>(m_TempLayerStack.size()))
  {
    RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_TempLayerStack.begin();
    std::advance(layerStackIt, index.row());
    mitk::DataNode* dataNode = layerStackIt->second;
    if (Qt::CheckStateRole == role && 0 == index.column())
    {
      bool visibility = false;
      dataNode->GetVisibility(visibility, m_BaseRenderer);
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
    else if (Qt::UserRole == role)
    {
      // user role always returns a reference to the data node,
      // which can be used to modify the data node in the data storage
      return QVariant::fromValue(dataNode);
    }
  }
  return QVariant();
}

Qt::ItemFlags QmitkRenderWindowDataModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (0 == index.column())
  {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

QVariant QmitkRenderWindowDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool QmitkRenderWindowDataModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
  if (!index.isValid())
  {
    return false;
  }

  if ((index.row()) < static_cast<int>(m_TempLayerStack.size()))
  {
    RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_TempLayerStack.begin();
    std::advance(layerStackIt, index.row());
    mitk::DataNode* dataNode = layerStackIt->second;
    if (Qt::CheckStateRole == role)
    {
      Qt::CheckState newCheckState = static_cast<Qt::CheckState>(value.toInt());

      if (Qt::PartiallyChecked == newCheckState || Qt::Checked == newCheckState)
      {
        dataNode->SetVisibility(true, m_BaseRenderer);
      }
      else
      {
        dataNode->SetVisibility(false, m_BaseRenderer);
      }
      emit dataChanged(index, index);
      mitk::RenderingManager::GetInstance()->RequestUpdate(m_BaseRenderer->GetRenderWindow());
      return true;
    }
  }
  return false;
}

void QmitkRenderWindowDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // given data storage is a new data storage
    if (m_DataStorage.IsNotNull())
    {
      // remove listener from old data storage
      m_DataStorage->ChangedNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
      m_DataStorage->RemoveNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
    }
    // set the new data storage
    m_DataStorage = dataStorage;
    // register new data storage listener
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->ChangedNodeEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
      m_DataStorage->RemoveNodeEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindowDataModel, const mitk::DataNode*>(this, &QmitkRenderWindowDataModel::DataChanged));
    }
    DataChanged(nullptr);
  }
}

void QmitkRenderWindowDataModel::SetCurrentRenderer(std::string renderWindowName)
{
  m_BaseRenderer = mitk::BaseRenderer::GetByName(renderWindowName);
  if (m_DataStorage.IsNotNull())
  {
    DataChanged(nullptr);
  }
}

void QmitkRenderWindowDataModel::DataChanged(const mitk::DataNode* /*dataNode*/)
{
  if (m_BaseRenderer.IsNotNull())
  {
    // update the model, so that the table will be filled with the nodes information of the new data storage
    beginResetModel();
    // get the current layer stack of the given base renderer
    m_TempLayerStack = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, m_BaseRenderer, true);
    endResetModel();
  }
}
