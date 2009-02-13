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

#include <QAbstractListModel>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

class QMITK_EXPORT QmitkDataStorageListModel : public QAbstractListModel
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

  mitk::DataStorage::SetOfObjects::ConstPointer GetDataNodes() const; 

  
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
protected:
  void reset();


  ///
  /// Pointer to the DataStorage from which the nodes are selected (remember: in OpenCherry there
  /// might be more than one DataStorage).
  ///
  mitk::DataStorage::Pointer m_DataStorage;

  ///
  /// Holds the predicate that defines this SubSet of Nodes. If m_Predicate
  /// is NULL all Nodes will be selected.
  ///
  mitk::NodePredicateBase* m_NodePredicate;

  ///
  /// Holds all selected Nodes.
  ///
  mitk::DataStorage::SetOfObjects::ConstPointer m_DataNodes;

  ///
  /// Saves if this model is currently working on events to prevent endless event loops.
  /// 
  bool m_ProcessesEvents;
};

#endif /* QMITKDATASTORAGELISTMODEL_H_ */
