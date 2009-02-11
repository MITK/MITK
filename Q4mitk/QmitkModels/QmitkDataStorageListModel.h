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

protected:
  void reset();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::NodePredicateBase* m_NodePredicate;
  mitk::DataStorage::SetOfObjects::ConstPointer m_DataNodes;
};

#endif /* QMITKDATASTORAGELISTMODEL_H_ */
