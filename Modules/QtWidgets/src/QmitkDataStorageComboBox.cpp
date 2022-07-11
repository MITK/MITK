/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataStorageComboBox.h"

#include <itkCommand.h>

QmitkDataStorageComboBox::QmitkDataStorageComboBox(QWidget *parent, bool autoSelectNewNodes)
  : QComboBox(parent),
    m_DataStorage(nullptr),
    m_Predicate(nullptr),
    m_BlockEvents(false),
    m_AutoSelectNewNodes(autoSelectNewNodes)
{
  this->Init();
}

QmitkDataStorageComboBox::QmitkDataStorageComboBox(mitk::DataStorage *dataStorage,
                                                   const mitk::NodePredicateBase *predicate,
                                                   QWidget *parent,
                                                   bool autoSelectNewNodes)
  : QComboBox(parent),
    m_DataStorage(nullptr),
    m_Predicate(predicate),
    m_BlockEvents(false),
    m_AutoSelectNewNodes(autoSelectNewNodes)
{
  // make connections, fill combobox
  this->Init();
  this->SetDataStorage(dataStorage);
}

QmitkDataStorageComboBox::~QmitkDataStorageComboBox()
{
  auto dataStorage = m_DataStorage.Lock();

  // if there was an old storage, remove listeners
  if (dataStorage.IsNotNull())
  {
    dataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
                                                                               &QmitkDataStorageComboBox::AddNode));

    dataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
                                                                               &QmitkDataStorageComboBox::RemoveNode));
  }
  // we have lots of observers to nodes and their name properties, this gets ugly if nodes live longer than the box
  while (m_Nodes.size() > 0)
    RemoveNode(0);
}

int QmitkDataStorageComboBox::Find(const mitk::DataNode *dataNode) const
{
  std::iterator_traits<mitk::DataNode*>::difference_type index = -1;

  auto nodeIt = std::find(m_Nodes.begin(), m_Nodes.end(), dataNode);

  if (nodeIt != m_Nodes.end())
    index = std::distance(m_Nodes.begin(), nodeIt);

  return static_cast<int>(index);
}

mitk::DataStorage::Pointer QmitkDataStorageComboBox::GetDataStorage() const
{
  return m_DataStorage.Lock();
}

const mitk::NodePredicateBase::ConstPointer QmitkDataStorageComboBox::GetPredicate() const
{
  return m_Predicate.GetPointer();
}

mitk::DataNode::Pointer QmitkDataStorageComboBox::GetNode(int index) const
{
  return (this->HasIndex(index)) ? m_Nodes.at(index) : nullptr;
}

mitk::DataNode::Pointer QmitkDataStorageComboBox::GetSelectedNode() const
{
  if (this->count() == 0)
    return nullptr;

  int currentIndex = this->currentIndex();

  return currentIndex >= 0 ? this->GetNode(currentIndex) : nullptr;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataStorageComboBox::GetNodes() const
{
  mitk::DataStorage::SetOfObjects::Pointer setOfObjects = mitk::DataStorage::SetOfObjects::New();

  for (auto it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
  {
    setOfObjects->push_back(*it);
  }

  return setOfObjects.GetPointer();
}

bool QmitkDataStorageComboBox::GetAutoSelectNewItems()
{
  return m_AutoSelectNewNodes;
}

void QmitkDataStorageComboBox::SetDataStorage(mitk::DataStorage *dataStorage)
{
  auto currentDataStorage = m_DataStorage.Lock();

  // reset only if datastorage really changed
  if (currentDataStorage.GetPointer() != dataStorage)
  {
    // if there was an old storage, remove listeners
    if (currentDataStorage.IsNotNull())
    {
      currentDataStorage->AddNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
          &QmitkDataStorageComboBox::AddNode));

      currentDataStorage->RemoveNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
          &QmitkDataStorageComboBox::RemoveNode));
    }
    // set new storage
    m_DataStorage = dataStorage;
    currentDataStorage = m_DataStorage.Lock();

    // if there is a new storage, add listeners
    if (currentDataStorage.IsNotNull())
    {
      currentDataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
          &QmitkDataStorageComboBox::AddNode));

      currentDataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageComboBox, const mitk::DataNode *>(this,
          &QmitkDataStorageComboBox::RemoveNode));
    }

    // reset predicate to reset the combobox
    this->Reset();
  }
}

void QmitkDataStorageComboBox::SetPredicate(const mitk::NodePredicateBase *predicate)
{
  if (m_Predicate != predicate)
  {
    m_Predicate = predicate;
    this->Reset();
  }
}

