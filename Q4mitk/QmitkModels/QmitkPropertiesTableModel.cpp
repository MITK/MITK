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

QmitkPropertiesTableModel::QmitkPropertiesTableModel(mitk::DataTreeNode::Pointer _Node, QObject* parent)
: QAbstractTableModel(parent)
, m_Node(_Node)
{
}

QmitkPropertiesTableModel::~QmitkPropertiesTableModel()
{
}

void QmitkPropertiesTableModel::setNode(mitk::DataTreeNode::Pointer _Node)
{
  if(m_Node != _Node)
  {
    m_Node = _Node;
    QAbstractTableModel::reset();
  }
}

mitk::DataTreeNode::Pointer QmitkPropertiesTableModel::getNode() const
{
  return m_Node;
}

Qt::ItemFlags QmitkPropertiesTableModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QmitkPropertiesTableModel::data(const QModelIndex& index, int role) const
{
  // empty data by default
  QVariant data;

  if(!index.isValid())
    return data;

  mitk::PropertyList::Pointer list = m_Node->GetPropertyList();
  const mitk::PropertyList::PropertyMap* map = list->GetMap(); // this is stupid!

  if(!map)
    return data;

  mitk::PropertyList::PropertyMap::const_iterator selectedProperty = map->begin();
  // advance the iterator to the property we need
  std::advance(selectedProperty, index.row());
  
  if(selectedProperty == map->end())
    return data;

  switch(role)
  {
    case Qt::DisplayRole:
      // the properties name
      if(index.column() == 0)
        data = QString::fromStdString(selectedProperty->first);
      // the real properties value
      else if(index.column() == 1)
        data = QString::fromStdString(selectedProperty->second.first->GetValueAsString());
      // enabled/disabled value
      else if(index.column() == 2)
        data = selectedProperty->second.second;
    break;
  }

  return data;
}

bool QmitkPropertiesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool setData = false;

  return setData;
}

QVariant QmitkPropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) 
    {
     case 0:
       return tr("Name");

     case 1:
       return tr("Value");

     case 2:
       return tr("Activated");

     default:
       return QVariant();
    }
  }

  return QVariant();
}

int QmitkPropertiesTableModel::rowCount(const QModelIndex& parent) const
{
  // return the number of properties in the properties list.
  return m_Node.IsNull() ? 0: m_Node->GetPropertyList()->GetMap()->size();
}

int QmitkPropertiesTableModel::columnCount(const QModelIndex &parent)const
{
  return 3;
}