/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATASTORAGESIMPLETREEMODEL_H
#define QMITKDATASTORAGESIMPLETREEMODEL_H

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

class QmitkDataStorageTreeModelInternalItem;

/**
* @brief The 'QmitkDataStorageSimpleTreeModel' is a basic tree model, derived from the 'QmitkAbstractDataStorageModel'.
*  It provides functions to accept a data storage and a node predicate in order to customize the model data nodes.
*  Furthermore it overrides the functions of 'QAbstractItemModel' to create a simple qt list model.*
*  This model can be used in conjunction with a 'QmitkDataStorageSelectionConnector'.
*  This model is a "light" version of the classic QmitkDataStorgageTreeModel. The differences between both are the following:
*  - This class currently does not support DragNDrop.
*  - This class does not have the ability to change hierarchy or changes the layer property of nodes.
*    This was skipped on purpose, because that is not the job of the storage model.
*  - If a tree item A is removed this class does not attach children of A to the parent of A.
*    Instead the complete tree representation is updated. This was changed on purpose because otherwise the internal
*    representation of the model would not reflect the data storage graph anymore.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageSimpleTreeModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:
  QmitkDataStorageSimpleTreeModel(QObject *parent);
  ~QmitkDataStorageSimpleTreeModel() override;

  // override from 'QmitkAbstractDataStorageModel'
  /*
   * @brief See 'QmitkAbstractDataStorageModel'
   */
  void DataStorageChanged() override;
  /*
   * @brief See 'QmitkAbstractDataStorageModel'
   */
  void NodePredicateChanged() override;
  /*
   * @brief See 'QmitkAbstractDataStorageModel'
   */
  void NodeAdded(const mitk::DataNode *node) override;
  /*
   * @brief See 'QmitkAbstractDataStorageModel'
   */
  void NodeChanged(const mitk::DataNode *node) override;
  /*
   * @brief See 'QmitkAbstractDataStorageModel'
   */
  void NodeRemoved(const mitk::DataNode *node) override;

  // override pure virtual from 'QAbstractItemModel'
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
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
  Prefered over iterating over the tree by hand because we can use std::find.*/
  std::list<const TreeItem*> m_TreeItems;
};

#endif // QMITKDATASTORAGESIMPLETREEMODEL_H
