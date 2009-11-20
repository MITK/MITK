/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date: 2008-08-13 16:56:36 +0200 (Mi, 13 Aug 2008) $
 Version:   $Revision: 14972 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#include <mitkStringProperty.h>
#include <mitkNodePredicateFirstLevel.h>
#include <mitkNodePredicateAND.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include "QmitkDataStorageTreeModel.h"
#include "QmitkNodeDescriptorManager.h"
#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

#include <QIcon>
#include <QMimeData>
#include <QTextStream>

QmitkDataStorageTreeModel::QmitkDataStorageTreeModel( mitk::DataStorage* _DataStorage, QObject* parent )
: QAbstractItemModel(parent)
, m_DataStorage(0)
, m_Root(0)
{
  mitk::NodePredicateData::Pointer dataIsNull = mitk::NodePredicateData::New(0);
  mitk::NodePredicateNOT::Pointer dataIsNotNull 
    = mitk::NodePredicateNOT::New(dataIsNull);// Show only nodes that really contain dat

  mitk::NodePredicateProperty::Pointer isHelperObject = mitk::NodePredicateProperty::New("helper object"
    , mitk::BoolProperty::New(true));

  mitk::NodePredicateNOT::Pointer isNotHelperObject 
    = mitk::NodePredicateNOT::New(isHelperObject);// Show only nodes that really contain dat

  mitk::NodePredicateAND::Pointer dataIsNotNullAndIsNotHelperObject = mitk::NodePredicateAND::New(dataIsNotNull,
    isNotHelperObject);

  m_Predicate = dataIsNotNullAndIsNotHelperObject;

  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageTreeModel::~QmitkDataStorageTreeModel()
{
  // set data storage to 0 = remove all listeners
  this->SetDataStorage(0);
  m_Root->Delete(); m_Root = 0;
}

mitk::DataTreeNode::Pointer QmitkDataStorageTreeModel::GetNode( const QModelIndex &index ) const
{
  return this->TreeItemFromIndex(index)->GetDataTreeNode();
}

const mitk::DataStorage::Pointer QmitkDataStorageTreeModel::GetDataStorage() const
{
  return m_DataStorage.GetPointer();
}

QModelIndex QmitkDataStorageTreeModel::index( int row, int column, const QModelIndex & parent ) const
{
  TreeItem* parentItem;

  if (!parent.isValid())
    parentItem = m_Root;
  else
    parentItem = static_cast<TreeItem*>(parent.internalPointer());

  TreeItem *childItem = parentItem->GetChild(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();


}

int QmitkDataStorageTreeModel::rowCount(const QModelIndex &parent) const
{
  TreeItem *parentTreeItem = this->TreeItemFromIndex(parent);
  return parentTreeItem->GetChildCount();
}


Qt::ItemFlags QmitkDataStorageTreeModel::flags( const QModelIndex& index ) const
{
  if (index.isValid())
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
      | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  else
    return Qt::ItemIsDropEnabled;
}

int QmitkDataStorageTreeModel::columnCount( const QModelIndex & parent /*= QModelIndex() */ ) const
{
  return 1;
}

QModelIndex QmitkDataStorageTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  TreeItem *childItem = this->TreeItemFromIndex(index);
  TreeItem *parentItem = childItem->GetParent();

  if (parentItem == m_Root)
    return QModelIndex();

  return this->createIndex(parentItem->GetIndex(), 0, parentItem);
}

QmitkDataStorageTreeModel::TreeItem* QmitkDataStorageTreeModel::TreeItemFromIndex( const QModelIndex &index ) const
{
  if (index.isValid())
    return static_cast<TreeItem *>(index.internalPointer());
  else 
    return m_Root;
}
Qt::DropActions QmitkDataStorageTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

bool QmitkDataStorageTreeModel::dropMimeData(const QMimeData *data,
                                     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  if(data->hasFormat("application/x-qabstractitemmodeldatalist"))
  {
    QString arg = QString(data->data("application/x-qabstractitemmodeldatalist").data());
    long val = arg.toLong();
    TreeItem* draggedItem = static_cast<TreeItem *>((void*)val);
    TreeItem* dropItem = this->TreeItemFromIndex(parent);
    TreeItem* parentItem = dropItem->GetParent();
    if(dropItem == m_Root) // item was dropped onto empty space 
      parentItem = m_Root;

    if(draggedItem != dropItem && draggedItem->GetParent() == parentItem) // dragging is only allowed within the same parent
    {
      QModelIndex parentModelIndex = this->IndexFromTreeItem(parentItem);

      // remove dragged item
      this->beginRemoveRows(parentModelIndex, draggedItem->GetIndex(), draggedItem->GetIndex());

      parentItem->RemoveChild(draggedItem);

      endRemoveRows();

      // now insert it again at the drop item position
      int index = parentItem->IndexOfChild(dropItem);
      if(dropItem == m_Root)
        index = parentItem->GetChildCount();

      beginInsertRows(parentModelIndex, index, index);

      // add node
      parentItem->InsertChild( draggedItem, index );

      // emit endInsertRows event
      endInsertRows();

      this->AdjustLayerProperty();
    }
  }
  return false;
}


QMimeData * QmitkDataStorageTreeModel::mimeData(const QModelIndexList & indexes) const{
  QMimeData * ret = new QMimeData;
  long a = reinterpret_cast<long>(indexes.at(0).internalPointer());
  
  QString result;
  QTextStream(&result) << a;
  ret->setData("application/x-qabstractitemmodeldatalist", QByteArray(result.toAscii()));
  return ret;
}

QVariant QmitkDataStorageTreeModel::data( const QModelIndex & index, int role ) const
{
  mitk::DataTreeNode* dataNode = this->TreeItemFromIndex(index)->GetDataTreeNode();

  // get name of treeItem (may also be edited)
  QString nodeName = QString::fromStdString(dataNode->GetName());
  if(nodeName.isEmpty())
    nodeName = "unnamed";

  if (role == Qt::DisplayRole)
    return nodeName;
  else if(role == Qt::ToolTipRole)
    return nodeName;
  else if(role == Qt::DecorationRole)
  {
    QmitkNodeDescriptor* nodeDescriptor 
      = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(dataNode);
    return nodeDescriptor->GetIcon();
  }
  else if(role == Qt::CheckStateRole)
  {
    return dataNode->IsVisible(0);
  }
  else if(role == QmitkDataTreeNodeRole)
  {
    return QVariant::fromValue<mitk::DataTreeNode::Pointer>(mitk::DataTreeNode::Pointer(dataNode));
  }

  return QVariant();
}

QVariant QmitkDataStorageTreeModel::headerData(int section,
                                 Qt::Orientation orientation,
                                 int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_Root)
     return QString::fromStdString(m_Root->GetDataTreeNode()->GetName());

   return QVariant();
}

