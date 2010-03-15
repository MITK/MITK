#include "QmitkDataStorageTableModel.h"

//# Own includes
#include "mitkNodePredicateBase.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "QmitkEnums.h"
#include "QmitkCustomVariants.h"
#include <QmitkNodeDescriptorManager.h>

//# Toolkit includes
#include <itkCommand.h>
#include <QIcon>

//#CTORS/DTOR
QmitkDataStorageTableModel::QmitkDataStorageTableModel(mitk::DataStorage::Pointer _DataStorage
                                                       , mitk::NodePredicateBase* _Predicate
                                                       , QObject* parent )
: QAbstractTableModel(parent)
, m_DataStorage(0)
, m_Predicate(0)
, m_BlockEvents(false)
, m_SortDescending(false)
{
  this->SetPredicate(_Predicate);
  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageTableModel::~QmitkDataStorageTableModel()
{
  // set data storage 0 to remove event listeners
  this->SetDataStorage(0);
}

//# Public GETTER
const mitk::DataStorage::Pointer QmitkDataStorageTableModel::GetDataStorage() const
{
  return m_DataStorage.GetPointer();
}

mitk::NodePredicateBase::Pointer QmitkDataStorageTableModel::GetPredicate() const
{
  return m_Predicate;
}

mitk::DataNode::Pointer QmitkDataStorageTableModel::GetNode( const QModelIndex &index ) const
{
  mitk::DataNode::Pointer node;

  if(index.isValid())
  {
    node = m_NodeSet.at(index.row());
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
      else if(section == 1)
        headerData = "Data Type";
      else if(section == 2)
        headerData = "Visibility";
    }
    else if( orientation == Qt::Vertical )
    {
      // show numbers for rows
      headerData = section+1;
    }
  }

  return headerData;
}

Qt::ItemFlags QmitkDataStorageTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  // name & visibility is editable
  if (index.column() == 0)
  {
    flags |= Qt::ItemIsEditable;
  }
  else if (index.column() == 2)
  {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

int QmitkDataStorageTableModel::rowCount(const QModelIndex &) const
{
  return m_NodeSet.size();
}

int QmitkDataStorageTableModel::columnCount(const QModelIndex &) const
{
  // show name, type and visible columnn
  int columns = 3;
  return columns;
}

QVariant QmitkDataStorageTableModel::data(const QModelIndex &index, int role) const
{
  QVariant data;

  if (index.isValid() && !m_NodeSet.empty())
  {
    mitk::DataNode::Pointer node = m_NodeSet.at(index.row());

    std::string nodeName = node->GetName();
    if(nodeName.empty())
      nodeName = "unnamed";

    // get name
    if(index.column() == 0)
    {
      // get name of node (may also be edited)
      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
        data = nodeName.c_str();
      }
      else if (role == QmitkDataNodeRole)
      {
        data = QVariant::fromValue(node);
      }
    }
    else if (index.column() == 1)
    {

      QmitkNodeDescriptor* nodeDescriptor 
        = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(node);

      // get type property of mitk::BaseData
      if (role == Qt::DisplayRole)
      {

        data = nodeDescriptor->GetClassName();
      }
      // show some nice icons for datatype
      else if(role == Qt::DecorationRole)
      {
        data = nodeDescriptor->GetIcon();
      }
    }
    else if (index.column() == 2)
    {
      // get visible property of mitk::BaseData
      bool visibility = false;

      if(node->GetVisibility(visibility, 0) && role == Qt::CheckStateRole)
      {
        data = (visibility ? Qt::Checked : Qt::Unchecked);
      } // node->GetVisibility(visibility, 0) && role == Qt::CheckStateRole

    } // index.column() == 2

  } // index.isValid() && !m_NodeSet.empty()
  return data;
}

//# Public SETTERS
void QmitkDataStorageTableModel::SetPredicate( mitk::NodePredicateBase* _Predicate )
{
  // ensure that a new predicate is set in order to avoid unnecessary changed events
  if(m_Predicate != _Predicate)
  {
    m_Predicate = _Predicate;
    this->Reset();
  }
}

void QmitkDataStorageTableModel::SetDataStorage( mitk::DataStorage::Pointer _DataStorage )
{
  // only proceed if we have a new datastorage
  if(m_DataStorage.GetPointer() != _DataStorage.GetPointer())
  {
    // if a data storage was set before remove old event listeners
    if(m_DataStorage != 0)
    {
      this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTableModel::AddNode ) );

      this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTableModel::RemoveNode ) );
    }

    // set new data storage
    m_DataStorage = _DataStorage.GetPointer();

    // if new storage is not 0 subscribe for events
    if(m_DataStorage != 0)
    {
      // subscribe for node added/removed events
      this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTableModel::AddNode ) );

      this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTableModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTableModel::RemoveNode ) );
    }

    // Reset model (even if datastorage is 0->will be checked in Reset())
    this->Reset();
  }
}


