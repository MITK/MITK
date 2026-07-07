/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAbstractDataStorageModel_h
#define QmitkAbstractDataStorageModel_h

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>
#include <mitkWeakPointer.h>

// qt
#include <QAbstractItemModel>

/**
 * \brief Abstract base class extending QAbstractItemModel for DataStorage-backed models.
 *
 * This class bridges the Qt model/view framework with the MITK DataStorage by
 * accepting an mitk::DataStorage and an optional mitk::NodePredicateBase for
 * filtering. It automatically registers itself as a listener for node events
 * (AddNodeEvent, ChangedNodeEvent, RemoveNodeEvent) on the data storage and
 * provides pure virtual callbacks (NodeAdded, NodeChanged, NodeRemoved) so that
 * subclasses can react to these events and update their internal state.
 *
 * Subclasses must also implement DataStorageChanged() and NodePredicateChanged()
 * to handle changes of the data storage or predicate, respectively.
 *
 * A model that implements this class must return mitk::DataNode::Pointer objects
 * for model indexes when the data role is QmitkDataNodeRole.
 *
 * \sa QmitkAbstractDataStorageInspector
 * \sa QmitkDataStorageDefaultListModel
 * \sa QmitkDataStorageSimpleTreeModel
 */
class MITKQTWIDGETS_EXPORT QmitkAbstractDataStorageModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  ~QmitkAbstractDataStorageModel() override;

  /**
   * \brief Sets the data storage and registers node event listeners.
   *
   * Removes listeners from the previous data storage (if any), sets the new
   * data storage, and registers listeners for AddNodeEvent, RemoveNodeEvent,
   * ChangedNodeEvent, and the DeleteEvent. Finally calls DataStorageChanged()
   * to allow the subclass to update its internal representation.
   *
   * \param[in] dataStorage Pointer to the new data storage. May be nullptr.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Returns a locked pointer to the current data storage.
   * \return A smart pointer to the data storage, or nullptr if none is set or it has been deleted.
   */
  mitk::DataStorage::Pointer GetDataStorage() const;

  /**
   * \brief Sets the node predicate and triggers a model update.
   *
   * The predicate is used to filter the set of nodes shown by the model.
   * If the predicate changes, NodePredicateChanged() is called.
   *
   * \param[in] nodePredicate Pointer to the node predicate. May be nullptr to show all nodes.
   */
  void SetNodePredicate(const mitk::NodePredicateBase* nodePredicate);

  /**
   * \brief Returns the currently set node predicate.
   * \return Pointer to the current node predicate, or nullptr if none is set.
   */
  const mitk::NodePredicateBase* GetNodePredicate() const { return m_NodePredicate; }

protected:

  virtual void DataStorageChanged() = 0;
  virtual void NodePredicateChanged() = 0;

  virtual void NodeAdded(const mitk::DataNode* node) = 0;
  virtual void NodeChanged(const mitk::DataNode* node) = 0;
  virtual void NodeRemoved(const mitk::DataNode* node) = 0;

  QmitkAbstractDataStorageModel(QObject* parent = nullptr);
  QmitkAbstractDataStorageModel(mitk::DataStorage* dataStorage, QObject* parent = nullptr);

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::ConstPointer m_NodePredicate;

private:

  /** Helper triggered on the storage delete event */
  void SetDataStorageDeleted();

  unsigned long m_DataStorageDeletedTag;

};

#endif