void QmitkDataStorageTreeModel::SetDataStorage( mitk::DataStorage* _DataStorage )
{
  if(m_DataStorage != _DataStorage) // dont take the same again
  {
    if(m_DataStorage.IsNotNull())
    {
      // remove Listener for the data storage itself
      m_DataStorage.ObjectDelete.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const itk::Object*>( this, &QmitkDataStorageTreeModel::SetDataStorageDeleted ) );

      // remove listeners for the nodes
      m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::AddNode ) );

      m_DataStorage->ChangedNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::SetNodeModified ) );

      m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::RemoveNode ) );

    }

    // take over the new data storage
    m_DataStorage = _DataStorage;

    // delete the old root (if necessary, create new)
    if(m_Root)
      m_Root->Delete();
    mitk::DataTreeNode::Pointer rootDataNode = mitk::DataTreeNode::New();
    rootDataNode->SetName("Data Storage");
    m_Root = new TreeItem(rootDataNode, 0);
    this->reset();

    if(m_DataStorage.IsNotNull())
    {
      // add Listener for the data storage itself
      m_DataStorage.ObjectDelete.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const itk::Object*>( this, &QmitkDataStorageTreeModel::SetDataStorageDeleted ) );

      // add listeners for the nodes
      m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::AddNode ) );

      m_DataStorage->ChangedNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::SetNodeModified ) );

      m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageTreeModel::RemoveNode ) );

      mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = m_DataStorage->GetSubset(m_Predicate);

      // finally add all nodes to the model
      for(mitk::DataStorage::SetOfObjects::const_iterator it=_NodeSet->begin(); it!=_NodeSet->end()
        ; it++)
      {
        // save node
        this->AddNode(*it);
      }
    }
  }
}

void QmitkDataStorageTreeModel::SetDataStorageDeleted( const itk::Object* _DataStorage )
{
  this->SetDataStorage(0);
}

