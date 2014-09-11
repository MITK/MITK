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
#include <mitkStringProperty.h>
#include <mitkNodePredicateFirstLevel.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include "QmitkDataStorageTreeModel.h"
#include "QmitkNodeDescriptorManager.h"
#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

#include <QIcon>
#include <QMimeData>
#include <QTextStream>

#include <map>

QmitkDataStorageTreeModel::QmitkDataStorageTreeModel( mitk::DataStorage* _DataStorage
                                                      , bool _PlaceNewNodesOnTop
                                                      , QObject* parent )
: QAbstractItemModel(parent)
, m_DataStorage(0)
, m_PlaceNewNodesOnTop(_PlaceNewNodesOnTop)
, m_Root(0)
{
  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageTreeModel::~QmitkDataStorageTreeModel()
{
  // set data storage to 0 = remove all listeners
  this->SetDataStorage(0);
  m_Root->Delete(); m_Root = 0;
}

mitk::DataNode::Pointer QmitkDataStorageTreeModel::GetNode( const QModelIndex &index ) const
{
  return this->TreeItemFromIndex(index)->GetDataNode();
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
  mitk::DataNode* dataNode = this->TreeItemFromIndex(index)->GetDataNode();
  if (index.isValid())
  {
    if(DicomPropertiesExists(*dataNode))
    {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }else{
    return Qt::ItemIsDropEnabled;
  }
}

int QmitkDataStorageTreeModel::columnCount( const QModelIndex& /* parent = QModelIndex() */ ) const
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

Qt::DropActions QmitkDataStorageTreeModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

bool QmitkDataStorageTreeModel::dropMimeData(const QMimeData *data,
                                     Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
  // Early exit, returning true, but not actually doing anything (ignoring data).
  if (action == Qt::IgnoreAction)
  {
    return true;
  }

  // Note, we are returning true if we handled it, and false otherwise
  bool returnValue = false;

  if(data->hasFormat("application/x-qabstractitemmodeldatalist"))
  {
    returnValue = true;

    // First we extract a Qlist of TreeItem* pointers.
    QString arg = QString(data->data("application/x-qabstractitemmodeldatalist").data());
    QStringList listOfTreeItemAddressPointers = arg.split(",");

    QStringList::iterator slIter;
    QList<TreeItem*> listOfItemsToDrop;

    for(slIter  = listOfTreeItemAddressPointers.begin();
        slIter != listOfTreeItemAddressPointers.end();
        slIter++)
    {
      long val = (*slIter).toLong();
      listOfItemsToDrop << static_cast<TreeItem *>((void*)val);
    }

    // Retrieve the TreeItem* where we are dropping stuff, and its parent.
    TreeItem* dropItem = this->TreeItemFromIndex(parent);
    TreeItem* parentItem = dropItem->GetParent();

    // If item was dropped onto empty space, we select the root node
    if(dropItem == m_Root)
    {
      parentItem = m_Root;
    }

    // Dragging and Dropping is only allowed within the same parent, so use the first item in list to validate.
    // (otherwise, you could have a derived image such as a segmentation, and assign it to another image).
    // NOTE: We are assuming the input list is valid... i.e. when it was dragged, all the items had the same parent.

    if(listOfItemsToDrop[0] != dropItem && listOfItemsToDrop[0]->GetParent() == parentItem)
    {
      // Retrieve the index of where we are dropping stuff.
      QModelIndex dropItemModelIndex = this->IndexFromTreeItem(dropItem);
      QModelIndex parentModelIndex = this->IndexFromTreeItem(parentItem);

      // Iterate through the list of TreeItem (which may be at non-consecutive indexes).
      QList<TreeItem*>::iterator diIter;
      for (diIter  = listOfItemsToDrop.begin();
           diIter != listOfItemsToDrop.end();
           diIter++)
      {
        // Here we assume that as you remove items, one at a time, that GetIndex() will be valid.
        this->beginRemoveRows(parentModelIndex, (*diIter)->GetIndex(), (*diIter)->GetIndex());
        parentItem->RemoveChild(*diIter);
        this->endRemoveRows();
      }

      // Select the target index position, or put it at the end of the list.
      int dropIndex = dropItemModelIndex.row();
      if (dropIndex == -1)
      {
        dropIndex = parentItem->GetChildCount();
      }

      // Now insert items again at the drop item position
      this->beginInsertRows(parentModelIndex, dropIndex, dropIndex + listOfItemsToDrop.size() - 1);

      for (diIter  = listOfItemsToDrop.begin();
           diIter != listOfItemsToDrop.end();
           diIter++)
      {
        parentItem->InsertChild( (*diIter), dropIndex );
        dropIndex++;
      }
      this->endInsertRows();

      // Change Layers to match.
      this->AdjustLayerProperty();
    }
  }
  else if(data->hasFormat("application/x-mitk-datanodes"))
  {
    returnValue = true;

    QString arg = QString(data->data("application/x-mitk-datanodes").data());
    QStringList listOfDataNodeAddressPointers = arg.split(",");
    int numberOfNodesDropped = 0;

    QStringList::iterator slIter;
    for (slIter = listOfDataNodeAddressPointers.begin();
         slIter != listOfDataNodeAddressPointers.end();
         slIter++)
    {
      long val = (*slIter).toLong();
      mitk::DataNode* node = static_cast<mitk::DataNode *>((void*)val);

      if(node && m_DataStorage.IsNotNull() && !m_DataStorage->Exists(node))
      {
          m_DataStorage->Add( node );
          mitk::BaseData::Pointer basedata = node->GetData();

          if (basedata.IsNotNull())
          {
            mitk::RenderingManager::GetInstance()->InitializeViews(
              basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );

            numberOfNodesDropped++;
          }
      }
    }
    // Only do a rendering update, if we actually dropped anything.
    if (numberOfNodesDropped > 0)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  return returnValue;
}

QStringList QmitkDataStorageTreeModel::mimeTypes() const
{
    QStringList types = QAbstractItemModel::mimeTypes();
    types << "application/x-qabstractitemmodeldatalist";
    types << "application/x-mitk-datanodes";
    return types;
}

QMimeData * QmitkDataStorageTreeModel::mimeData(const QModelIndexList & indexes) const{

  QMimeData * ret = new QMimeData;

  QString treeItemAddresses("");
  QString dataNodeAddresses("");

  for (int i = 0; i < indexes.size(); i++)
  {
    TreeItem* treeItem = static_cast<TreeItem*>(indexes.at(i).internalPointer());
    long treeItemAddress = reinterpret_cast<long>(treeItem);
    long dataNodeAddress = reinterpret_cast<long>(treeItem->GetDataNode().GetPointer());
    QTextStream(&treeItemAddresses) << treeItemAddress;
    QTextStream(&dataNodeAddresses) << dataNodeAddress;

    if (i != indexes.size() - 1)
    {
      QTextStream(&treeItemAddresses) << ",";
      QTextStream(&dataNodeAddresses) << ",";
    }
  }

  ret->setData("application/x-qabstractitemmodeldatalist", QByteArray(treeItemAddresses.toAscii()));
  ret->setData("application/x-mitk-datanodes", QByteArray(dataNodeAddresses.toAscii()));

  return ret;
}

QVariant QmitkDataStorageTreeModel::data( const QModelIndex & index, int role ) const
{
  mitk::DataNode* dataNode = this->TreeItemFromIndex(index)->GetDataNode();

  // get name of treeItem (may also be edited)
  QString nodeName;
  if(DicomPropertiesExists(*dataNode))
  {
    mitk::BaseProperty* seriesDescription = (dataNode->GetProperty("dicom.series.SeriesDescription"));
    mitk::BaseProperty* studyDescription = (dataNode->GetProperty("dicom.study.StudyDescription"));
    mitk::BaseProperty* patientsName = (dataNode->GetProperty("dicom.patient.PatientsName"));

    nodeName.append(patientsName->GetValueAsString().c_str()).append("\n");
    nodeName.append(studyDescription->GetValueAsString().c_str()).append("\n");
    nodeName.append(seriesDescription->GetValueAsString().c_str());
  }else{
      nodeName = QString::fromStdString(dataNode->GetName());
  }
  if(nodeName.isEmpty())
  {
    nodeName = "unnamed";
  }

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
  else if(role == QmitkDataNodeRole)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }
  else if(role == QmitkDataNodeRawPointerRole)
  {
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
  }

  return QVariant();
}

bool QmitkDataStorageTreeModel::DicomPropertiesExists(const mitk::DataNode& node) const
{
    bool propertiesExists = false;
    mitk::BaseProperty* seriesDescription = (node.GetProperty("dicom.series.SeriesDescription"));
    mitk::BaseProperty* studyDescription = (node.GetProperty("dicom.study.StudyDescription"));
    mitk::BaseProperty* patientsName = (node.GetProperty("dicom.patient.PatientsName"));

    if(patientsName!=NULL && studyDescription!=NULL && seriesDescription!=NULL)
    {
        if((!patientsName->GetValueAsString().empty())&&
            (!studyDescription->GetValueAsString().empty())&&
            (!seriesDescription->GetValueAsString().empty()))
        {
            propertiesExists = true;
        }
    }
    return propertiesExists;
}


QVariant QmitkDataStorageTreeModel::headerData(int /*section*/,
                                 Qt::Orientation orientation,
                                 int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_Root)
     return QString::fromStdString(m_Root->GetDataNode()->GetName());

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
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::AddNode ) );

      m_DataStorage->ChangedNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::SetNodeModified ) );

      m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::RemoveNode ) );

    }

    // take over the new data storage
    m_DataStorage = _DataStorage;

    // delete the old root (if necessary, create new)
    if(m_Root)
      m_Root->Delete();
    mitk::DataNode::Pointer rootDataNode = mitk::DataNode::New();
    rootDataNode->SetName("Data Manager");
    m_Root = new TreeItem(rootDataNode, 0);
    this->reset();

    if(m_DataStorage.IsNotNull())
    {
      // add Listener for the data storage itself
      m_DataStorage.ObjectDelete.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const itk::Object*>( this, &QmitkDataStorageTreeModel::SetDataStorageDeleted ) );

      // add listeners for the nodes
      m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::AddNode ) );

      m_DataStorage->ChangedNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::SetNodeModified ) );

      m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageTreeModel
        , const mitk::DataNode*>( this, &QmitkDataStorageTreeModel::RemoveNode ) );

      mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = m_DataStorage->GetSubset(m_Predicate);

      // finally add all nodes to the model
      this->Update();
    }
  }
}

