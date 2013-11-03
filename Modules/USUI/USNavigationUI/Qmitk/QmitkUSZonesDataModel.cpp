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

QmitkUSZonesDataModel::QmitkUSZonesDataModel(QObject *parent) :
  QAbstractTableModel(parent)
{
}

void QmitkUSZonesDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;

  m_BaseNode = mitk::DataNode::New();
  m_BaseNode->SetName("Zones");
  m_BaseNode->SetData(mitk::Surface::New());
  m_DataStorage->Add(m_BaseNode);
}

void QmitkUSZonesDataModel::AddNode(mitk::Point3D center)
{
  this->insertRow(this->columnCount()-1);

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetData(this->MakeSphere(dataNode));
  dataNode->GetData()->GetGeometry()->SetOrigin(center);

  if (m_DataStorage.IsNotNull()) { m_DataStorage->Add(dataNode, m_BaseNode); }
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
    std::string stringValue;
    if ( curNode->GetStringProperty("zone.color", stringValue) && ! stringValue.empty())
    {
      QColor color(stringValue.c_str());
      if (color.isValid()) { return QVariant(QBrush(color)); }
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
      std::string stringValue;
      if ( curNode->GetStringProperty("zone.color", stringValue) ) { return QString::fromStdString(stringValue); }
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
      curNode->SetStringProperty("zone.color", value.toString().toStdString().c_str());
      break;
    }
    default:
      return false;
    }

    emit dataChanged(index, index);

    // add node to data storage if it isn't there already
    /*if ( m_DataStorage.IsNotNull() && ! m_DataStorage->Exists(curNode) )
    {
      mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
      curNode->SetData(pointSet);
      m_DataStorage->Add(curNode, m_BaseNode);
    }*/

  }

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

mitk::Surface::Pointer QmitkUSZonesDataModel::MakeSphere(const mitk::DataNode::Pointer dataNode) const
{
  int radius;
  if ( ! dataNode->GetIntProperty("zone.size", radius) ) { radius = 5; }

  mitk::Surface::Pointer zone= mitk::Surface::New();

  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius( radius );
  vtkData->SetCenter(0,0,0);
  vtkData->Update();
  zone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  return zone;
}