void QmitkDataStorageTreeModel::AddNode( const mitk::DataTreeNode* node )
{
  if(node == 0 
    || m_DataStorage.IsNull() 
    || !m_DataStorage->Exists(node) 
    || !m_Predicate->CheckNode(node) 
    || m_Root->Find(node) != 0)
    return;

  // find out if we have a root node
  TreeItem* parentTreeItem = m_Root;
  QModelIndex index;
  mitk::DataTreeNode* parentDataNode = this->GetParentNode(node);

  if(parentDataNode) // no top level data node
  {    
    parentTreeItem = m_Root->Find(parentDataNode); // find the corresponding tree item
    if(!parentTreeItem)
    {
      this->AddNode(parentDataNode);
      parentTreeItem = m_Root->Find(parentDataNode);
      if(!parentTreeItem)
        return;
    }

    // get the index of this parent with the help of the grand parent
    index = this->createIndex(parentTreeItem->GetIndex(), 0, parentTreeItem);
  }
  // emit beginInsertRows event
  beginInsertRows(index, 0, 0);

  // add node
  parentTreeItem->InsertChild(new TreeItem(const_cast<mitk::DataTreeNode*>(node)), 0);

  // emit endInsertRows event
  endInsertRows();

  this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::RemoveNode( const mitk::DataTreeNode* node )
{
  if(!m_Root) return;

  TreeItem* treeItem = m_Root->Find(node);
  if(!treeItem)
    return; // return because there is no treeitem containing this node

  TreeItem* parentTreeItem = treeItem->GetParent();
  QModelIndex parentIndex = this->IndexFromTreeItem(parentTreeItem);

  // emit beginRemoveRows event (QModelIndex is empty because we dont have a tree model)
  this->beginRemoveRows(parentIndex, treeItem->GetIndex(), treeItem->GetIndex());

  // remove node
  std::vector<TreeItem*> children = treeItem->GetChildren();
  delete treeItem;

  // emit endRemoveRows event
  endRemoveRows();

  // move all children of deleted node into its parent
  for ( std::vector<TreeItem*>::iterator it = children.begin()
    ; it != children.end(); it++)
  {
    // emit beginInsertRows event
    beginInsertRows(parentIndex, parentTreeItem->GetChildCount(), parentTreeItem->GetChildCount());

    // add nodes again
    parentTreeItem->AddChild(*it);

    // emit endInsertRows event
    endInsertRows();  	
  }

  this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::SetNodeModified( const mitk::DataTreeNode* node )
{
  TreeItem* treeItem = m_Root->Find(node);
  if(!treeItem)
    return;

  TreeItem* parentTreeItem = treeItem->GetParent();
  // as the root node should not be removed one should always have a parent item
  if(!parentTreeItem)
    return;
  QModelIndex index = this->createIndex(treeItem->GetIndex(), 0, treeItem);

  // now emit the dataChanged signal
  emit dataChanged(index, index);
}

mitk::DataTreeNode* QmitkDataStorageTreeModel::GetParentNode( const mitk::DataTreeNode* node ) const
{
  mitk::DataTreeNode* dataNode = 0;

  mitk::DataStorage::SetOfObjects::ConstPointer _Sources = m_DataStorage->GetSources(node);

  if(_Sources->Size() > 0)
    dataNode = _Sources->front();

  return dataNode;
}

bool QmitkDataStorageTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  mitk::DataTreeNode* dataNode = this->TreeItemFromIndex(index)->GetDataTreeNode();
  if(!dataNode)
    return false;

  if(role == Qt::EditRole && !value.toString().isEmpty())
  {
    dataNode->SetStringProperty("name", value.toString().toStdString().c_str());
  }
  else if(role == Qt::CheckStateRole)
  {
    // Please note: value.toInt() returns 2, independentely from the actual checkstate of the index element. 
	// Therefore the checkstate is being estimated again here. 
    
	QVariant qcheckstate = index.data(Qt::CheckStateRole);
	int checkstate = qcheckstate.toInt();
    bool isVisible = bool(checkstate);
	dataNode->SetVisibility(!isVisible);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  // inform listeners about changes
  emit dataChanged(index, index);
  return true;
}

bool QmitkDataStorageTreeModel::setHeaderData( int section, Qt::Orientation orientation, const QVariant &value, int role /*= Qt::EditRole*/ )
{
  return false;
}

void QmitkDataStorageTreeModel::AdjustLayerProperty()
{
  /// transform the tree into an array and set the layer property descending
  std::vector<TreeItem*> vec;
  this->TreeToVector(m_Root, vec);

  int i = vec.size()-1;
  for(std::vector<TreeItem*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
  {
    (*it)->GetDataTreeNode()->SetIntProperty("layer", i);
    --i;
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataStorageTreeModel::TreeToVector(TreeItem* parent, std::vector<TreeItem*>& vec) const
{
  TreeItem* current;
  for(int i = 0; i<parent->GetChildCount(); ++i)
  {
    current = parent->GetChild(i);
    this->TreeToVector(current, vec);
    vec.push_back(current);
  }
}

QModelIndex QmitkDataStorageTreeModel::IndexFromTreeItem( TreeItem* item ) const
{
  if(item == m_Root)
    return QModelIndex();
  else
    return this->createIndex(item->GetIndex(), 0, item);
}

std::vector<mitk::DataTreeNode*> QmitkDataStorageTreeModel::GetNodeSet() const
{
  std::vector<mitk::DataTreeNode*> vec;
  if(m_Root)
    this->TreeToNodeSet(m_Root, vec);

  return vec;
}

void QmitkDataStorageTreeModel::TreeToNodeSet( TreeItem* parent, std::vector<mitk::DataTreeNode*>& vec ) const
{
  TreeItem* current;
  for(int i = 0; i<parent->GetChildCount(); ++i)
  {
    current = parent->GetChild(i);
    vec.push_back(current->GetDataTreeNode());
    this->TreeToNodeSet(current, vec);
  }
}

QModelIndex QmitkDataStorageTreeModel::GetIndex( const mitk::DataTreeNode* node ) const
{
  if(m_Root)
  {
    TreeItem* item = m_Root->Find(node);
    if(item)
      return this->IndexFromTreeItem(item);
  }
  return QModelIndex();
}

QmitkDataStorageTreeModel::TreeItem::TreeItem( mitk::DataTreeNode* _DataTreeNode, TreeItem* _Parent )
: m_Parent(_Parent)
, m_DataTreeNode(_DataTreeNode)
{
  if(m_Parent)
    m_Parent->AddChild(this);
}

QmitkDataStorageTreeModel::TreeItem::~TreeItem()
{
  if(m_Parent)
    m_Parent->RemoveChild(this);
}

void QmitkDataStorageTreeModel::TreeItem::Delete()
{
  while(m_Children.size() > 0)
    delete m_Children.back();

  delete this;
}

QmitkDataStorageTreeModel::TreeItem* QmitkDataStorageTreeModel::TreeItem::Find( const mitk::DataTreeNode* _DataTreeNode ) const
{
  QmitkDataStorageTreeModel::TreeItem* item = 0;
  if(_DataTreeNode)
  {
    if(m_DataTreeNode == _DataTreeNode)
      item = const_cast<TreeItem*>(this);
    else
    {
      for(std::vector<TreeItem*>::const_iterator it = m_Children.begin(); it != m_Children.end(); ++it)
      {
        if(item)
          break;
        item = (*it)->Find(_DataTreeNode);
      }
    }
  }
  return item;
}

int QmitkDataStorageTreeModel::TreeItem::IndexOfChild( const TreeItem* item ) const
{
  std::vector<TreeItem*>::const_iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  return it != m_Children.end() ? std::distance(m_Children.begin(), it): -1;
}

QmitkDataStorageTreeModel::TreeItem* QmitkDataStorageTreeModel::TreeItem::GetChild( int index ) const
{
  return (m_Children.size() > 0 && index >= 0 && index < (int)m_Children.size())? m_Children.at(index): 0;
}

void QmitkDataStorageTreeModel::TreeItem::AddChild( TreeItem* item )
{ 
  this->InsertChild(item);
}

void QmitkDataStorageTreeModel::TreeItem::RemoveChild( TreeItem* item )
{
  std::vector<TreeItem*>::iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  if(it != m_Children.end())
  {
    m_Children.erase(it);
    item->SetParent(0);
  }
}

int QmitkDataStorageTreeModel::TreeItem::GetChildCount() const
{
  return m_Children.size();
}

int QmitkDataStorageTreeModel::TreeItem::GetIndex() const
{
  if (m_Parent)
    return m_Parent->IndexOfChild(this);

  return 0;
}

QmitkDataStorageTreeModel::TreeItem* QmitkDataStorageTreeModel::TreeItem::GetParent() const
{
  return m_Parent;
}

mitk::DataTreeNode::Pointer QmitkDataStorageTreeModel::TreeItem::GetDataTreeNode() const
{
  return m_DataTreeNode;
}

void QmitkDataStorageTreeModel::TreeItem::InsertChild( TreeItem* item, int index )
{
  std::vector<TreeItem*>::iterator it = std::find(m_Children.begin(), m_Children.end(), item);
  if(it == m_Children.end())
  {
    if(m_Children.size() > 0 && index >= 0 && index < (int)m_Children.size())
    {
      it = m_Children.begin();
      std::advance(it, index);
      m_Children.insert(it, item);
    }
    else
      m_Children.push_back(item);

    // add parent if necessary
    if(item->GetParent() != this)
      item->SetParent(this);
  }
}

std::vector<QmitkDataStorageTreeModel::TreeItem*> QmitkDataStorageTreeModel::TreeItem::GetChildren() const
{
  return m_Children;
}

void QmitkDataStorageTreeModel::TreeItem::SetParent( TreeItem* _Parent )
{
  m_Parent = _Parent;
  if(m_Parent)
    m_Parent->AddChild(this);
}
