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

#include <QBrush>

#include <vtkSphereSource.h>

#include "mitkProperties.h"
#include "mitkRenderingManager.h"

QmitkUSZonesDataModel::QmitkUSZonesDataModel(QObject *parent) :
  QAbstractTableModel(parent)
{
}

void QmitkUSZonesDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer baseNode)
{
  m_DataStorage = dataStorage;
  m_BaseNode = baseNode;
}

void QmitkUSZonesDataModel::AddNode(mitk::DataNode::Pointer node)
{
  this->insertRow(this->rowCount());
  m_ZoneNodes.at(m_ZoneNodes.size()-1) = node;
}

int QmitkUSZonesDataModel::rowCount ( const QModelIndex& /*parent*/ ) const
{
  return m_ZoneNodes.size();
}

int QmitkUSZonesDataModel::columnCount ( const QModelIndex& /*parent*/ ) const
{
  return 3;
}

QVariant QmitkUSZonesDataModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
  if ( role != Qt::DisplayRole ) { return QVariant(QVariant::Invalid); }

  if ( orientation == Qt::Horizontal )
  {
    switch ( section )
    {
    case 0: return QVariant("Name");
    case 1: return QVariant("Size");
    case 2: return QVariant("Color");
    }
  }
  else
  {
    return QVariant(section+1);
  }

  return QVariant(QVariant::Invalid);
}

Qt::ItemFlags QmitkUSZonesDataModel::flags ( const QModelIndex& /*index*/ ) const
{
  return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant QmitkUSZonesDataModel::data ( const QModelIndex& index, int role ) const
{
  // make sure that row and column index fit data borders
  if (static_cast<unsigned int>(index.row()) >= m_ZoneNodes.size()
      || index.column() >= this->columnCount())
  {
    return QVariant(QVariant::Invalid);
  }

  mitk::DataNode::Pointer curNode = m_ZoneNodes.at(index.row());


  switch (role)
  {
  case Qt::BackgroundColorRole:
  {
    mitk::ScalarType color[3];
    if ( curNode->GetColor(color) )
    {
      QColor qColor(color[0] * 255, color[1] * 255, color[2] * 255);
      if (qColor.isValid()) { return QVariant(QBrush(qColor)); }
    }

    break;
  }

  case Qt::EditRole:
  case Qt::DisplayRole:
  {
    switch ( index.column() )
    {
    case 0:
    {
      return QString::fromStdString(curNode->GetName());
    }
    case 1:
    {
      int intValue;
      if ( curNode->GetIntProperty("zone.size", intValue) ) { return intValue; }
      else { return QVariant(QVariant::Invalid); }
    }
    case 2:
    {
      mitk::ScalarType color[3];
      if ( curNode->GetColor(color) )
      {
        QColor qColor(color[0] * 255, color[1] * 255, color[2] * 255);

        if (qColor == Qt::green) { return QVariant("Green"); }
        else if (qColor == Qt::red) { return QVariant("Red"); }
        else if (qColor == Qt::blue) { return QVariant("Blue"); }
        else if (qColor == Qt::yellow) { return QVariant("Yellow"); }
        else { return QVariant(QVariant::Invalid); }
      }
      else { return QVariant(QVariant::Invalid); }
    }
    }
    break;
  }
  }

  return QVariant(QVariant::Invalid);
}

bool QmitkUSZonesDataModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
  if (role == Qt::EditRole)
  {
    if (static_cast<unsigned int>(index.row()) >= m_ZoneNodes.size()
        || index.column() >= this->columnCount())
    {
      return false;
    }

    mitk::DataNode::Pointer curNode = m_ZoneNodes.at(index.row());

    switch ( index.column() )
    {
    case 0:
    {
      curNode->SetName(value.toString().toStdString());
      break;
    }
    case 1:
    {
      curNode->SetIntProperty("zone.size", value.toInt());
      break;
    }
    case 2:
    {
      QColor color(value.toString());
      curNode->SetColor(color.redF(), color.greenF(), color.blueF());
      break;
    }
    default:
      return false;
    }

    emit dataChanged(index, index);
  }

  // update the RenderWindow to show new points
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}

bool QmitkUSZonesDataModel::insertRows ( int row, int count, const QModelIndex & parent )
{
  this->beginInsertRows(parent, row, row+count-1);

  for ( int n = 0; n < count; ++n )
  {
    m_ZoneNodes.insert(m_ZoneNodes.begin()+row, mitk::DataNode::New());
  }

  this->endInsertRows();

  return true;
}

bool QmitkUSZonesDataModel::removeRows ( int row, int count, const QModelIndex & parent )
{
  if ( static_cast<unsigned int>(row+count) > m_ZoneNodes.size() ) { return false; }

  this->beginRemoveRows(parent, row, row+count-1);

  for ( int n = count-1; n >= 0; --n )
  {
    DataNodeVector::iterator it = m_ZoneNodes.begin()+row+n;
    if (m_DataStorage.IsNotNull()) { m_DataStorage->Remove(*it); }
    m_ZoneNodes.erase(it);
  }

  this->endRemoveRows();

  return true;
}
