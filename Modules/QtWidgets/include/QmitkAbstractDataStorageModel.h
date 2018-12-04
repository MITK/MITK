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

#ifndef QMITKABSTRACTDATASTORAGEMODEL_H
#define QMITKABSTRACTDATASTORAGEMODEL_H

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>
#include <mitkWeakPointer.h>

// qt
#include <QAbstractItemModel>

/*
* @brief This abstract class extends the 'QAbstractItemModel' to accept an 'mitk::DataStorage' and a 'mitk::NodePredicateBase'.
*   It registers itself as a node event listener of the data storage.
*   The 'QmitkAbstractDataStorageModel' provides three empty functions, 'NodeAdded', 'NodeChanged' and 'NodeRemoved', that
*   may be implemented by subclasses. These functions allow to react to the 'AddNodeEvent', 'ChangedNodeEvent' and
*   'RemoveNodeEvent' of the data storage. This might be useful to force an update on a custom view to correctly
*   represent the content of the data storage.
*
*   A concrete implementation of this class is used to store the temporarily shown data nodes of the data storage.
*   These nodes may be a subset of all the nodes inside the data storage, if a specific node predicate is set.
*
*   A model that implements this class has to return mitk::DataNode::Pointer objects for model indexes when the
*   role is QmitkDataNodeRole.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractDataStorageModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  virtual ~QmitkAbstractDataStorageModel();
  /*
  * @brief Sets the data storage and adds listener for node events.
  *
  * @param dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  mitk::DataStorage* GetDataStorage() const;
  /*
  * @brief Sets the node predicate and updates the model data, according to the node predicate.
  *
  * @param nodePredicate    A pointer to node predicate.
  */
  void SetNodePredicate(mitk::NodePredicateBase* nodePredicate);

  mitk::NodePredicateBase* GetNodePredicate() const { return m_NodePredicate; }

protected:

  virtual void DataStorageChanged() = 0;
  virtual void NodePredicateChanged() = 0;

  virtual void NodeAdded(const mitk::DataNode* node) = 0;
  virtual void NodeChanged(const mitk::DataNode* node) = 0;
  virtual void NodeRemoved(const mitk::DataNode* node) = 0;

  QmitkAbstractDataStorageModel(QObject* parent = nullptr);
  QmitkAbstractDataStorageModel(mitk::DataStorage* dataStorage, QObject* parent = nullptr);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_NodePredicate;

private:

  /** Helper triggered on the storage delete event */
  void SetDataStorageDeleted();

  unsigned long m_DataStorageDeletedTag;

};

#endif // QMITKABSTRACTDATASTORAGEMODEL_H
