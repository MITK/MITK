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
  // if there was an old storage, remove listeners
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();

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
  int index = -1;

  auto nodeIt = std::find(m_Nodes.begin(), m_Nodes.end(), dataNode);

  if (nodeIt != m_Nodes.end())
    index = std::distance(m_Nodes.begin(), nodeIt);

  return index;
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

    // if there is a new storage, add listeners
    if (!m_DataStorage.IsExpired())
    {
      currentDataStorage = m_DataStorage.Lock();

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
    // remove itk::Event observer
    mitk::DataNode *dataNode = m_Nodes.at(index);
    // get name property first
    mitk::BaseProperty *nameProperty = dataNode->GetProperty("name");
    // if prop exists remove modified listener
    if (nameProperty)
    {
      nameProperty->RemoveObserver(m_NodesModifiedObserverTags[index]);
      // remove name property map
      m_PropertyToNode.erase(dataNode);
    }
    // then remove delete listener on the node itself
    dataNode->RemoveObserver(m_NodesDeleteObserverTags[index]);
    // remove observer tags from lists
    m_NodesModifiedObserverTags.erase(m_NodesModifiedObserverTags.begin() + index);
    m_NodesDeleteObserverTags.erase(m_NodesDeleteObserverTags.begin() + index);
    // remove node from node vector
    m_Nodes.erase(m_Nodes.begin() + index);
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
    this->InsertNode(index, dataNode);
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

void QmitkDataStorageComboBox::OnDataNodeDeleteOrModified(const itk::Object *caller, const itk::EventObject &event)
{
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    // check if we have a modified event (if not it is a delete event)
    const itk::ModifiedEvent *modifiedEvent = dynamic_cast<const itk::ModifiedEvent *>(&event);

    // when node was modified reset text
    if (modifiedEvent)
    {
      const mitk::BaseProperty *nameProperty = dynamic_cast<const mitk::BaseProperty *>(caller);

      // node name changed, set it
      // but first of all find associated node
      for (auto it = m_PropertyToNode.begin(); it != m_PropertyToNode.end(); ++it)
      {
        // property is found take node
        if (it->second == nameProperty)
        {
          // looks strange but when calling setnode with the same node, that means the node gets updated
          this->SetNode(it->first, it->first);
          break;
        }
      }
    }
    else
    {
      const mitk::DataNode *constDataNode = dynamic_cast<const mitk::DataNode *>(caller);
      if (constDataNode)
        // node will be deleted, remove it
        this->RemoveNode(constDataNode);
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

void QmitkDataStorageComboBox::SetSelectedNode(mitk::DataNode::Pointer item)
{
  int index = this->Find(item);
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
  std::string itemName = "unnamed node";
  if (!changedNode)
  {
    // break on duplicated nodes (that doesn't make sense to have duplicates in the combobox)
    if (this->Find(dataNode) != -1)
      return;

    // add modified observer
    itk::MemberCommand<QmitkDataStorageComboBox>::Pointer modifiedCommand =
      itk::MemberCommand<QmitkDataStorageComboBox>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkDataStorageComboBox::OnDataNodeDeleteOrModified);

    // add modified observer for the name property
    // first try retrieving the name property of the data node
    mitk::BaseProperty *nameProperty = nonConstDataNode->GetProperty("name");
    if (nameProperty)
    {
      m_NodesModifiedObserverTags.push_back(nameProperty->AddObserver(itk::ModifiedEvent(), modifiedCommand));
      m_PropertyToNode[nonConstDataNode] = nameProperty;
      itemName = nameProperty->GetValueAsString();
    }
    // if there is no name node save an invalid value for the observer tag (-1)
    else
      m_NodesModifiedObserverTags.push_back(-1);

    // add delete observer
    itk::MemberCommand<QmitkDataStorageComboBox>::Pointer deleteCommand =
      itk::MemberCommand<QmitkDataStorageComboBox>::New();
    deleteCommand->SetCallbackFunction(this, &QmitkDataStorageComboBox::OnDataNodeDeleteOrModified);
    m_NodesDeleteObserverTags.push_back(nonConstDataNode->AddObserver(itk::DeleteEvent(), modifiedCommand));
  }

  // add node to the vector
  if (addNewNode)
    m_Nodes.push_back(nonConstDataNode);
  else if (insertNewNode)
    m_Nodes.insert(m_Nodes.begin() + index, nonConstDataNode);

  if (addNewNode)
  {
    this->addItem(QString::fromStdString(itemName));
    // select new node if m_AutoSelectNewNodes is true or if we have just added the first node
    if (m_AutoSelectNewNodes || m_Nodes.size() == 1)
      this->setCurrentIndex(index);
  }
  else
  {
    // update text in combobox
    this->setItemText(index, QString::fromStdString(itemName));
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

  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
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
