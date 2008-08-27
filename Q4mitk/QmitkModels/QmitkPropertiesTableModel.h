/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 14921 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKPROPERTIESTABLEMODEL_H_
#define QMITKPROPERTIESTABLEMODEL_H_

 #include <QAbstractTableModel>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

class QMITK_EXPORT QmitkPropertiesTableModel : public QAbstractTableModel
{
public:

  /**
   * The NodePredicate is owned by the model
   */
  QmitkPropertiesTableModel(mitk::NodePredicateBase* pred = 0, QObject* parent = 0);
  ~QmitkPropertiesTableModel();

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent)const;

private:

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataStorage::SetOfObjects::ConstPointer m_DataNodes;
  mitk::NodePredicateBase* m_NodePredicate;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
