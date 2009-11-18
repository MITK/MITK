#include "mitkDataStorageSelection.h"
#include <itkCommand.h>
#include <mitkBaseProperty.h>
#include <mitkPropertyList.h>

namespace mitk
{

  DataStorageSelection::DataStorageSelection()
    : m_DataStorage(0), m_Predicate(0), m_SelfCall(false)
  {
  }

  DataStorageSelection::DataStorageSelection(mitk::DataStorage* _DataStorage)
    : m_DataStorage(0), m_Predicate(0), m_SelfCall(false)
  {
    this->SetDataStorage(_DataStorage);
  }

  DataStorageSelection::DataStorageSelection(mitk::DataStorage* _DataStorage
                                                           , mitk::NodePredicateBase* _Predicate)
    : m_DataStorage(0), m_Predicate(_Predicate), m_SelfCall(false)
  {
    this->SetDataStorage(_DataStorage);
  }

  DataStorageSelection::~DataStorageSelection()
  {
    // kick datastorage and all nodes and all listeners
    this->SetDataStorage(0);
  }

  mitk::DataStorage::Pointer DataStorageSelection::GetDataStorage() const
  {
    return m_DataStorage;
  }

  mitk::NodePredicateBase::Pointer DataStorageSelection::GetPredicate() const
  {
    return m_Predicate;
  }

  unsigned int DataStorageSelection::GetSize() const
  {
    return m_Nodes.size();
  }

  mitk::DataTreeNode::Pointer DataStorageSelection::GetNode(unsigned int index) const
  {
    return (index < m_Nodes.size())? m_Nodes.at(index): 0;
  }

  std::vector<mitk::DataTreeNode*> DataStorageSelection::GetNodes() const
  {
    return m_Nodes;
  }

  void DataStorageSelection::AddListener(DataStorageSelection::Listener* listener)
  {
    m_Listener.insert(listener);
  }

  void DataStorageSelection::RemoveListener(DataStorageSelection::Listener* listener)
  {
    m_Listener.erase(listener);
  }

  void DataStorageSelection::SetDataStorage(mitk::DataStorage* _DataStorage)
  {
    // only proceed if we have a new datastorage
    if(m_DataStorage != _DataStorage)
    {
      // if a data storage was set before remove old event listeners
      if(m_DataStorage != 0)
      {
        this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataTreeNode*>( this, &DataStorageSelection::AddNode ) );

        this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataTreeNode*>( this, &DataStorageSelection::RemoveNode ) );

        m_DataStorage->RemoveObserver(m_DataStorageDeletedTag);
        m_DataStorageDeletedTag = 0;
      }

      // set new data storage
      m_DataStorage = _DataStorage;

      // if new storage is not 0 subscribe for events
      if(m_DataStorage != 0)
      {
        // subscribe for node added/removed events
        this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataTreeNode*>( this, &DataStorageSelection::AddNode ) );

