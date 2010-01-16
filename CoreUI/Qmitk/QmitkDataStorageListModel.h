/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKDATASTORAGELISTMODEL_H_
#define QMITKDATASTORAGELISTMODEL_H_

//# Own includes
// mitk
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkCommon.h"
// Qmitk

//# Toolkit includes
// Qt
#include <QAbstractListModel>
// stl
#include <vector>

class QMITK_EXPORT QmitkDataStorageListModel: public QAbstractListModel
{
public:
  //# Ctors / Dtor
  ///
  /// The NodePredicate is owned by the model
  ///
  QmitkDataStorageListModel(mitk::DataStorage::Pointer dataStorage = 0, mitk::NodePredicateBase* pred = 0, QObject* parent = 0);
  ~QmitkDataStorageListModel();


  //# Getter / Setter
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
  mitk::DataStorage::Pointer GetDataStorage() const;

  void SetPredicate(mitk::NodePredicateBase* pred);
  mitk::NodePredicateBase* GetPredicate() const;

  std::vector<mitk::DataTreeNode*> GetDataNodes() const; 

  mitk::DataTreeNode::Pointer getNode(const QModelIndex &index) const;

  //# From QAbstractListModel
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  ///
  /// Called when a DataStorage Add Event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeAdded(const mitk::DataTreeNode* node);
  ///
  /// Called when a DataStorage Remove Event was thrown. May be reimplemented
  /// by deriving classes.
  ///
  virtual void NodeRemoved(const mitk::DataTreeNode* node);

  ///
  /// \brief Called when a itk::Object that is hold as a member variable was 
  /// modified in order to react to it.
  ///
  virtual void OnModified(const itk::Object *caller, const itk::EventObject &event);

  ///
  /// \brief Called when a itk::Object that is hold as a member variable is about to be 
  /// deleted in order to react to it.
  ///
  virtual void OnDelete(const itk::Object *caller, const itk::EventObject &event);

protected:
  ///
  /// \brief Resets the whole model. Get all nodes matching the predicate from the data storage.
  ///
  void reset();

  ///
  /// Holds the predicate that defines this SubSet of Nodes. If m_Predicate
  /// is NULL all Nodes will be selected. *Attention: this class owns the predicate and deletes it*
  ///
  mitk::NodePredicateBase::Pointer m_NodePredicate;

  ///
  /// Pointer to the DataStorage from which the nodes are selected (remember: in BlueBerry there
  /// might be more than one DataStorage).
  ///
  mitk::DataStorage* m_DataStorage;

  ///
  /// \brief Holds the tag of the datastorage-delete observer.
  ///
  unsigned long m_DataStorageDeleteObserverTag;

  ///
  /// Holds all selected Nodes. Dont hold smart pointer as we are in a GUI class.
  ///
  std::vector<mitk::DataTreeNode*> m_DataNodes;

  ///
  /// \brief Holds the tags of the node-modified observers.
  ///
  std::vector<unsigned long> m_DataNodesModifiedObserversTags;

  ///
  /// Saves if this model is currently working on events to prevent endless event loops.
  /// 
  bool m_BlockEvents;
};

#endif /* QMITKDATASTORAGELISTMODEL_H_ */