void QmitkDataStorageTreeModel::SetDataStorageDeleted( const itk::Object* /*_DataStorage*/ )
{
  this->SetDataStorage(0);
}

void QmitkDataStorageTreeModel::AddNodeInternal(const mitk::DataNode *node)
{
    if(node == 0
      || m_DataStorage.IsNull()
      || !m_DataStorage->Exists(node)
      || m_Root->Find(node) != 0)
      return;

    // find out if we have a root node
    TreeItem* parentTreeItem = m_Root;
    QModelIndex index;
    mitk::DataNode* parentDataNode = this->GetParentNode(node);

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

    // add node
    if(m_PlaceNewNodesOnTop)
    {
      // emit beginInsertRows event
      beginInsertRows(index, 0, 0);
      parentTreeItem->InsertChild(new TreeItem(
          const_cast<mitk::DataNode*>(node)), 0);
    }
    else
    {
      beginInsertRows(index, parentTreeItem->GetChildCount()
                      , parentTreeItem->GetChildCount());
      new TreeItem(const_cast<mitk::DataNode*>(node), parentTreeItem);
    }

    // emit endInsertRows event
    endInsertRows();

    this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::AddNode( const mitk::DataNode* node )
{
    if(node == 0
      || m_DataStorage.IsNull()
      || !m_DataStorage->Exists(node)
      || m_Root->Find(node) != 0)
      return;

      this->AddNodeInternal(node);
}


void QmitkDataStorageTreeModel::SetPlaceNewNodesOnTop(bool _PlaceNewNodesOnTop)
{
  m_PlaceNewNodesOnTop = _PlaceNewNodesOnTop;
}

void QmitkDataStorageTreeModel::RemoveNodeInternal( const mitk::DataNode* node )
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

void QmitkDataStorageTreeModel::RemoveNode( const mitk::DataNode* node )
{
    if (node == 0)
        return;

    this->RemoveNodeInternal(node);
}

void QmitkDataStorageTreeModel::SetNodeModified( const mitk::DataNode* node )
{
  TreeItem* treeItem = m_Root->Find(node);
  if(treeItem) {
    TreeItem* parentTreeItem = treeItem->GetParent();
    // as the root node should not be removed one should always have a parent item
    if(!parentTreeItem)
      return;
    QModelIndex index = this->createIndex(treeItem->GetIndex(), 0, treeItem);

    // now emit the dataChanged signal
    emit dataChanged(index, index);
  }
}

mitk::DataNode* QmitkDataStorageTreeModel::GetParentNode( const mitk::DataNode* node ) const
{
  mitk::DataNode* dataNode = 0;

  mitk::DataStorage::SetOfObjects::ConstPointer _Sources = m_DataStorage->GetSources(node);

  if(_Sources->Size() > 0)
    dataNode = _Sources->front();

  return dataNode;
}

bool QmitkDataStorageTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  mitk::DataNode* dataNode = this->TreeItemFromIndex(index)->GetDataNode();
  if(!dataNode)
    return false;

  if(role == Qt::EditRole && !value.toString().isEmpty())
  {
    dataNode->SetStringProperty("name", value.toString().toStdString().c_str());

    mitk::PlanarFigure* planarFigure = dynamic_cast<mitk::PlanarFigure*>(dataNode->GetData());

    if (planarFigure != NULL)
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

bool QmitkDataStorageTreeModel::setHeaderData( int /*section*/, Qt::Orientation /*orientation*/, const QVariant& /* value */, int /*role = Qt::EditRole*/ )
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
    mitk::DataNode::Pointer dataNode = (*it)->GetDataNode();
    bool fixedLayer = false;

    if (!(dataNode->GetBoolProperty("fixedLayer", fixedLayer) && fixedLayer))
      dataNode->SetIntProperty("layer", i);

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

QList<mitk::DataNode::Pointer> QmitkDataStorageTreeModel::GetNodeSet() const
{
  QList<mitk::DataNode::Pointer> res;
  if(m_Root)
    this->TreeToNodeSet(m_Root, res);

  return res;
}

void QmitkDataStorageTreeModel::TreeToNodeSet( TreeItem* parent, QList<mitk::DataNode::Pointer>& vec ) const
{
  TreeItem* current;
  for(int i = 0; i<parent->GetChildCount(); ++i)
  {
    current = parent->GetChild(i);
    vec.push_back(current->GetDataNode());
    this->TreeToNodeSet(current, vec);
  }
}

QModelIndex QmitkDataStorageTreeModel::GetIndex( const mitk::DataNode* node ) const
{
  if(m_Root)
  {
    TreeItem* item = m_Root->Find(node);
    if(item)
      return this->IndexFromTreeItem(item);
  }
  return QModelIndex();
}

QmitkDataStorageTreeModel::TreeItem::TreeItem( mitk::DataNode* _DataNode, TreeItem* _Parent )
: m_Parent(_Parent)
, m_DataNode(_DataNode)
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

QmitkDataStorageTreeModel::TreeItem* QmitkDataStorageTreeModel::TreeItem::Find( const mitk::DataNode* _DataNode ) const
{
  QmitkDataStorageTreeModel::TreeItem* item = 0;
  if(_DataNode)
  {
    if(m_DataNode == _DataNode)
      item = const_cast<TreeItem*>(this);
    else
    {
      for(std::vector<TreeItem*>::const_iterator it = m_Children.begin(); it != m_Children.end(); ++it)
      {
        if(item)
          break;
        item = (*it)->Find(_DataNode);
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

mitk::DataNode::Pointer QmitkDataStorageTreeModel::TreeItem::GetDataNode() const
{
  return m_DataNode;
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

void QmitkDataStorageTreeModel::Update()
{
    if (m_DataStorage.IsNotNull())
    {
        this->reset();

        mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = m_DataStorage->GetAll();

        for (mitk::DataStorage::SetOfObjects::const_iterator it = _NodeSet->begin(); it != _NodeSet->end(); it++)
        {
            // save node
            this->AddNodeInternal(*it);
        }
    }
}