void QmitkDataStorageTableModel::AddNode( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_BlockEvents)
  {
    // if we have a predicate, check node against predicate first
    if(m_Predicate.IsNotNull() && !m_Predicate->CheckNode(node))
      return;

    // dont add nodes without data (formerly known as helper objects)
    if(node->GetData() == 0)
      return;

    // create listener commands to listen to changes in the name or the visibility of the node
    itk::MemberCommand<QmitkDataStorageTableModel>::Pointer propertyModifiedCommand
      = itk::MemberCommand<QmitkDataStorageTableModel>::New();
    propertyModifiedCommand->SetCallbackFunction(this, &QmitkDataStorageTableModel::PropertyModified);

    mitk::BaseProperty* tempProperty = 0;

    // add listener for properties
    tempProperty = node->GetProperty("visible");
    if(tempProperty)
      m_VisiblePropertyModifiedObserverTags[tempProperty]
        = tempProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);

    tempProperty = node->GetProperty("name");
    if(tempProperty)
      m_NamePropertyModifiedObserverTags[tempProperty]
        = tempProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);

    // emit beginInsertRows event
    beginInsertRows(QModelIndex(), m_NodeSet.size(), m_NodeSet.size());

    // add node
    m_NodeSet.push_back(const_cast<mitk::DataNode*>(node));

    // emit endInsertRows event
    endInsertRows();
  }
}

void QmitkDataStorageTableModel::RemoveNode( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown
  if(!m_BlockEvents)
  {
    // find corresponding node
    std::vector<mitk::DataNode*>::iterator nodeIt
      = std::find(m_NodeSet.begin(), m_NodeSet.end(), node);

    if(nodeIt != m_NodeSet.end())
    {
      // now: remove listeners for name property ...
      mitk::BaseProperty* tempProperty = 0;

      tempProperty = (*nodeIt)->GetProperty("visible");
      if(tempProperty)
        tempProperty->RemoveObserver(m_VisiblePropertyModifiedObserverTags[tempProperty]);
      m_VisiblePropertyModifiedObserverTags.erase(tempProperty);

      // ... and visibility property
      tempProperty = (*nodeIt)->GetProperty("name");
      if(tempProperty)
        tempProperty->RemoveObserver(m_NamePropertyModifiedObserverTags[tempProperty]);
      m_NamePropertyModifiedObserverTags.erase(tempProperty);

      // get an index from iterator
      int row = std::distance(m_NodeSet.begin(), nodeIt);

      // emit beginRemoveRows event (QModelIndex is empty because we dont have a tree model)
      this->beginRemoveRows(QModelIndex(), row, row);

      // remove node
      m_NodeSet.erase(nodeIt);

      // emit endRemoveRows event
      endRemoveRows();
    }
  }
}

void QmitkDataStorageTableModel::PropertyModified( const itk::Object *caller, const itk::EventObject & )
{
  if(!m_BlockEvents)
  {
    // get modified property
    const mitk::BaseProperty* modifiedProperty = dynamic_cast<const mitk::BaseProperty*>(caller);

    if(modifiedProperty)
    {
      // find node that holds the modified property
      int row = -1;
      int column = -1;

      std::vector<mitk::DataNode*>::iterator it;
      mitk::BaseProperty* visibilityProperty = 0;
      mitk::BaseProperty* nameProperty = 0;

      // search for property that changed and emit datachanged on the corresponding ModelIndex
      for(it=m_NodeSet.begin(); it!=m_NodeSet.end(); it++)
      {
        // check for the visible property or the name property
        visibilityProperty = (*it)->GetProperty("visible");
        if(modifiedProperty == visibilityProperty)
        {
          column = 2;
          break;
        }

        nameProperty = (*it)->GetProperty("name");
        if(modifiedProperty == nameProperty)
        {
          column = 0;
          break;
        }
      }

      // if we have the property we have a valid iterator
      if( it != m_NodeSet.end() )
        row = std::distance(m_NodeSet.begin(), it);

      // now emit the dataChanged signal
      QModelIndex indexOfChangedProperty = index(row, column);
      emit dataChanged(indexOfChangedProperty, indexOfChangedProperty);
    }
  }
}

