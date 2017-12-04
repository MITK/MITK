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

#ifndef QMITKIDATASTORAGEVIEWMODEL_H
#define QMITKIDATASTORAGEVIEWMODEL_H

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

// qt
#include <QAbstractItemModel>

/*
* @brief This interface extends the 'QAbstractItemModel' to accept an 'mitk::DataStorage' and a 'mitk::NodePredicateBase'.
*
*   The interface is used inside the 'QmitkDataStorageAbstractView' and its concrete implementations to store the
*   temporarily shown data nodes of the data storage. These nodes may be a subset of all the nodes inside the data
*   storage, if a specific node predicate is set.
*
*   A model that implements this interface has to return mitk::DataNode::Pointer objects for model indices when the
*   role is QmitkDataNodeRole.
*/
class MITKQTWIDGETS_EXPORT QmitkIDataStorageViewModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  /*
  * @brief Sets the data storage.
  *
  *   This function is called inside the "QmitkDataStorageAbstractView" to propagate a new data storage to the model.
  *
  * @par dataStorage      A pointer to the data storage to set.
  */
  virtual void SetDataStorage(mitk::DataStorage* dataStorage) = 0;
  /*
  * @brief Sets the node predicate and updates the model data, according to the node predicate.
  *
  *   This function is called inside the "QmitkDataStorageAbstractView" to propagate a new node predicate to the model.
  *
  * @par nodePredicate    A pointer to node predicate.
  */
  virtual void SetNodePredicate(mitk::NodePredicateBase::Pointer nodePredicate) = 0;
};

#endif // QMITKIDATASTORAGEVIEWMODEL_H