        this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataTreeNode*>( this, &DataStorageSelection::RemoveNode ) );

        itk::MemberCommand<DataStorageSelection>::Pointer ObjectChangedCommand
          = itk::MemberCommand<DataStorageSelection>::New();
        ObjectChangedCommand->SetCallbackFunction(this, &DataStorageSelection::ObjectChanged);

        m_DataStorageDeletedTag = m_DataStorage->AddObserver(itk::DeleteEvent(), ObjectChangedCommand);
      }
      // Reset model (even if datastorage is 0->will be checked in Reset())
      this->Reset();
    }
  }

  void DataStorageSelection::SetPredicate(mitk::NodePredicateBase* _Predicate)
  {
    // ensure that a new predicate is set in order to avoid unnecessary changed events
    if(m_Predicate != _Predicate)
    {
      m_Predicate = _Predicate;
      this->Reset();
    }
  }

  void DataStorageSelection::AddNode(const mitk::DataTreeNode* node)
  {
    // garantuee no recursions when a new node event is thrown
    if(m_SelfCall)
      return;

    // if we have a predicate, check node against predicate first
    if(m_Predicate.IsNotNull() && !m_Predicate->CheckNode(node))
      return;

    // no duplicates
    if(std::find(m_Nodes.begin(), m_Nodes.end(), node) != m_Nodes.end())
      return;

    mitk::DataTreeNode* nonConstNode = const_cast<mitk::DataTreeNode*>(node);
    // add listener
    this->AddListener(nonConstNode);

    // add node
    m_Nodes.push_back(nonConstNode);


    for(std::set<Listener*>::iterator it=m_Listener.begin(); it!=m_Listener.end(); ++it)
    {
      (*it)->NodeAdded(node);
    }
  }


  void DataStorageSelection::RemoveNode(const mitk::DataTreeNode* node)
  {
    if(m_SelfCall)
      return;

    // find corresponding node
    std::vector<mitk::DataTreeNode*>::iterator nodeIt
      = std::find(m_Nodes.begin(), m_Nodes.end(), node);

    if(nodeIt == m_Nodes.end())
      return;

    mitk::DataTreeNode* nonConstNode = const_cast<mitk::DataTreeNode*>(node);
    // add listener
    this->RemoveListener(nonConstNode);

    // remove node
    m_Nodes.erase(nodeIt);

    for(std::set<Listener*>::iterator it=m_Listener.begin(); it!=m_Listener.end(); ++it)
    {
      (*it)->NodeRemoved(node);
    }

  }

  void DataStorageSelection::RemoveAllNodes()
  {
    // remove all nodes now (dont use iterators because removing elements
    // would invalidate the iterator)
    // start at the last element: first in, last out
    unsigned int i = m_Nodes.size();
    while(!m_Nodes.empty())
    {
      --i;
      this->RemoveNode(m_Nodes.at(i));
    }
  }

  void DataStorageSelection::ObjectChanged(const itk::Object *caller, const itk::EventObject &event)
  {
    if(m_SelfCall)
      return;

    const itk::DeleteEvent* delEvent = 0;
    const itk::ModifiedEvent* modifiedEvent = dynamic_cast<const itk::ModifiedEvent*>(&event);
    if(!modifiedEvent)
      delEvent = dynamic_cast<const itk::DeleteEvent*>(&event);

    const mitk::BaseProperty* prop = 0;
    const mitk::PropertyList* propList = 0;
    const mitk::DataTreeNode* node = dynamic_cast<const mitk::DataTreeNode*>(caller);
    if(!node)
    {
      if((prop = dynamic_cast<const mitk::BaseProperty*>(caller)))
      {
        node = this->FindNode(prop);
      }
      else if((propList = dynamic_cast<const mitk::PropertyList*>(caller)))
      {
        node = this->FindNode(propList);
      }
      else if(dynamic_cast<const mitk::DataStorage*>(caller))
      {
        this->SetDataStorage(0);
      }
    }

    if(node)
    {
      for(std::set<Listener*>::iterator it=m_Listener.begin(); it!=m_Listener.end(); ++it)
      {
        (*it)->NodeChanged(node);
        if(prop)
          (*it)->PropertyChanged(prop);
      }
    }
  }

  //# protected
  mitk::DataTreeNode::Pointer DataStorageSelection::FindNode(const mitk::BaseProperty* prop) const
  {
    mitk::DataTreeNode* node = 0;
    for(Nodes::const_iterator it=m_Nodes.begin(); it!=m_Nodes.end(); ++it)
    {
      for(mitk::PropertyList::PropertyMap::const_iterator it2
        = (*it)->GetPropertyList()->GetMap()->begin()
        ; it2 != (*it)->GetPropertyList()->GetMap()->end(); ++it2)
      {
        if(it2->second.first == prop)
        {
          node = *it;
          break;
        }
      }
    }
    return node;
  }

  mitk::DataTreeNode::Pointer DataStorageSelection::FindNode(const mitk::PropertyList* propList) const
  {
    mitk::DataTreeNode* node = 0;
    for(Nodes::const_iterator it=m_Nodes.begin(); it!=m_Nodes.end(); ++it)
    {
      if((*it)->GetPropertyList() == propList)
      {
        node = *it;
        break;
      }
    }
    return node;
  }

  void DataStorageSelection::Reset()
  {
    this->RemoveAllNodes();
    // the whole reset depends on the fact if a data storage is set or not
    if(m_DataStorage)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet;
      if(m_Predicate.IsNotNull())
        // get subset
        _NodeSet = m_DataStorage->GetSubset(m_Predicate);
      // if predicate is NULL, select all nodes
      else
      {
        _NodeSet = m_DataStorage->GetAll();
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

  void DataStorageSelection::RemoveListener(mitk::DataTreeNode* node)
  {
    // remove node listener
    node->RemoveObserver(m_NodeModifiedObserverTags[node]);
    m_NodeModifiedObserverTags.erase(node);

    // remove propertylist listener
    mitk::PropertyList* propList = node->GetPropertyList();
    propList->RemoveObserver(m_PropertyListModifiedObserverTags[propList]);
    m_PropertyListModifiedObserverTags.erase(propList);
    propList->RemoveObserver(m_PropertyListDeletedObserverTags[propList]);
    m_PropertyListDeletedObserverTags.erase(propList);

    mitk::BaseProperty* prop = 0;
    // do the same for each property
    for(mitk::PropertyList::PropertyMap::const_iterator it=propList->GetMap()->begin()
      ; it!=propList->GetMap()->end()
      ; ++it)
    {
      prop = it->second.first;
      prop->RemoveObserver(m_PropertyModifiedObserverTags[prop]);
      m_PropertyModifiedObserverTags.erase(prop);
      prop->RemoveObserver(m_PropertyDeletedObserverTags[prop]);
      m_PropertyDeletedObserverTags.erase(prop);
    }
  }

  void DataStorageSelection::AddListener(mitk::DataTreeNode* node)
  {
    // node listener
    itk::MemberCommand<DataStorageSelection>::Pointer ObjectChangedCommand
      = itk::MemberCommand<DataStorageSelection>::New();
    ObjectChangedCommand->SetCallbackFunction(this, &DataStorageSelection::ObjectChanged);

    m_NodeModifiedObserverTags[node] = node->AddObserver(itk::ModifiedEvent()
      , ObjectChangedCommand);

    // create propertylist listener
    mitk::PropertyList* propList = node->GetPropertyList();
    m_PropertyListModifiedObserverTags[propList] = propList
      ->AddObserver(itk::ModifiedEvent(), ObjectChangedCommand);
    m_PropertyListDeletedObserverTags[propList] = propList
      ->AddObserver(itk::DeleteEvent(), ObjectChangedCommand);

    mitk::BaseProperty* prop = 0;
    // do the same for each property
    for(mitk::PropertyList::PropertyMap::const_iterator it=propList->GetMap()->begin()
      ; it!=propList->GetMap()->end()
      ; ++it)
    {
      prop = it->second.first;
      m_PropertyModifiedObserverTags[prop] = prop->AddObserver(itk::ModifiedEvent()
        , ObjectChangedCommand);
      m_PropertyDeletedObserverTags[prop] = prop->AddObserver(itk::ModifiedEvent()
        , ObjectChangedCommand);
    }
  }

}