void QmitkDataStorageComboBox::AddNode(const mitk::DataNode *dataNode)
{
  // this is an event function, make sure that we didn't call ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;
    // pass a -1 to the InsertNode function in order to append the datatree node to the end
    this->InsertNode(-1, dataNode);
    m_BlockEvents = false;
  }
}

void QmitkDataStorageComboBox::RemoveNode(int index)
{
  if (this->HasIndex(index))
  {
    RemoveNodeAndPropertyLists(index);
    // remove node name from combobox
    this->removeItem(index);
  }
}

void QmitkDataStorageComboBox::RemoveNode(const mitk::DataNode *dataNode)
{
  // this is an event function, make sure that we didn't call ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;
    this->RemoveNode(this->Find(dataNode));
    m_BlockEvents = false;
  }
}

void QmitkDataStorageComboBox::SetNode(int index, const mitk::DataNode *dataNode)
{
  if (this->HasIndex(index))
  {
    // if node is identical, we only update the name in the QComboBoxItem
    if (dataNode == m_Nodes.at(index))
    {
      this->setItemText(index, QString::fromStdString(dataNode->GetName()));
    }
    else
    {
      this->InsertNode(index, dataNode);
    }
  }
}

void QmitkDataStorageComboBox::SetNode(const mitk::DataNode *dataNode, const mitk::DataNode *otherDataNode)
{
  this->SetNode(this->Find(dataNode), otherDataNode);
}

void QmitkDataStorageComboBox::SetAutoSelectNewItems(bool autoSelectNewItems)
{
  m_AutoSelectNewNodes = autoSelectNewItems;
}

void QmitkDataStorageComboBox::OnPropertyListChanged(const itk::Object *caller, const itk::EventObject &event)
{
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    // check if we have a modified event
    const itk::ModifiedEvent *modifiedEvent = dynamic_cast<const itk::ModifiedEvent *>(&event);
    if (modifiedEvent)
    {
      const mitk::PropertyList *propertyList = dynamic_cast<const mitk::PropertyList *>(caller);
      UpdateComboBoxText(propertyList);
    }

    m_BlockEvents = false;
  }
}

bool QmitkDataStorageComboBox::HasIndex(unsigned int index) const
{
  return (m_Nodes.size() > 0 && index < m_Nodes.size());
}

void QmitkDataStorageComboBox::OnCurrentIndexChanged(int index)
{
  if (index >= 0 && index < this->count())
    emit OnSelectionChanged(this->GetSelectedNode());
  if (index == -1)
    emit OnSelectionChanged(nullptr);
}

void QmitkDataStorageComboBox::SetSelectedNode(const mitk::DataNode::Pointer& node)
{
  int index = this->Find(node);
  if (index == -1)
  {
    MITK_INFO << "QmitkDataStorageComboBox: item not available";
  }
  else
  {
    this->setCurrentIndex(index);
  }
}

void QmitkDataStorageComboBox::InsertNode(int index, const mitk::DataNode *dataNode)
{
  // check new or updated node first
  if (m_Predicate.IsNotNull() && !m_Predicate->CheckNode(dataNode))
    return;

  bool addNewNode = false;
  bool insertNewNode = false;
  bool changedNode = false;

  // if this->HasIndex(index), then a node shall be updated
  if (this->HasIndex(index))
  {
    // if we really have another node at this position then ...
    if (dataNode != m_Nodes.at(index))
    {
      // ... remove node, then proceed as usual
      this->RemoveNode(index);
      insertNewNode = true;
    }
    else
      changedNode = true;
  }
  // otherwise a new node shall be added, let index point to the element after the last element
  else
  {
    index = m_Nodes.size();
    addNewNode = true;
  }

  // const cast because we need non const nodes
  mitk::DataNode *nonConstDataNode = const_cast<mitk::DataNode *>(dataNode);
  if (!changedNode)
  {
    // break on duplicated nodes (that doesn't make sense to have duplicates in the combobox)
    if (this->Find(dataNode) != -1)
      return;

    // add modified observer
    itk::MemberCommand<QmitkDataStorageComboBox>::Pointer propertyListChangedCommand =
      itk::MemberCommand<QmitkDataStorageComboBox>::New();
    propertyListChangedCommand->SetCallbackFunction(this, &QmitkDataStorageComboBox::OnPropertyListChanged);

    // add observer for the data node property list
    mitk::PropertyList* dataNodePropertyList = nonConstDataNode->GetPropertyList();
    if (nullptr != dataNodePropertyList)
    {
      m_DataNodePropertyListObserverTags.push_back(dataNodePropertyList->AddObserver(itk::ModifiedEvent(),
        propertyListChangedCommand));
    }
    else
    {
      // fill vector with invalid value
      m_DataNodePropertyListObserverTags.push_back(-1);
    }

    mitk::PropertyList* baseDataPropertyList;
    //add observer for the base data property list
    mitk::BaseData* baseData = dynamic_cast<mitk::BaseData*>(nonConstDataNode->GetData());
    if (nullptr != baseData)
    {
      baseDataPropertyList = baseData->GetPropertyList();
      if (nullptr != baseDataPropertyList)
      {
        m_BaseDatapropertyListObserverTags.push_back(baseDataPropertyList->AddObserver(itk::ModifiedEvent(),
          propertyListChangedCommand));
      }
      else
      {
        // fill vector with invalid value
        m_BaseDatapropertyListObserverTags.push_back(-1);
      }
    }
    else
    {
      // fill vector with invalid value
      m_BaseDatapropertyListObserverTags.push_back(-1);
    }
  }

  // add node to the vector
  if (addNewNode)
    m_Nodes.push_back(nonConstDataNode);
  else if (insertNewNode)
    m_Nodes.insert(m_Nodes.begin() + index, nonConstDataNode);

  if (addNewNode)
  {
    this->addItem(QString::fromStdString(nonConstDataNode->GetName()));
    // select new node if m_AutoSelectNewNodes is true or if we have just added the first node
    if (m_AutoSelectNewNodes || m_Nodes.size() == 1)
      this->setCurrentIndex(index);
  }
  else
  {
    // update text in combobox
    this->setItemText(index, QString::fromStdString(nonConstDataNode->GetName()));
  }
}

