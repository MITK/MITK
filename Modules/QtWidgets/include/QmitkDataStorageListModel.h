/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageListModel_h
#define QmitkDataStorageListModel_h

#include <MitkQtWidgetsExports.h>

// MITK
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"

// Qt
#include <QAbstractListModel>

//! \ingroup QmitkModule
//! Qt list model for the (optionally filtered) nodes in a DataStorage.
//!
//! Given a data storage instance, this model will observe the storage
//! for its list of nodes and keep the provided Qt model up to date.
//! When given a NodePredicateBase instance, the Qt model will only
//! contain nodes that satisfy the predicate. This is useful to
//! display lists of a certain data type only, for example.
//!
//! Developer notes:
//! - class should be reviewed by somebody who knows well Qt models
//! - The OnSomethingModifedAddedRemoved() methods are declared virtual. They are required
//!   to be executed on event reception, though! They should not be virtual.
//! - Is there any valid use case for sub-classing? Declare class final?
//! - Is GetDataNodes needed? DataStorage or Qt model would yield the same result.

class MITKQTWIDGETS_EXPORT QmitkDataStorageListModel : public QAbstractListModel
{
public:
  //! \param dataStorage the data storage to represent
  //! \param pred the optional predicate to filter filters
  //! \param parent the Qt parent of this Qt object
  QmitkDataStorageListModel(mitk::DataStorage *dataStorage = nullptr,
                            mitk::NodePredicateBase::Pointer pred = nullptr,
                            QObject *parent = nullptr);

  ~QmitkDataStorageListModel() override;

  //! Change the data storage to represent
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  //! Get the represented data storage
  mitk::DataStorage *GetDataStorage() const;

  //! Change the filter predicate
  void SetPredicate(mitk::NodePredicateBase *pred);

  //! Get the filter predicate in use
  mitk::NodePredicateBase *GetPredicate() const;

  //! Get all current data nodes
  std::vector<mitk::DataNode *> GetDataNodes() const;

  //! Return the node for given model index
  mitk::DataNode::Pointer getNode(const QModelIndex &index) const;

  //! Return the model index of the given node
  QModelIndex getIndex(const mitk::DataNode *node) const;

  //! Implements QAbstractListModel
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  //! Implements QAbstractListModel
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  //! Implements QAbstractListModel
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  //! Implements QAbstractListModel
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  //! Called when a DataStorage Add Event was thrown. May be reimplemented
  //! by deriving classes.
  //!
  //! \warning When sub-classing, call this class' method first! Otherwise the node
  //!          addition will not be reflected in the Qt model!
  virtual void OnDataStorageNodeAdded(const mitk::DataNode *node);

  //! Called when a DataStorage Remove Event was thrown. May be reimplemented
  //! by deriving classes.
  //!
  //! \warning When sub-classing, call this class' method first! Otherwise the node
  //!          removal will not be reflected in the Qt model!
  virtual void OnDataStorageNodeRemoved(const mitk::DataNode *node);

  //! Callback entry for observed DataNodes' ModifiedEvent().
  //!
  //! Emits signal dataChanged().
  virtual void OnDataNodeModified(const itk::Object *caller, const itk::EventObject &event);

  //! Callback entry for observed BaseDatas' ModifiedEvent().
  //!
  //! Emits signal dataChanged().
  virtual void OnDataModified(const itk::Object *caller, const itk::EventObject &event);

  //! Callback entry for DataStorage's DeleteEvent().
  //!
  //! Clears the model.
  virtual void OnDataStorageDeleted(const itk::Object *caller, const itk::EventObject &event);

protected:
  //! \brief Resets the whole model. Get all nodes matching the predicate from the data storage.
  void reset();

  //! Internal helper: adds given node to end of list
  void AddNodeToInternalList(mitk::DataNode *node);

  //! Internal helper: remove given node
  void RemoveNodeFromInternalList(mitk::DataNode *node);

  //! Internal helper: Clear complete model list
  void ClearInternalNodeList();

private:
  enum OBSERVER_TUPLE_NAMES
  {
    NODE = 0,
    NODE_OBSERVER = 1,
    DATA_OBSERVER = 2,
  };

  //! Holds the predicate that defines what nodes are part of the model.
  mitk::NodePredicateBase::Pointer m_NodePredicate;

  //! The DataStorage that is represented in the model.
  //! We keep only a weak pointer and observe the storage for deletion.
  mitk::DataStorage *m_DataStorage;

  //! ITK observer tag for the storage's DeleteEvent()
  unsigned long m_DataStorageDeleteObserverTag;

  //! List of the current model's DataNodes along with their ModifiedEvent observer tags
  //! - element 0 : node
  //! - element 1 : node's ModifiedEvent observer.
  //! - element 2 : node data's ModifiedEvent observer.
  std::vector<std::tuple<mitk::DataNode *, unsigned long, unsigned long>> m_NodesAndObserverTags;

  //! Prevents infinite loops.
  bool m_BlockEvents;
};

#endif /* QMITKDATASTORAGELISTMODEL_H_ */
