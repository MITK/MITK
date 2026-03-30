/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageSimpleTreeModel_h
#define QmitkDataStorageSimpleTreeModel_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

class QmitkDataStorageTreeModelInternalItem;

/**
 * \brief Lightweight tree model for DataStorage contents.
 *
 * This model extends QmitkAbstractDataStorageModel to present data storage
 * nodes in a tree hierarchy reflecting parent-child source relationships.
 * It is a simplified alternative to QmitkDataStorageTreeModel with the
 * following design differences:
 *
 * - Drag-and-drop is not supported.
 * - Node hierarchy and layer properties cannot be modified through the model.
 * - When a node is removed, the entire tree is rebuilt rather than re-parenting
 *   orphaned children, ensuring the model always reflects the actual data
 *   storage graph.
 *
 * \sa QmitkAbstractDataStorageModel
 * \sa QmitkDataStorageTreeModel
 * \sa QmitkDataStorageTreeInspector
 */
class MITKQTWIDGETS_EXPORT QmitkDataStorageSimpleTreeModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the tree model.
   * \param[in] parent The parent QObject.
   */
  QmitkDataStorageSimpleTreeModel(QObject *parent);
  ~QmitkDataStorageSimpleTreeModel() override;

  /** \brief Called when the data storage has changed. Rebuilds the tree. */
  void DataStorageChanged() override;

  /** \brief Called when the node predicate has changed. Rebuilds the tree. */
  void NodePredicateChanged() override;

  /**
   * \brief Called when a node is added to the data storage.
   * \param[in] node The node that was added.
   */
  void NodeAdded(const mitk::DataNode *node) override;

  /**
   * \brief Called when a node in the data storage has changed.
   * \param[in] node The node that was changed.
   */
  void NodeChanged(const mitk::DataNode *node) override;

  /**
   * \brief Called when a node is removed from the data storage.
   * \param[in] node The node that was removed.
   */
  void NodeRemoved(const mitk::DataNode *node) override;

  /**
   * \brief Returns the model index for the given row and column under the given parent.
   * \param[in] row    The row number.
   * \param[in] column The column number.
   * \param[in] parent The parent index.
   * \return A valid model index if the position is valid, otherwise an invalid index.
   */
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the parent index for the given child.
   * \param[in] child The child index.
   * \return The parent index, or an invalid index for top-level items.
   */
  QModelIndex parent(const QModelIndex &child) const override;

  /**
   * \brief Returns the number of child rows under the given parent.
   * \param[in] parent The parent index.
   * \return The number of children.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns (always 1).
   * \param[in] parent The parent index.
   * \return 1.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns data for the given index and role.
   * \param[in] index The model index.
   * \param[in] role  The data role.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /**
   * \brief Sets data at the given index (supports renaming nodes).
   * \param[in] index The model index.
   * \param[in] value The new value.
   * \param[in] role  The data role.
   * \return True if the data was set successfully.
   */
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  /**
   * \brief Returns header data for the given section.
   * \param[in] section     The section number.
   * \param[in] orientation The orientation.
   * \param[in] role        The data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /**
   * \brief Returns item flags for the given index.
   * \param[in] index The model index.
   * \return Item flags including Qt::ItemIsEditable for valid indexes.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
  using TreeItem = QmitkDataStorageTreeModelInternalItem;

private:
  void UpdateModelData();
  void AddNodeInternal(const mitk::DataNode *node);

  mitk::DataNode *GetParentNode(const mitk::DataNode *node) const;

  TreeItem *TreeItemFromIndex(const QModelIndex &index) const;
  QModelIndex IndexFromTreeItem(TreeItem *item) const;

  void ResetTree();

  TreeItem *m_Root;

  /**helper structure to check, if a tree item is really part of the model.
  Preferred over iterating over the tree by hand because we can use std::find.*/
  std::list<const TreeItem*> m_TreeItems;
};

#endif
