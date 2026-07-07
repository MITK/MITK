/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageDefaultListModel_h
#define QmitkDataStorageDefaultListModel_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/**
 * \brief A flat list model that exposes DataStorage nodes matching an optional predicate.
 *
 * This model derives from QmitkAbstractDataStorageModel and presents a simple
 * flat (non-hierarchical) list of data nodes. It reacts to node add, change,
 * and remove events by rebuilding its internal node list. The model returns
 * mitk::DataNode::Pointer for the QmitkDataNodeRole and the node name for
 * Qt::DisplayRole.
 *
 * \sa QmitkAbstractDataStorageModel
 * \sa QmitkDataStorageListInspector
 */
class MITKQTWIDGETS_EXPORT QmitkDataStorageDefaultListModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  /**
   * \brief Constructs the model.
   * \param[in] parent The parent QObject.
   */
  QmitkDataStorageDefaultListModel(QObject *parent);

  /** \brief Called when the data storage has changed. Rebuilds the model data. */
  void DataStorageChanged() override;

  /** \brief Called when the node predicate has changed. Rebuilds the model data. */
  void NodePredicateChanged() override;

  /**
   * \brief Called when a node is added to the data storage.
   * \param[in] node The node that was added.
   */
  void NodeAdded(const mitk::DataNode* node) override;

  /**
   * \brief Called when a node in the data storage has changed.
   * \param[in] node The node that was changed.
   */
  void NodeChanged(const mitk::DataNode* node) override;

  /**
   * \brief Called when a node is removed from the data storage.
   * \param[in] node The node that was removed.
   */
  void NodeRemoved(const mitk::DataNode* node) override;

  /**
   * \brief Returns the model index for the given row and column.
   * \param[in] row    The row number.
   * \param[in] column The column number.
   * \param[in] parent The parent index (unused for flat list models).
   * \return A valid model index for valid rows and column 0, otherwise an invalid index.
   */
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the parent index. Always returns an invalid index (flat model).
   * \param[in] child The child index.
   * \return An invalid QModelIndex.
   */
  QModelIndex parent(const QModelIndex &child) const override;

  /**
   * \brief Returns the number of rows (nodes) in the model.
   * \param[in] parent The parent index (must be invalid for top-level count).
   * \return The number of data nodes.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns. Always returns 1.
   * \param[in] parent The parent index.
   * \return 1.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns data for the given index and role.
   *
   * Supports Qt::DisplayRole (node name), Qt::DecorationRole (node descriptor icon),
   * QmitkDataNodeRole (mitk::DataNode::Pointer), and QmitkDataNodeRawPointerRole.
   *
   * \param[in] index The model index.
   * \param[in] role  The data role.
   * \return The requested data as QVariant, or an invalid QVariant.
   */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
   * \brief Returns header data for the given section and orientation.
   * \param[in] section     The section number.
   * \param[in] orientation The orientation (horizontal or vertical).
   * \param[in] role        The data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /**
   * \brief Returns the item flags for the given index.
   * \param[in] index The model index.
   * \return Item flags including Qt::ItemIsEnabled and Qt::ItemIsSelectable for valid indexes.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:

  virtual void UpdateModelData();

  std::vector<mitk::DataNode::Pointer> m_DataNodes;

};

#endif
