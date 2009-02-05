#include "QmitkDataStorageTableModel.h"

#include "mitkNodePredicateBase.h"

QmitkDataStorageTableModel::QmitkDataStorageTableModel(mitk::DataStorage::Pointer _DataStorage
                                                       , mitk::NodePredicateBase* _Predicate
                                                       , QObject* parent )
: QAbstractTableModel(parent)
, m_Predicate(_Predicate)
, m_ProcessesEvents(false)
{
  this->setDataStorage(_DataStorage);
}

void QmitkDataStorageTableModel::reset()
{
  if(m_Predicate != 0)
    // get subset
    m_NodeSet = m_DataStorage->GetSubset(*m_Predicate);
  // if predicate is NULL, select all nodes
  else
  {
    m_NodeSet = m_DataStorage->GetAll();
    // remove ghost root node
  }
    
  // emit reset signals
  QAbstractItemModel::reset();
}

QmitkDataStorageTableModel::~QmitkDataStorageTableModel()
{
  // remove event listeners
  this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
    , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeAdded ) );

  this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
    , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeRemoved ) );

  delete m_Predicate;
  m_Predicate = 0;
}

void QmitkDataStorageTableModel::setPredicate(mitk::NodePredicateBase* _Predicate)
{
  // ensure that a new predicate is set in order to avoid unnecessary changed events
  if(m_Predicate != _Predicate)
  {
    m_Predicate = _Predicate;
    this->reset();
  }
}

mitk::NodePredicateBase* QmitkDataStorageTableModel::getPredicate() const
{
  return m_Predicate;
}

void QmitkDataStorageTableModel::setDataStorage(mitk::DataStorage::Pointer _DataStorage)
{
  if(m_DataStorage != _DataStorage)
  {
    // if a data storage was set before remove old event listeners
    if(m_DataStorage.IsNotNull())
    {
      this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeRemoved ) );
    }

    m_DataStorage = _DataStorage;

    if(m_DataStorage.IsNotNull())
    {
      // subscribe for node added/removed events
      this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeAdded ) );

      this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTableModel::NodeRemoved ) );
    }

    this->reset();
  }
}

const mitk::DataStorage::Pointer QmitkDataStorageTableModel::getDataStorage() const
{
  return m_DataStorage;
}

int QmitkDataStorageTableModel::rowCount(const QModelIndex &parent) const
{
  return m_NodeSet->size();
}

int QmitkDataStorageTableModel::columnCount(const QModelIndex &parent) const
{
  // only display "Name"-column so far
  return 1;
}

QVariant QmitkDataStorageTableModel::data(const QModelIndex &index, int role) const
{
  QVariant data;

  if (index.isValid())
  {
    if (role == Qt::DisplayRole) 
    {
      mitk::DataTreeNode::Pointer node = m_NodeSet->ElementAt(index.row());
      std::string nodeName = node->GetName();
      if(nodeName.empty())
        nodeName = "unnamed";      

      data = nodeName.c_str();
    }
  }
  return data;
}

mitk::DataTreeNode::Pointer QmitkDataStorageTableModel::getNode(const QModelIndex &index) const
{
  mitk::DataTreeNode::Pointer node;

  if(index.isValid())
  {
    node = m_NodeSet->ElementAt(index.row());
  }

  return node;
}

QVariant QmitkDataStorageTableModel::headerData(int section, Qt::Orientation orientation,
  int role) const
{
  QVariant headerData;

  // show only horizontal header
  if ( role == Qt::DisplayRole )
  {
    if( orientation == Qt::Horizontal )
    {
      // first column: "Name"
      if(section == 0)
        headerData = "Name";
    }
    else if( orientation == Qt::Vertical )
    {
      // show numbers for rows
      headerData = section+1;
    }
  }

  return headerData;
}

bool QmitkDataStorageTableModel::setData(const QModelIndex &index, const QVariant &value,
  int role)
{
  bool setData = false;

  // no editing so far
  return setData;
}

Qt::ItemFlags QmitkDataStorageTableModel::flags(const QModelIndex &index) const
{
  // no editing so far, return default (enabled, selectable)
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  return flags;
}

void QmitkDataStorageTableModel::NodeAdded( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_ProcessesEvents)
  {
    m_ProcessesEvents = true;

    // check if node should be added to the model
    bool addNode = true;
    if(m_Predicate && !m_Predicate->CheckNode(node))
      addNode = false;

    // currently we dont have the possibility to add single nodes to the node set
    if(addNode)
      this->reset();

    m_ProcessesEvents = false;
  }
}

void QmitkDataStorageTableModel::NodeRemoved( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_ProcessesEvents)
  {
    m_ProcessesEvents = true;

    bool removeNode = false;    
    // check if node is contained in current list, if yes: reset model
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = m_NodeSet->Begin()
      ; nodeIt != m_NodeSet->End(); nodeIt++)  // for each node
    {
      if( nodeIt.Value() == node )
      {
        removeNode = true;
        break;
      }
    }

    if(removeNode)
      this->reset();

    m_ProcessesEvents = false;
  }
}