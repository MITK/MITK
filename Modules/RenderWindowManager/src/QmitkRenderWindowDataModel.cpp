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

#include <QColor>
#include <QSize>

#include "QmitkRenderWindowDataModel.h"

// mitk
#include <mitkBaseRenderer.h>

QmitkRenderWindowDataModel::QmitkRenderWindowDataModel(QObject* parent)
  : QAbstractTableModel(parent)
{

}

int QmitkRenderWindowDataModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  std::map<RenderWindowName, DataNodeNamesVector>::const_iterator iterator = m_RenderWindowDataNodesMap.find(m_CurrentRenderWindowName);
  if (iterator == m_RenderWindowDataNodesMap.end())
  {
    return 0;
  }
  else
  {
    return iterator->second.size();
  }
}

int QmitkRenderWindowDataModel::columnCount(const QModelIndex &parent) const
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

  // find the list of data nodes of the given render window
  std::map<RenderWindowName, DataNodeNamesVector>::const_iterator iterator = m_RenderWindowDataNodesMap.find(m_CurrentRenderWindowName);
  if (iterator != m_RenderWindowDataNodesMap.end()
    && static_cast<size_t>(index.row()) < iterator->second.size())
  {
    const std::string &dataNodeName = iterator->second.at(iterator->second.size() - index.row() - 1);
    // different behavior for the columns
    switch (index.column())
    {
    case 0:
      if (role == Qt::CheckStateRole)
      {
        const mitk::DataNode* selectedDataNode = m_DataStorage->GetNamedNode(dataNodeName);
        const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_CurrentRenderWindowName);

        bool visibility = false;
        // TODO: use the current renderer
        selectedDataNode->GetVisibility(visibility, selectedRenderer);
        if (visibility)
        {
          return Qt::Checked;
        }
        else
        {
          return Qt::Unchecked;
        }
      }
      else if (role == Qt::ToolTipRole)
      {
        return QVariant("Show/hide data node.");
      }
      break;
    case 1:
      if (role == Qt::DisplayRole)
      {
        return QVariant(QString::fromStdString(dataNodeName));
      }
      else if (role == Qt::ToolTipRole)
      {
        return QVariant("Name of the data node.");
      }
    }
  }
  return QVariant();
}

Qt::ItemFlags QmitkRenderWindowDataModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (index.column() == 0)
  {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

QVariant QmitkRenderWindowDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (section == 0)
    {
      return QVariant("Visibility");
    }
    else if (section == 1)
    {
      return QVariant("Data node");
    }
  }
  return QVariant();
}

bool QmitkRenderWindowDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
  {
    return false;
  }

  // find the list of data nodes of the given render window
  std::map<RenderWindowName, DataNodeNamesVector>::const_iterator iterator = m_RenderWindowDataNodesMap.find(m_CurrentRenderWindowName);
  if (iterator != m_RenderWindowDataNodesMap.end()
    && static_cast<size_t>(index.row()) < iterator->second.size())
  {

    if (Qt::CheckStateRole == role)
    {
      Qt::CheckState newCheckState = static_cast<Qt::CheckState>(value.toInt());

      // retrieve the data node from the data storage
      const std::string &dataNodeName = iterator->second.at(iterator->second.size() - index.row() - 1);
      mitk::DataNode* selectedDataNode = m_DataStorage->GetNamedNode(dataNodeName);
      const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_CurrentRenderWindowName);

      if (Qt::PartiallyChecked == newCheckState || Qt::Checked == newCheckState)
      {
        selectedDataNode->SetVisibility(true, selectedRenderer);
      }
      else
      {
        selectedDataNode->SetVisibility(false, selectedRenderer);
      }
      emit dataChanged(index, index);
      mitk::RenderingManager::GetInstance()->RequestUpdate(selectedRenderer->GetRenderWindow());
      return true;
    }
  }
  return false;
}

void QmitkRenderWindowDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

void QmitkRenderWindowDataModel::SetCurrentRenderWindowName(std::string renderWindowName)
{
  m_CurrentRenderWindowName = renderWindowName;

  int numberOfRows = m_RenderWindowDataNodesMap[m_CurrentRenderWindowName].size();

  // notify view, so that the table will be filled with the nodes information of the newly selected renderer
  emit dataChanged(createIndex(0, 0), createIndex(numberOfRows, columnCount()));
}

void QmitkRenderWindowDataModel::InsertLayerNode(std::string dataNodeName, std::string renderer)
{
  // checking for the node list of the given renderer
  // checking with [] creates an empty 'DataNodeNamesVector' if the renderer is currently not used as a key
  DataNodeNamesVector &dataNodeNamesVector = m_RenderWindowDataNodesMap[renderer];
  beginInsertRows(QModelIndex(), 0, 0);
  dataNodeNamesVector.push_back(dataNodeName);
  endInsertRows();
}

void QmitkRenderWindowDataModel::MoveNodeUp(int rowIndex, std::string dataNodeName, std::string renderer)
{
  std::map<RenderWindowName, DataNodeNamesVector>::iterator iterator = m_RenderWindowDataNodesMap.find(renderer);
  if (iterator != m_RenderWindowDataNodesMap.end())
  {
    // swap item at 'dataNodeName' with the previous data node in the vector
    std::swap(iterator->second[rowIndex], iterator->second[rowIndex - 1]);
    emit dataChanged(createIndex(rowIndex, 1), createIndex(rowIndex - 1, 1));
  }
}

void QmitkRenderWindowDataModel::MoveNodeDown(int rowIndex, std::string dataNodeName, std::string renderer)
{
  std::map<RenderWindowName, DataNodeNamesVector>::iterator iterator = m_RenderWindowDataNodesMap.find(renderer);
  if (iterator != m_RenderWindowDataNodesMap.end())
  {
    // swap item at 'dataNodeName' with the next data node in the vector
    std::swap(iterator->second[rowIndex], iterator->second[rowIndex + 1]);
    emit dataChanged(createIndex(rowIndex, 1), createIndex(rowIndex + 1, 1));
  }
}