bool QmitkDataStorageTableModel::setData(const QModelIndex &index, const QVariant &value,
  int role)
{
  bool noErr = false;

  if (index.isValid() && (role == Qt::EditRole || role == Qt::CheckStateRole))
  {
    // any change events produced here should not be caught in this class
    // --> set m_BlockEvents to true
    m_BlockEvents = true;

    mitk::DataNode::Pointer node = m_NodeSet.at(index.row());

    if(index.column() == 0)
    {
      node->SetStringProperty("name", value.toString().toStdString().c_str());
    }
    else if(index.column() == 2)
    {
      node->SetBoolProperty("visible", (value.toInt() == Qt::Checked ? true : false));
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    // inform listeners about changes
    emit dataChanged(index, index);

    m_BlockEvents = false;
    noErr = true;
  }

  return noErr;
}

//#Protected SETTER
void QmitkDataStorageTableModel::Reset()
{
  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet;

  // remove all nodes now (dont use iterators because removing elements
  // would invalidate the iterator)
  // start at the last element: first in, last out
  unsigned int i = m_NodeSet.size();
  while(!m_NodeSet.empty())
  {
    --i;
    this->RemoveNode(m_NodeSet.at(i));
  }

  // normally now everything should be empty->just to be sure
  // erase all arrays again
  m_NamePropertyModifiedObserverTags.clear();
  m_VisiblePropertyModifiedObserverTags.clear();
  m_NodeSet.clear();

  // the whole reset depends on the fact if a data storage is set or not
  if(m_DataStorage.IsNotNull())
  {
    if(m_Predicate.IsNotNull())
      // get subset
      _NodeSet = m_DataStorage->GetSubset(m_Predicate);
    // if predicate is NULL, select all nodes
    else
    {
      _NodeSet = m_DataStorage->GetAll();
      // remove ghost root node
    }

    // finally add all nodes to the model
    for(mitk::DataStorage::SetOfObjects::const_iterator it=_NodeSet->begin(); it!=_NodeSet->end()
      ; it++)
    {
      // save node
      this->AddNode(*it);
    }

  }
}

void QmitkDataStorageTableModel::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder */ )
{
  bool sortDescending = (order == Qt::DescendingOrder) ? true: false;

  // do not sort twice !!! (dont know why, but qt calls this func twice. STUPID!)
/*
  if(sortDescending != m_SortDescending)
  {*/

    //m_SortDescending = sortDescending;

    DataNodeCompareFunction::CompareCriteria _CompareCriteria
      = DataNodeCompareFunction::CompareByName;

    DataNodeCompareFunction::CompareOperator _CompareOperator
      = sortDescending ? DataNodeCompareFunction::Greater: DataNodeCompareFunction::Less;

    if(column == 1)
      _CompareCriteria = DataNodeCompareFunction::CompareByClassName;

    else if(column == 2)
      _CompareCriteria = DataNodeCompareFunction::CompareByVisibility;


    DataNodeCompareFunction compareFunc(_CompareCriteria, _CompareOperator);
    std::sort(m_NodeSet.begin(), m_NodeSet.end(), compareFunc);

    QAbstractTableModel::reset();
  //}
}

std::vector<mitk::DataNode*> QmitkDataStorageTableModel::GetNodeSet() const
{
  return m_NodeSet;
}

QmitkDataStorageTableModel::DataNodeCompareFunction::DataNodeCompareFunction( CompareCriteria _CompareCriteria
                                                                                          , CompareOperator _CompareOperator )
                                                                                          : m_CompareCriteria(_CompareCriteria)
                                                                                          , m_CompareOperator(_CompareOperator)
{
}

bool QmitkDataStorageTableModel::DataNodeCompareFunction::operator()
( const mitk::DataNode::Pointer& _Left
 , const mitk::DataNode::Pointer& _Right ) const
{
  switch(m_CompareCriteria)
  {
  case CompareByClassName:
    if(m_CompareOperator == Less)
      return (_Left->GetData()->GetNameOfClass() < _Right->GetData()->GetNameOfClass() );
    else
      return (_Left->GetData()->GetNameOfClass()  > _Right->GetData()->GetNameOfClass() );
    break;

  case CompareByVisibility:
    {

      bool _LeftVisibility = false;
      bool _RightVisibility = false;
      _Left->GetVisibility(_LeftVisibility, 0);
      _Right->GetVisibility(_RightVisibility, 0);

      if(m_CompareOperator == Less)
        return (_LeftVisibility < _RightVisibility);
      else
        return (_LeftVisibility > _RightVisibility);
    }
    break;

    // CompareByName:
  default:
    if(m_CompareOperator == Less)
      return (_Left->GetName() < _Right->GetName());
    else
      return (_Left->GetName() > _Right->GetName());
    break;
  }
}

