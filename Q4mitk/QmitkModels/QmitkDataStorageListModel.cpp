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

QmitkDataStorageListModel::QmitkDataStorageListModel(mitk::DataStorage::Pointer dataStorage
                                                     , mitk::NodePredicateBase* pred, QObject* parent)
 : QAbstractListModel(parent), m_DataStorage(0), m_NodePredicate(pred), m_ProcessesEvents(false)
{
  if(dataStorage.IsNull())
    dataStorage = mitk::DataStorage::GetInstance();
  
  this->SetDataStorage(dataStorage);
}

QmitkDataStorageListModel::~QmitkDataStorageListModel()
{
  // set data storage to 0 so that event listener get removed
  SetDataStorage(0);

  if (m_NodePredicate) delete m_NodePredicate;
}

void QmitkDataStorageListModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if( m_DataStorage != dataStorage)
  {
    // remove old listeners
    if(m_DataStorage.IsNotNull())
    {
      this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeRemoved ) );
    }

    m_DataStorage = dataStorage;

    // remove event listeners
    if(m_DataStorage.IsNotNull())
    {
      // subscribe for node added/removed events
      this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeRemoved ) );
    }

    // reset model
    reset();
  }
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
  return QVariant("Nodes");
}

int QmitkDataStorageListModel::rowCount(const QModelIndex& parent) const
{
  return m_DataNodes->Size();
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataStorageListModel::GetDataNodes() const
{
  return m_DataNodes;
}

mitk::DataStorage::Pointer QmitkDataStorageListModel::GetDataStorage() const
{
  return m_DataStorage;
}


void QmitkDataStorageListModel::SetPredicate(mitk::NodePredicateBase* pred)
{
  m_NodePredicate = pred;
  reset();
  QAbstractListModel::reset();
}

mitk::NodePredicateBase* QmitkDataStorageListModel::GetPredicate() const
{
  return m_NodePredicate;
}

void QmitkDataStorageListModel::reset()
{
  if(m_DataStorage.IsNotNull())
  {    
    if (m_NodePredicate)
      m_DataNodes = m_DataStorage->GetSubset(*m_NodePredicate);
    else
      m_DataNodes = m_DataStorage->GetAll();    

    //QAbstractListModel::reset();
  }
}

void QmitkDataStorageListModel::NodeAdded( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_ProcessesEvents)
  {
    m_ProcessesEvents = true;

    // check if node should be added to the model
    bool addNode = true;
    if(m_NodePredicate && !m_NodePredicate->CheckNode(node))
      addNode = false;

    // currently we dont have the possibility to add single nodes to the node set
    if(addNode)
    {
      beginInsertRows(QModelIndex(), m_DataNodes->Size(), m_DataNodes->Size());

      reset();

      endInsertRows();
    }

//     QModelIndex index = QAbstractListModel::createIndex(m_DataNodes->Size(), 0, 0);
//     emit QAbstractListModel::dataChanged(index, index);

    m_ProcessesEvents = false;
  }
}

void QmitkDataStorageListModel::NodeRemoved( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_ProcessesEvents)
  {
    m_ProcessesEvents = true;

    int row = -1;
    bool removeNode = false;    
    // check if node is contained in current list, if yes: reset model
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = m_DataNodes->Begin()
      ; nodeIt != m_DataNodes->End(); nodeIt++, row++)  // for each node
    {
      if( nodeIt.Value() == node )
      {
        removeNode = true;
        break;
      }
    }

    if(removeNode)
    {
      beginRemoveRows(QModelIndex(), row, row);

      reset();

      endRemoveRows();
    }

    //QModelIndex index = QAbstractListModel::createIndex(row, 0, 0);
    //emit QAbstractListModel::dataChanged(index, index);

    m_ProcessesEvents = false;
  }
}