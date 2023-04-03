/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowDataNodeTableModel.h"

// mitk core
#include <mitkRenderingManager.h>

// qt widgets module
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkMimeTypes.h>
#include <QmitkNodeDescriptorManager.h>
#include <QmitkStyleManager.h>

#include <QIcon>
#include <itkCommand.h>

QmitkRenderWindowDataNodeTableModel::QmitkRenderWindowDataNodeTableModel(QObject* parent /*= nullptr*/)
  : QAbstractItemModel(parent)
{
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();

  m_VisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg"));
  m_InvisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg"));
  m_ArrowIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/reset.svg"));
  m_TimesIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/times.svg"));
}

void QmitkRenderWindowDataNodeTableModel::UpdateModelData()
{
  auto baseRenderer = m_BaseRenderer.Lock();

  auto greaterThan = [&baseRenderer](const mitk::DataNode* dataNodeLeft, const mitk::DataNode* dataNodeRight)
  {
    int layerLeft = -1;
    int layerRight = -1;

    bool layerLeftFound = dataNodeLeft->GetIntProperty("layer", layerLeft, baseRenderer);
    bool layerRightFound = dataNodeRight->GetIntProperty("layer", layerRight, baseRenderer);

    if (layerLeftFound && layerRightFound)
    {
      return layerLeft > layerRight;
    }

    return true;
  };

  // sort node selection
  beginResetModel();
  std::sort(m_CurrentSelection.begin(), m_CurrentSelection.end(), greaterThan);
  endResetModel();

  emit ModelUpdated();
}

void QmitkRenderWindowDataNodeTableModel::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_RenderWindowLayerController->SetDataStorage(dataStorage);
}

void QmitkRenderWindowDataNodeTableModel::SetCurrentRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer == baseRenderer)
  {
    // resetting the same base renderer does nothing
    return;
  }

  m_BaseRenderer = baseRenderer;

  // update the model, since a new base renderer could have set the relevant node-properties differently
  this->UpdateModelData();
}

mitk::BaseRenderer::Pointer QmitkRenderWindowDataNodeTableModel::GetCurrentRenderer() const
{
  return m_BaseRenderer.Lock();
}

void QmitkRenderWindowDataNodeTableModel::SetCurrentSelection(NodeList selectedNodes)
{
  m_CurrentSelection = selectedNodes;

  // update the model: sort the current internal selection
  this->UpdateModelData();
}

QmitkRenderWindowDataNodeTableModel::NodeList QmitkRenderWindowDataNodeTableModel::GetCurrentSelection() const
{
  return m_CurrentSelection;
}

QModelIndex QmitkRenderWindowDataNodeTableModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkRenderWindowDataNodeTableModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

int QmitkRenderWindowDataNodeTableModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_CurrentSelection.size());
}

int QmitkRenderWindowDataNodeTableModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 4;
}

Qt::ItemFlags QmitkRenderWindowDataNodeTableModel::flags(const QModelIndex &index) const
{
  if (this != index.model())
  {
    return Qt::NoItemFlags;
  }

  if (index.isValid())
  {
    if (index.column() == 0)
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable |
             Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }
  else
  {
    return Qt::ItemIsDropEnabled;
  }
}

QVariant QmitkRenderWindowDataNodeTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || this != index.model())
  {
    return QVariant();
  }

  if (index.row() < 0 || index.row() >= static_cast<int>(m_CurrentSelection.size()))
  {
    return QVariant();
  }

  mitk::DataNode* dataNode = m_CurrentSelection.at(index.row());

  if (role == QmitkDataNodeRole)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }
  
  if (role == QmitkDataNodeRawPointerRole)
  {
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
  }

  if (index.column() == 0) // data node information column
  {
    QString nodeName = QString::fromStdString(dataNode->GetName());
    if (nodeName.isEmpty())
    {
      nodeName = "unnamed";
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      return nodeName;
    }

    if (role == Qt::ToolTipRole)
    {
      return QVariant("Name of the data node.");
    }

    if (role == Qt::DecorationRole)
    {
      QmitkNodeDescriptor* nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(dataNode);
      return QVariant(nodeDescriptor->GetIcon(dataNode));
    }
  }

  if (index.column() == 1) // node visibility column
  {
    if (role == Qt::DecorationRole)
    {
      auto baseRenderer = m_BaseRenderer.Lock();
      bool visibility = false;
      dataNode->GetVisibility(visibility, baseRenderer);

      return visibility ? QVariant(m_VisibleIcon) : QVariant(m_InvisibleIcon);
    }

    if (role == Qt::EditRole)
    {
      auto baseRenderer = m_BaseRenderer.Lock();
      bool visibility = false;
      dataNode->GetVisibility(visibility, baseRenderer);

      return QVariant(visibility);
    }
  }

  if (index.column() == 2)  // reset geometry column
  {
    if (role == Qt::DecorationRole)
    {
      return QVariant(m_ArrowIcon);
    }
  }

  if (index.column() == 3)  // remove node column
  {
    if (role == Qt::DecorationRole)
    {
      return QVariant(m_TimesIcon);
    }
  }

  return QVariant();
}

bool QmitkRenderWindowDataNodeTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || this != index.model())
  {
    return false;
  }

  if (index.row() < 0 || index.row() >= static_cast<int>(m_CurrentSelection.size()))
  {
    return false;
  }

  mitk::DataNode* dataNode = m_CurrentSelection.at(index.row());

  if (index.column() == 0) // data node information column
  {
    if (role == Qt::EditRole && !value.toString().isEmpty())
    {
      dataNode->SetName(value.toString().toStdString());
      emit dataChanged(index, index);
      return true;
    }
  }

  if (index.column() == 1) // data node visibility column
  {
    if (role == Qt::EditRole)
    {
      auto baseRenderer = m_BaseRenderer.Lock();
      bool visibility = value.toBool();
      dataNode->SetVisibility(visibility, baseRenderer);
      
      if (baseRenderer.IsNotNull())
      {
        // Explicitly request an update since a renderer-specific property change does not mark the node as modified.
        // see https://phabricator.mitk.org/T22322
        mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
      }

      emit dataChanged(index, index);
      return true;
    }
  }

  return false;
}

Qt::DropActions QmitkRenderWindowDataNodeTableModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions QmitkRenderWindowDataNodeTableModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList QmitkRenderWindowDataNodeTableModel::mimeTypes() const
{
  QStringList types = QAbstractItemModel::mimeTypes();
  types << QmitkMimeTypes::DataNodePtrs;
  return types;
}

QMimeData* QmitkRenderWindowDataNodeTableModel::mimeData(const QModelIndexList& indexes) const
{
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  for (const auto& index : indexes)
  {
    if (index.isValid())
    {
      auto dataNode = data(index, QmitkDataNodeRawPointerRole).value<mitk::DataNode*>();
      stream << reinterpret_cast<quintptr>(dataNode);
    }
  }

  mimeData->setData(QmitkMimeTypes::DataNodePtrs, encodedData);
  return mimeData;
}

bool QmitkRenderWindowDataNodeTableModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& parent)
{
  if (action == Qt::IgnoreAction)
  {
    return true;
  }

  if (!data->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    return false;
  }

  if (parent.isValid())
  {
    auto baseRenderer = m_BaseRenderer.Lock();
    int layer = -1;
    auto dataNode = this->data(parent, QmitkDataNodeRawPointerRole).value<mitk::DataNode*>();
    if (nullptr != dataNode)
    {
      dataNode->GetIntProperty("layer", layer, baseRenderer);
    }

    auto dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
    for (const auto& dataNode : qAsConst(dataNodeList))
    {
      m_RenderWindowLayerController->MoveNodeToPosition(dataNode, layer, baseRenderer);
    }

    this->UpdateModelData();

    return true;
  }

  return false;
}
