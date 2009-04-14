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

//# Own includes
// mitk
#include "mitkStringProperty.h"

//# Toolkit includes
// itk
#include "itkCommand.h"

QmitkDataStorageListModel::QmitkDataStorageListModel(mitk::DataStorage::Pointer dataStorage
                                                     , mitk::NodePredicateBase* pred, QObject* parent)
 : QAbstractListModel(parent), m_NodePredicate(0), m_DataStorage(0), m_BlockEvents(false)
{  
  this->SetPredicate(pred);
  this->SetDataStorage(dataStorage);
}

QmitkDataStorageListModel::~QmitkDataStorageListModel()
{
  // set data storage to 0 so that event listener get removed
  this->SetDataStorage(0);

  if (m_NodePredicate) delete m_NodePredicate;
}

void QmitkDataStorageListModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if( m_DataStorage != dataStorage)
  {
    // remove old listeners
    if(m_DataStorage != 0)
    {
      this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeRemoved ) );

      // remove delete observer
      m_DataStorage->RemoveObserver(m_DataStorageDeleteObserverTag);
      // this is good coding style ! reset variables whenever they are not used anymore.
      m_DataStorageDeleteObserverTag = 0;
    }

    m_DataStorage = dataStorage;

    // remove event listeners
    if(m_DataStorage != 0)
    {
      // subscribe for node added/removed events
      this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageListModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageListModel::NodeRemoved ) );

      // add itk delete listener on datastorage
      itk::MemberCommand<QmitkDataStorageListModel>::Pointer deleteCommand =
        itk::MemberCommand<QmitkDataStorageListModel>::New();
      deleteCommand->SetCallbackFunction(this, &QmitkDataStorageListModel::OnDelete);
      // add observer
      m_DataStorageDeleteObserverTag = m_DataStorage->AddObserver(itk::DeleteEvent(), deleteCommand);
    }

    // reset model
    reset();
  }
}

Qt::ItemFlags QmitkDataStorageListModel::flags(const QModelIndex&) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QmitkDataStorageListModel::data(const QModelIndex& index, int role) const
{
  if(index.isValid())
  {
    switch ( role )
    {
      case Qt::DisplayRole:
      {
        const mitk::DataTreeNode* node = m_DataNodes.at(index.row());
        std::string name = node->GetName();
        return QVariant(QString::fromStdString(name));
      }
      break;

    }
  } // index.isValid()

  return QVariant();
}

QVariant QmitkDataStorageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant("Nodes");
}

int QmitkDataStorageListModel::rowCount(const QModelIndex& parent) const
{
  return m_DataNodes.size();
}

std::vector<mitk::DataTreeNode*> QmitkDataStorageListModel::GetDataNodes() const
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
  if(m_DataStorage != 0)
  {    
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects;

    if (m_NodePredicate)
      setOfObjects = m_DataStorage->GetSubset(*m_NodePredicate);
    else
      setOfObjects = m_DataStorage->GetAll();   

    // remove all observes
    unsigned int i = 0;
    for(std::vector<mitk::DataTreeNode*>::iterator it=m_DataNodes.begin()
      ; it!=m_DataNodes.end()
      ; ++it, ++i)
    {
      (*it)->RemoveObserver(m_DataNodesModifiedObserversTags[i]);     
    }

    // clear vector with nodes
    m_DataNodesModifiedObserversTags.clear();
    m_DataNodes.clear();
    itk::MemberCommand<QmitkDataStorageListModel>::Pointer modifiedCommand;
    // copy all selected nodes the vector
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt, ++i)  // for each node
    {
      // add modified observer
      modifiedCommand = itk::MemberCommand<QmitkDataStorageListModel>::New();
      modifiedCommand->SetCallbackFunction(this, &QmitkDataStorageListModel::OnModified);
      m_DataNodesModifiedObserversTags.push_back( m_DataStorage->AddObserver(itk::ModifiedEvent(), modifiedCommand) );
      m_DataNodes.push_back( nodeIt.Value().GetPointer());

    } // for

  } // m_DataStorage != 0

} // reset()

void QmitkDataStorageListModel::NodeAdded( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_BlockEvents)
  {
    m_BlockEvents = true;

    // check if node should be added to the model
    bool addNode = true;
    if(m_NodePredicate && !m_NodePredicate->CheckNode(node))
      addNode = false;

    if(addNode)
    {
      beginInsertRows(QModelIndex(), m_DataNodes.size(), m_DataNodes.size());
      //reset();
      m_DataNodes.push_back(const_cast<mitk::DataTreeNode*>(node));
      endInsertRows();
    }

    m_BlockEvents = false;
  }
}

void QmitkDataStorageListModel::NodeRemoved( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_BlockEvents)
  {
    m_BlockEvents = true;

    int row = -1;
    //bool removeNode = false;    
    // check if node is contained in current list, if yes: reset model
    for (std::vector<mitk::DataTreeNode*>::const_iterator nodeIt = m_DataNodes.begin()
      ; nodeIt != m_DataNodes.end(); nodeIt++)  // for each node
    {
      row++;
      if( (*nodeIt) == node )
      {
        // node found, remove it
        beginRemoveRows(QModelIndex(), row, row);
        m_DataNodes.erase(std::find(m_DataNodes.begin(), m_DataNodes.end(), (*nodeIt)));
        endRemoveRows();
        break;
      }
    }

    m_BlockEvents = false;
  }
}

void QmitkDataStorageListModel::OnModified( const itk::Object *caller, const itk::EventObject &event )
{
  if(m_BlockEvents) return;

  const mitk::DataTreeNode* modifiedNode = dynamic_cast<const mitk::DataTreeNode*>(caller);
  if(modifiedNode)
  {
    int row = std::distance(std::find(m_DataNodes.begin(), m_DataNodes.end(), modifiedNode), m_DataNodes.end());
    QModelIndex indexOfChangedProperty = index(row, 1);

    emit dataChanged(indexOfChangedProperty, indexOfChangedProperty);
  }
}

void QmitkDataStorageListModel::OnDelete( const itk::Object *caller, const itk::EventObject &event )
{
  if(m_BlockEvents) return;

  const mitk::DataStorage* dataStorage = dynamic_cast<const mitk::DataStorage*>(caller);
  if(dataStorage)
  {
    // set datastorage to 0 -> empty model
    this->SetDataStorage(0);
  }
}

mitk::DataTreeNode::Pointer QmitkDataStorageListModel::getNode( const QModelIndex &index ) const
{
  mitk::DataTreeNode::Pointer node;

  if(index.isValid())
  {
    node = m_DataNodes.at(index.row());
  }

  return node;
}