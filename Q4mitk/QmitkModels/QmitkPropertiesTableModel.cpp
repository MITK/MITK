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

#include "QmitkPropertiesTableModel.h"

#include <mitkStringProperty.h>

QmitkPropertiesTableModel::QmitkPropertiesTableModel(mitk::NodePredicateBase* pred, QObject* parent)
: QAbstractTableModel(parent), m_NodePredicate(pred)
{
  SetDataStorage(mitk::DataStorage::GetInstance());
}

QmitkPropertiesTableModel::~QmitkPropertiesTableModel()
{
  if (m_NodePredicate) delete m_NodePredicate;
}

void QmitkPropertiesTableModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;

  if (m_NodePredicate)
    m_DataNodes = m_DataStorage->GetSubset(*m_NodePredicate);
  else
    m_DataNodes = m_DataStorage->GetAll();

  this->reset();
}

Qt::ItemFlags QmitkPropertiesTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QmitkPropertiesTableModel::data(const QModelIndex& index, int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
    {
      mitk::DataTreeNode::Pointer node = m_DataNodes->GetElement(index.row());

      mitk::PropertyList::Pointer list = node->GetPropertyList();
      const mitk::PropertyList::PropertyMap* map = list->GetMap(); // this is stupid!
      if (!map) return NULL;

      // iterate whole property map
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = map->begin();
        iter != map->end();
        ++iter )
      {
        std::string name = iter->first;
        return QVariant(QString::fromStdString(name));
      }

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

QVariant QmitkPropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
             case 0:
               return tr("Property Name");

             case 1:
               return tr("Property Value");

             case 2:
               return tr("(De)/Activated");

             default:
               return QVariant();
    }
  }
  return QVariant();

  //return QVariant("Header");
}

int QmitkPropertiesTableModel::rowCount(const QModelIndex& parent) const
{
  return m_DataNodes->Size();
}

int QmitkPropertiesTableModel::columnCount(const QModelIndex &parent)const
{
  return 3;
}