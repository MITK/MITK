/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkUSZonesDataModel.h"

QmitkUSZonesDataModel::QmitkUSZonesDataModel(QObject *parent) :
  QAbstractTableModel(parent)
{
}

int QmitkUSZonesDataModel::rowCount ( const QModelIndex& /*parent*/ ) const
{
  return m_ZoneNodes.size();
}

int QmitkUSZonesDataModel::columnCount ( const QModelIndex& /*parent*/ ) const
{
  return 3;
}

QVariant QmitkUSZonesDataModel::headerData ( int section, Qt::Orientation orientation, int /*role*/ ) const
{
  if ( orientation != Qt::Horizontal) { return QVariant(QVariant::Invalid); }

  switch ( section )
  {
  case 0: return QVariant("Name");
  case 1: return QVariant("Size");
  case 2: return QVariant("Color");
  }

  return QVariant(QVariant::Invalid);
}

Qt::ItemFlags QmitkUSZonesDataModel::flags ( const QModelIndex & index ) const
{
  return Qt::ItemIsEditable;
}

QVariant QmitkUSZonesDataModel::data ( const QModelIndex& index, int /*role*/ ) const
{
  if (index.row() >= m_ZoneNodes.size() || index.column() >= this->columnCount())
  {
    return QVariant(QVariant::Invalid);
  }

  mitk::DataNode::Pointer curNode = m_ZoneNodes.at(index.row());

  switch ( index.column() )
  {
  case 0:
  {
    return QVariant(QString::fromStdString(curNode->GetName()));
  }
  case 1:
  {
    int intValue;
    if ( curNode->GetIntProperty("zone.size", intValue) )
    {
      return QVariant(intValue);
    }
    else
    {
      return QVariant(QVariant::Invalid);
    }
  }
  case 2:
  {
    std::string stringValue;
    if ( curNode->GetStringProperty("zone.color", stringValue) )
    {
      return QVariant(QString::fromStdString(stringValue));
    }
    else
    {
      return QVariant(QVariant::Invalid);
    }

  }
  }

  return QVariant(QVariant::Invalid);
}

bool QmitkUSZonesDataModel::setData ( const QModelIndex & index, const QVariant & value, int /*role*/ )
{
  if (index.row() >= m_ZoneNodes.size() || index.column() >= this->columnCount())
  {
    return false;
  }

  mitk::DataNode::Pointer curNode = m_ZoneNodes.at(index.row());

  switch ( index.column() )
  {
  case 0:
  {
    curNode->SetName(value.toString().toStdString());
    return true;
  }
  case 1:
  {
    curNode->SetIntProperty("zone.size", value.toInt());
    return true;
  }
  case 2:
  {
    curNode->SetStringProperty("zone.color", value.toString().toStdString().c_str());
    return true;
  }
  }

  return false;
}

bool QmitkUSZonesDataModel::insertRows ( int row, int count, const QModelIndex & parent )
{
  this->beginInsertRows(parent, row, row+count);

  for ( unsigned int n = 0; n < count; ++n )
  {
    m_ZoneNodes.insert(m_ZoneNodes.begin()+row, mitk::DataNode::New());
  }

  this->endInsertRows();

  return true;
}

bool QmitkUSZonesDataModel::removeRows ( int row, int count, const QModelIndex & parent )
{
  this->beginRemoveRows(parent, row, row+count);

  for ( int n = count-1; n > 0; n )
  {
    m_ZoneNodes.erase(m_ZoneNodes.begin()+row);
  }

  this->endRemoveRows();

  return true;
}
