#include "QmitkDataStorageComboBox.h"

#include <itkCommand.h>

//#CTORS/DTOR

QmitkDataStorageComboBox::QmitkDataStorageComboBox( QWidget* parent, bool _AutoSelectNewNodes )
: QComboBox(parent)
, m_DataStorage(0)
, m_Predicate(0)
, m_BlockEvents(false)
, m_AutoSelectNewNodes(_AutoSelectNewNodes)
{
  this->Init();
}

QmitkDataStorageComboBox::QmitkDataStorageComboBox( mitk::DataStorage* _DataStorage, const mitk::NodePredicateBase* _Predicate,
                                                   QWidget* parent, bool _AutoSelectNewNodes )
: QComboBox(parent)
, m_DataStorage(0)
, m_Predicate(_Predicate)
, m_BlockEvents(false)
, m_AutoSelectNewNodes(_AutoSelectNewNodes)
{
  // make connections, fill combobox
  this->Init();
  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageComboBox::~QmitkDataStorageComboBox()
{
}

//#PUBLIC GETTER
mitk::DataStorage::Pointer QmitkDataStorageComboBox::GetDataStorage() const
{
  return m_DataStorage.GetPointer();
}

const mitk::NodePredicateBase::ConstPointer QmitkDataStorageComboBox::GetPredicate() const
{
  return m_Predicate.GetPointer();
}

mitk::DataTreeNode::Pointer QmitkDataStorageComboBox::GetNode( int index ) const
{
  return (this->HasIndex(index))? m_Nodes.at(index): 0;
}

mitk::DataTreeNode::Pointer QmitkDataStorageComboBox::GetSelectedNode() const
{
  int _CurrentIndex = this->currentIndex();
  return (_CurrentIndex >= 0)? this->GetNode(_CurrentIndex): 0;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataStorageComboBox::GetNodes() const
{
  mitk::DataStorage::SetOfObjects::Pointer _SetOfObjects = mitk::DataStorage::SetOfObjects::New();

  for (std::vector<mitk::DataTreeNode*>::const_iterator it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
  {
    _SetOfObjects->push_back(*it);
  }

  return _SetOfObjects.GetPointer();
}

bool QmitkDataStorageComboBox::GetAutoSelectNewItems()
{
  return m_AutoSelectNewNodes;
}

//#PUBLIC SETTER
void QmitkDataStorageComboBox::SetDataStorage(mitk::DataStorage* _DataStorage)
{
  // reset only if datastorage really changed
  if(m_DataStorage.GetPointer() != _DataStorage)
  {
    // if there was an old storage, remove listeners
    if(m_DataStorage.IsNotNull())
    {
      this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageComboBox
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageComboBox::AddNode ) );

      this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkDataStorageComboBox
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageComboBox::RemoveNode ) );
    }
    // set new storage
    m_DataStorage = _DataStorage;

    // if there is a new storage, add listeners
    if(m_DataStorage.IsNotNull())
    {
      this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageComboBox
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageComboBox::AddNode ) );

      this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkDataStorageComboBox
        , const mitk::DataTreeNode*>( this, &QmitkDataStorageComboBox::RemoveNode ) );
    }

    // reset predicate to reset the combobox
    this->Reset();
  }
}
   
void QmitkDataStorageComboBox::SetPredicate(const mitk::NodePredicateBase* _Predicate)
{
  if(m_Predicate != _Predicate)
  {
    m_Predicate = _Predicate;
    this->Reset();
  }  
}

void QmitkDataStorageComboBox::AddNode( const mitk::DataTreeNode* _DataTreeNode )
{  
  // this is an event function, make sure that we didnt call ourself
  if(!m_BlockEvents)
  {
    m_BlockEvents = true;
    // pass a -1 to the InsertNode function in order to append the datatreenode to the end
    this->InsertNode(-1, _DataTreeNode);
    m_BlockEvents = false;
  }
}

