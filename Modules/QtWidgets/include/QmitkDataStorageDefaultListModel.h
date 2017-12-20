/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDATASTORAGEDEFAULTLISTMODEL_H
#define QMITKDATASTORAGEDEFAULTLISTMODEL_H

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkIDataStorageViewModel.h>

// qt
#include <QAbstractListModel>

/**
* @brief The 'QmitkDataStorageDefaultListModel' is a basic list model, that implements the 'QmitkIDataStorageViewModel'.
*        It provides functions to accept a data storage and a node predicate in order to customize the model data nodes.
*        Furthermore it overrides the functions of 'QAbstractItemModel' to create a simple qt list model.
*        This model can be used inside a concrete implementation of the 'QmitkDataStorageAbstractView' and therefore be
*        connected to the data storage and the signals and slots of a concrete data storage viewer.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageDefaultListModel : public QmitkIDataStorageViewModel
{

public:

  QmitkDataStorageDefaultListModel(QObject *parent);

  // override from 'QmitkIDataStorageViewModel'
  /*
  * @brief See 'QmitkIDataStorageViewModel'
  */
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  /*
  * @brief See 'QmitkIDataStorageViewModel'
  */
  void SetNodePredicate(mitk::NodePredicateBase* nodePredicate) override;

  // override pure virtual from 'QAbstractItemModel'
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  // override for customization
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

private:

  mitk::DataStorage::Pointer m_DataStorage;
  std::vector<mitk::DataNode::Pointer> m_DataNodes;

  mitk::NodePredicateBase::Pointer m_NodePredicate;

};

#endif // QMITKDATASTORAGEDEFAULTLISTMODEL_H
