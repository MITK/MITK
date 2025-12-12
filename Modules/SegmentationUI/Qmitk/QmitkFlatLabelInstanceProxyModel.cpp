/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkFlatLabelInstanceProxyModel.h>
#include <QmitkMultiLabelTreeModel.h>

QmitkFlatLabelInstanceProxyModel::QmitkFlatLabelInstanceProxyModel(QObject* parent)
  : QAbstractProxyModel(parent)
{
}

void QmitkFlatLabelInstanceProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  auto labelModel = dynamic_cast<QmitkMultiLabelTreeModel*>(sourceModel);
  if (nullptr == labelModel)
    mitkThrow() << "Error. QmitkFlatLabelInstanceProxyModel only accepts QmitkMultiLabelTreeModel as source.";
  this->beginResetModel();
  QAbstractProxyModel::setSourceModel(sourceModel);
  endResetModel();

  if (!sourceModel)
    return;

  connect(labelModel, &QAbstractItemModel::modelReset, this, &QmitkFlatLabelInstanceProxyModel::OnSourceModelReset);
  connect(labelModel, &QmitkMultiLabelTreeModel::modelChanged, this, &QmitkFlatLabelInstanceProxyModel::OnSourceDataChanged);

  this->RebuildMapping();
}

void QmitkFlatLabelInstanceProxyModel::RebuildMapping()
{
  this->beginResetModel();
  m_SourceIndexes.clear();
  if (sourceModel())
    this->CollectAcceptedIndexes(QModelIndex());
  this->endResetModel();
}

void QmitkFlatLabelInstanceProxyModel::CollectAcceptedIndexes(const QModelIndex& parent)
{
  auto* src = sourceModel();
  if (!src)
    return;

  int rows = src->rowCount(parent);
  for (int r = 0; r < rows; ++r)
  {
    QModelIndex idx = src->index(r, 0, parent);
    if (!idx.isValid())
      continue;

    QVariant labelInstanceData = src->data(idx, QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceDataRole);

    if (labelInstanceData.isValid() && !labelInstanceData.isNull())
    {
      m_SourceIndexes.push_back(idx);
    }

    // continue recursively into children
    this->CollectAcceptedIndexes(idx);
  }
}

int QmitkFlatLabelInstanceProxyModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return m_SourceIndexes.size();
}

int QmitkFlatLabelInstanceProxyModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return sourceModel() ? sourceModel()->columnCount() : 0;
}

QModelIndex QmitkFlatLabelInstanceProxyModel::index(int row, int column, const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  if (row < 0 || row >= m_SourceIndexes.size())
    return QModelIndex();
  return this->createIndex(row, column);
}

QModelIndex QmitkFlatLabelInstanceProxyModel::parent(const QModelIndex& child) const
{
  Q_UNUSED(child);
  return QModelIndex();
}

QModelIndex QmitkFlatLabelInstanceProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid() || proxyIndex.row() >= m_SourceIndexes.size())
    return QModelIndex();
  return m_SourceIndexes[proxyIndex.row()];
}

QModelIndex QmitkFlatLabelInstanceProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
  int row = m_SourceIndexes.indexOf(sourceIndex);
  if (row < 0)
    return QModelIndex();
  return this->createIndex(row, sourceIndex.column());
}

QVariant QmitkFlatLabelInstanceProxyModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || !sourceModel())
    return QVariant();
  QModelIndex src = this->mapToSource(index);
  return sourceModel()->data(src, role);
}

void QmitkFlatLabelInstanceProxyModel::OnSourceModelReset()
{
  this->RebuildMapping();
}

void QmitkFlatLabelInstanceProxyModel::OnSourceDataChanged()
{
  this->RebuildMapping();
}
