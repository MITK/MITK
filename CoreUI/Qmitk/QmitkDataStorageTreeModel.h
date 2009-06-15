/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date: 2008-08-13 16:56:36 +0200 (Mi, 13 Aug 2008) $
 Version:   $Revision: 14972 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKDATASTORAGETREEMODEL_H_
#define QMITKDATASTORAGETREEMODEL_H_

#include <QAbstractListModel>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

class QMITK_EXPORT QmitkDataStorageTreeModel : public QAbstractItemModel
{
public:

  enum DataStorageViewMode
  {
    DSVW_DatatypeBased,
	DSVW_Hirarchy
  };

  /**
   * The NodePredicate is owned by the model
   */
  QmitkDataStorageTreeModel(mitk::NodePredicateBase* pred = 0, QObject* parent = 0);
  ~QmitkDataStorageTreeModel();

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QModelIndex parent ( const QModelIndex & index ) const;
  QModelIndex index ( unsigned int row, unsigned int column, const QModelIndex & parent = QModelIndex() ) const;
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;
  
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
private:

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataStorage::SetOfObjects::ConstPointer m_DataNodes;
  mitk::NodePredicateBase::Pointer m_NodePredicate;
  DataStorageViewMode m_ViewMode;
};

#endif /* QMITKDATASTORAGETREEMODEL_H_ */