void QmitkDataStorageComboBox::RemoveNode( int index )
{
  if(this->HasIndex(index))
  {
    //# remove itk::Event observer
    mitk::DataTreeNode* _DataTreeNode = m_Nodes.at(index);
    // get name property first
    mitk::BaseProperty* nameProperty = _DataTreeNode->GetProperty("name");
    // if prop exists remove modified listener
    if(nameProperty)
    {
      nameProperty->RemoveObserver(m_NodesModifiedObserverTags[index]);
      // remove name property map
      m_PropertyToNode.erase(_DataTreeNode);
    }
    // then remove delete listener on the node itself
    _DataTreeNode->RemoveObserver(m_NodesDeleteObserverTags[index]);
    // remove observer tags from lists
    m_NodesModifiedObserverTags.erase(m_NodesModifiedObserverTags.begin()+index);
    m_NodesDeleteObserverTags.erase(m_NodesDeleteObserverTags.begin()+index);
    // remove node name from combobox
    this->removeItem(index);
    // remove node from node vector
    m_Nodes.erase(m_Nodes.begin()+index);  
  }
}

void QmitkDataStorageComboBox::RemoveNode( const mitk::DataTreeNode* _DataTreeNode )
{
  // this is an event function, make sure that we didnt call ourself
  if(!m_BlockEvents)
  {
    m_BlockEvents = true;
    this->RemoveNode( this->Find(_DataTreeNode) );
    m_BlockEvents = false;
  }
}

void QmitkDataStorageComboBox::SetNode(int index, const mitk::DataTreeNode* _DataTreeNode)
{
  if(this->HasIndex(index))
  {
    this->InsertNode(index, _DataTreeNode);
  }
}
    
void QmitkDataStorageComboBox::SetNode( const mitk::DataTreeNode* _DataTreeNode, const mitk::DataTreeNode* _OtherDataTreeNode)
{
  this->SetNode( this->Find(_DataTreeNode), _OtherDataTreeNode);
}

void QmitkDataStorageComboBox::SetAutoSelectNewItems( bool _AutoSelectNewItems )
{
  m_AutoSelectNewNodes = _AutoSelectNewItems;
}

void QmitkDataStorageComboBox::OnDataTreeNodeDeleteOrModified(const itk::Object *caller, const itk::EventObject &event)
{  
  if(!m_BlockEvents)
  {
    m_BlockEvents = true;

    // check if we have a modified event (if not it is a delete event)
    const itk::ModifiedEvent* modifiedEvent = dynamic_cast<const itk::ModifiedEvent*>(&event);

    // when node was modified reset text
    if(modifiedEvent)
    {
      const mitk::BaseProperty* _NameProperty = dynamic_cast<const mitk::BaseProperty*>(caller);

      // node name changed, set it
      // but first of all find associated node
      for(std::map<mitk::DataTreeNode*, const mitk::BaseProperty*>::iterator it=m_PropertyToNode.begin()
        ; it!=m_PropertyToNode.end()
        ; ++it)
      {
        // property is found take node
        if(it->second == _NameProperty)
        {
          // looks strange but when calling setnode with the same node, that means the node gets updated
          this->SetNode(it->first, it->first);
          break;
        }
      }              
    }
    else
    {
      const mitk::DataTreeNode* _ConstDataTreeNode = dynamic_cast<const mitk::DataTreeNode*>(caller);
      if(_ConstDataTreeNode)
        // node will be deleted, remove it
        this->RemoveNode(_ConstDataTreeNode);
    }

    m_BlockEvents = false;
  }
}

void QmitkDataStorageComboBox::SetSelectedNode(mitk::DataTreeNode::Pointer item)
{
  int index = this->Find(item);
  if (index == -1)
  {
    LOG_INFO << "QmitkDataStorageComboBox: item not available";
  }
  else
  {
    this->setCurrentIndex(index);
  }
  
}

//#PROTECTED GETTER
bool QmitkDataStorageComboBox::HasIndex(unsigned int index) const
{
  return (m_Nodes.size() > 0 && index < m_Nodes.size());
}

int QmitkDataStorageComboBox::Find( const mitk::DataTreeNode* _DataTreeNode ) const
{
  int index = -1;

  std::vector<mitk::DataTreeNode*>::const_iterator nodeIt = 
    std::find(m_Nodes.begin(), m_Nodes.end(), _DataTreeNode);

  if(nodeIt != m_Nodes.end())
    index = std::distance(m_Nodes.begin(), nodeIt);

  return index;
}

