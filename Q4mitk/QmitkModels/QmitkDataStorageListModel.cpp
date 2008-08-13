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

#include "QmitkDataStorageListModel.h"

#include <mitkStringProperty.h>

QmitkDataStorageListModel::QmitkDataStorageListModel(mitk::NodePredicateBase* pred, QObject* parent)
 : QAbstractListModel(parent), m_NodePredicate(pred)
{
  SetDataStorage(mitk::DataStorage::GetInstance());
}

QmitkDataStorageListModel::~QmitkDataStorageListModel()
{
  if (m_NodePredicate) delete m_NodePredicate;
}

void QmitkDataStorageListModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;

  if (m_NodePredicate)
    m_DataNodes = m_DataStorage->GetSubset(*m_NodePredicate);
  else
    m_DataNodes = m_DataStorage->GetAll();

  this->reset();
}

Qt::ItemFlags QmitkDataStorageListModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QmitkDataStorageListModel::data(const QModelIndex& index, int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
  {
    mitk::DataTreeNode::Pointer node = m_DataNodes->GetElement(index.row());
    std::string name = node->GetName();
    return QVariant(QString::fromStdString(name));
  }
  break;
  case QmitkDataTreeNodeRole:
  {
    mitk::DataTreeNode::Pointer node = m_DataNodes->GetElement(index.row());
    return QVariant::fromValue(node);
  }
    break;
  }

  return QVariant();
}

QVariant QmitkDataStorageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant("Header");
}

int QmitkDataStorageListModel::rowCount(const QModelIndex& parent) const
{
  return m_DataNodes->Size();
}
