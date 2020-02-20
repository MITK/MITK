/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATASTORAGEDEFAULTLISTMODEL_H
#define QMITKDATASTORAGEDEFAULTLISTMODEL_H

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/**
* @brief The 'QmitkDataStorageDefaultListModel' is a basic list model, derived from the 'QmitkAbstractDataStorageModel'.
*        It provides functions to accept a data storage and a node predicate in order to customize the model data nodes.
*        Furthermore it overrides the functions of 'QAbstractItemModel' to create a simple qt list model.
*        This model can be used in conjunction with a 'QmitkDataStorageSelectionConnector'.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageDefaultListModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  QmitkDataStorageDefaultListModel(QObject *parent);

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
  void NodeAdded(const mitk::DataNode* node) override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeChanged(const mitk::DataNode* node) override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeRemoved(const mitk::DataNode* node) override;

  // override pure virtual from 'QAbstractItemModel'
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  // override for customization
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:

  virtual void UpdateModelData();

  std::vector<mitk::DataNode::Pointer> m_DataNodes;

};

#endif // QMITKDATASTORAGEDEFAULTLISTMODEL_H
