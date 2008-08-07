/*=========================================================================

 Program:   openCherry Platform
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

class QmitkDataStorageListModel : public QAbstractListModel
{
public:

  /**
   * The NodePredicate is owned by the model
   */
  QmitkDataStorageListModel(mitk::NodePredicateBase* pred = 0, QObject* parent = 0);
  ~QmitkDataStorageListModel();

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;

private:

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataStorage::SetOfObjects::ConstPointer m_DataNodes;
  mitk::NodePredicateBase* m_NodePredicate;
};

#endif /* QMITKDATASTORAGELISTMODEL_H_ */