//#PROTECTED SETTER
void QmitkDataStorageComboBox::OnCurrentIndexChanged(int index)
{
  if(index >= 0 && index < this->count())
    emit OnSelectionChanged(this->GetSelectedNode());
}

void QmitkDataStorageComboBox::InsertNode(int index, const mitk::DataTreeNode* _DataTreeNode)
{
  // check new or updated node first
  if(m_Predicate.IsNotNull() && !m_Predicate->CheckNode(_DataTreeNode))
    return;

  bool addNewNode = false;
  bool insertNewNode = false;
  bool changedNode = false;

  // if this->HasIndex(index), then a node shall be updated
  if(this->HasIndex(index))
  {
    // if we really have another node at this position then ...
    if(_DataTreeNode != m_Nodes.at(index))
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
  mitk::DataTreeNode* _NonConstDataTreeNode = const_cast<mitk::DataTreeNode*>(_DataTreeNode);
  mitk::BaseProperty* nameProperty = _NonConstDataTreeNode->GetProperty("name");

  if(!changedNode)
  {
    // break on duplicated nodes (that doesnt make sense to have duplicates in the combobox)
    if(this->Find(_DataTreeNode) != -1)
      return;

    // add modified observer
    itk::MemberCommand<QmitkDataStorageComboBox>::Pointer modifiedCommand = itk::MemberCommand<QmitkDataStorageComboBox>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkDataStorageComboBox::OnDataTreeNodeDeleteOrModified);
    // !!!! add modified observer for the name 
    /// property of the node because this is the only thing we are interested in !!!!!
    if(nameProperty)
    {
      m_NodesModifiedObserverTags.push_back( nameProperty->AddObserver(itk::ModifiedEvent(), modifiedCommand) );
      m_PropertyToNode[_NonConstDataTreeNode] = nameProperty;
    }
    // if there is no name node save an invalid value for the observer tag (-1)
    else
      m_NodesModifiedObserverTags.push_back( -1 );
    
    // add delete observer
    itk::MemberCommand<QmitkDataStorageComboBox>::Pointer deleteCommand = itk::MemberCommand<QmitkDataStorageComboBox>::New();
    deleteCommand->SetCallbackFunction(this, &QmitkDataStorageComboBox::OnDataTreeNodeDeleteOrModified);
    m_NodesDeleteObserverTags.push_back( _NonConstDataTreeNode->AddObserver(itk::DeleteEvent(), modifiedCommand) );
  }

  // add node to the vector
  if(addNewNode)
    m_Nodes.push_back( _NonConstDataTreeNode );
  else if(insertNewNode)
    m_Nodes.insert( m_Nodes.begin()+index, _NonConstDataTreeNode );

  // ... and to the combobox
  std::string _NonConstDataTreeNodeName = "unnamed node";
  // _NonConstDataTreeNodeName is "unnamed node" so far, change it if there is a name property in the node
  if(nameProperty)
    _NonConstDataTreeNodeName = nameProperty->GetValueAsString();

  if(addNewNode)
  {
    this->addItem(QString::fromStdString(_NonConstDataTreeNodeName));
    // select new node if m_AutoSelectNewNodes is true or if we have just added the first node
    if(m_AutoSelectNewNodes || m_Nodes.size() == 1)
      this->setCurrentIndex(index);
  }
  else
  {
    // update text in combobox
    this->setItemText( index, QString::fromStdString(_NonConstDataTreeNodeName));
  }  
}

void QmitkDataStorageComboBox::Init()
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
}

void QmitkDataStorageComboBox::Reset()
{
  // remove all nodes first
  while( !m_Nodes.empty() )
  {
    // remove last node
    this->RemoveNode( m_Nodes.size() - 1 );
  }

  // clear combobox
  this->clear();

  if(m_DataStorage.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects;

    // select all if predicate == NULL
    if (m_Predicate.IsNotNull())
      setOfObjects = m_DataStorage->GetSubset(m_Predicate);
    else
      setOfObjects = m_DataStorage->GetAll();

    // add all found nodes
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each _DataTreeNode
    {
      // add node to the node vector and to the combobox
      this->AddNode( nodeIt.Value().GetPointer() );
    }
  }
}
