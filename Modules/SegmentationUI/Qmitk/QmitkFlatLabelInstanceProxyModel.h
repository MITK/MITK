/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFlatLabelInstanceProxyModel_h
#define QmitkFlatLabelInstanceProxyModel_h

#include <QAbstractProxyModel>
#include <QVector>
#include <QModelIndex>

/**
 * @brief A proxy model that flattens a hierarchical content of a QmitkMultiLabelTreeModel instance into a simple list.
 *
 * It only includes items that have a non-empty QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceDataRole.
 * So everything in the model tree that counts as an label instance representation.
 */
class QmitkFlatLabelInstanceProxyModel : public QAbstractProxyModel
{
  Q_OBJECT

public:
  explicit QmitkFlatLabelInstanceProxyModel(QObject* parent = nullptr);

  // QAbstractItemModel interface
  QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
  QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  void setSourceModel(QAbstractItemModel* sourceModel) override;

public slots:
  /// Rebuilds the internal mapping from source indices to flat indices.
  void RebuildMapping();

private slots:
  void OnSourceModelReset();
  void OnSourceDataChanged();

private:
  void CollectAcceptedIndexes(const QModelIndex& parent);

private:
  QVector<QPersistentModelIndex> m_SourceIndexes;
};

#endif