void QmitkDataStorageComboBox::Init()
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
}

void QmitkDataStorageComboBox::Reset()
{
  // remove all nodes first
  while (!m_Nodes.empty())
  {
    // remove last node
    // explicitly calling RemoveNode of QmitkDataStorageComboBox since derived classes may prevent the removal of all
    // nodes in their respective RemoveNode implementation. This is happening for example in
    // QmitkDataStorageComboBoxWithSelectNone.
    QmitkDataStorageComboBox::RemoveNode(m_Nodes.size() - 1);
  }

  // clear combobox
  this->clear();

  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects;

    // select all if predicate == nullptr
    if (m_Predicate.IsNotNull())
      setOfObjects = dataStorage->GetSubset(m_Predicate);
    else
      setOfObjects = dataStorage->GetAll();

    // add all found nodes
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin(); nodeIt != setOfObjects->End();
         ++nodeIt) // for each dataNode
    {
      // add node to the node vector and to the combobox
      this->AddNode(nodeIt.Value().GetPointer());
    }
  }
}

void QmitkDataStorageComboBox::RemoveNodeAndPropertyLists(int index)
{
  // remove itk::Event observer
  mitk::DataNode *dataNode = m_Nodes.at(index);

  // remove observer from data node property list
  mitk::PropertyList* dataNodePropertyList = dataNode->GetPropertyList();
  if (nullptr != dataNodePropertyList)
  {
    dataNodePropertyList->RemoveObserver(m_DataNodePropertyListObserverTags[index]);
    // remove observer tags from lists
    m_DataNodePropertyListObserverTags.erase(m_DataNodePropertyListObserverTags.begin() + index);
  }

  // remove observer from base data property list
  mitk::BaseData* baseData = dynamic_cast<mitk::BaseData*>(dataNode->GetData());
  if (nullptr != baseData)
  {
    mitk::PropertyList* baseDataPropertyList = baseData->GetPropertyList();
    if (nullptr != dataNodePropertyList)
    {
      baseDataPropertyList->RemoveObserver(m_BaseDatapropertyListObserverTags[index]);
      // remove observer tags from lists
      m_BaseDatapropertyListObserverTags.erase(m_BaseDatapropertyListObserverTags.begin() + index);
    }
  }

  // remove node from node vector
  m_Nodes.erase(m_Nodes.begin() + index);
}

void QmitkDataStorageComboBox::UpdateComboBoxText(const mitk::PropertyList* propertyList)
{
  mitk::PropertyList* dataNodePropertyList = nullptr;
  mitk::PropertyList* baseDataPropertyList = nullptr;
  mitk::BaseData* baseData;
  for (const auto& node : m_Nodes)
  {
    dataNodePropertyList = node->GetPropertyList();

    baseData = dynamic_cast<mitk::BaseData*>(node->GetData());
    if (nullptr != baseData)
    {
      baseDataPropertyList = baseData->GetPropertyList();
    }

    if (propertyList == dataNodePropertyList
     || propertyList == baseDataPropertyList)
    {
      // if one of the property list is the one that has just been modified
      // get the node's index and set its text to the node name
      // the node name might have been changed, depending on the modified property list
      auto index = Find(node);
      // update text in combobox
      this->setItemText(index, QString::fromStdString(node->GetName()));
      return;
    }
  }
}
