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

#include "mitkMessage.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkNodePredicateSource.h"

const char* QmitkUSZonesDataModel::DataNodePropertySize = "zone.size";


QmitkUSZonesDataModel::QmitkUSZonesDataModel(QObject *parent) :
  QAbstractTableModel(parent)
{
}

void QmitkUSZonesDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer baseNode)
{
  m_DataStorage = dataStorage;
  m_BaseNode = baseNode;

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*>
          (this, &QmitkUSZonesDataModel::AddNode));

    m_DataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*>
          (this, &QmitkUSZonesDataModel::RemoveNode));

    m_DataStorage->ChangedNodeEvent.AddListener(
          mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*>
            (this, &QmitkUSZonesDataModel::ChangeNode));

    m_DataStorage->InteractorChangedNodeEvent.AddListener(
          mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*>
            (this, &QmitkUSZonesDataModel::ChangeNode));
  }
}

void QmitkUSZonesDataModel::AddNode(const mitk::DataNode* node)
{
  // get source node of given node and test if m_BaseNode is a source node
  mitk::DataStorage::SetOfObjects::ConstPointer sourceNodes = m_DataStorage->GetSources(node);
  mitk::DataStorage::SetOfObjects::ConstIterator baseNodeIt = sourceNodes->Begin();
  while ( baseNodeIt != sourceNodes->End() && baseNodeIt->Value() != m_BaseNode ) { ++baseNodeIt; }

  // only nodes below m_BaseNode should be added to the model
  if ( baseNodeIt == sourceNodes->End() ) { return; }

  this->insertRow(this->rowCount());
  m_ZoneNodes.at(m_ZoneNodes.size()-1) = const_cast<mitk::DataNode*>(node);

  /*mitk::DataStorage::SetOfObjects::ConstPointer zoneNodes =
      m_DataStorage->GetSubset(mitk::NodePredicateSource::New(m_BaseNode, false, m_DataStorage));*/
}

void QmitkUSZonesDataModel::RemoveNode(const mitk::DataNode* node)
{
  // find index of the given node in the nodes vector
  unsigned int index = 0;
  DataNodeVector::iterator current = m_ZoneNodes.begin();
  while (current != m_ZoneNodes.end())
  {
    if ( *current == node ) { break; }
    ++index;
    ++current;
  }

  // remove node from model if it was found
  if ( current != m_ZoneNodes.end() )
  {
    // remove node from the model and make sure that there will be no
    // recursive removing calls (as removing node from data storage inside
    // removeRows function will lead into another call of RemoveNode).
    this->removeRows(index, 1, QModelIndex(), false);
  }
}

void QmitkUSZonesDataModel::ChangeNode(const mitk::DataNode* node)
{

  DataNodeVector::iterator oldNodeIt = find (m_ZoneNodes.begin(), m_ZoneNodes.end(), node);
  if (oldNodeIt == m_ZoneNodes.end()) { return; };

  // get row of the changed node and emit signal that the data of this row changed
  unsigned int row = oldNodeIt - m_ZoneNodes.begin();
  emit dataChanged(this->index(row, 0), this->index(row, this->columnCount()));
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
      float floatValue;
      if ( curNode->GetFloatProperty(DataNodePropertySize, floatValue) ) { return floatValue; }
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
      curNode->SetFloatProperty(DataNodePropertySize, value.toFloat());

      mitk::Point3D origin = curNode->GetData()->GetGeometry()->GetOrigin();
      curNode->SetData(this->MakeSphere(curNode, value.toFloat()));
      curNode->GetData()->GetGeometry()->SetOrigin(origin);

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
  return this->removeRows(row, count, parent, true);
}

bool QmitkUSZonesDataModel::removeRows ( int row, int count, const QModelIndex & parent, bool removeFromDataStorage )
{
  if ( static_cast<unsigned int>(row+count) > m_ZoneNodes.size() ) { return false; }

  this->beginRemoveRows(parent, row, row+count-1);

  for ( int n = count-1; n >= 0; --n )
  {
    DataNodeVector::iterator it = m_ZoneNodes.begin()+row+n;
    mitk::DataNode::Pointer curNode = *it;
    m_ZoneNodes.erase(it);

    if ( removeFromDataStorage && m_DataStorage.IsNotNull() )
    {
      m_DataStorage->Remove(curNode);
    }
  }

  this->endRemoveRows();

  return true;
}

mitk::Surface::Pointer QmitkUSZonesDataModel::MakeSphere(const mitk::DataNode::Pointer dataNode, mitk::ScalarType radius) const
{
  mitk::Surface::Pointer zone = mitk::Surface::New();

  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius( radius );
  vtkData->SetCenter(0,0,0);
  vtkData->Update();
  zone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  return zone;
}
