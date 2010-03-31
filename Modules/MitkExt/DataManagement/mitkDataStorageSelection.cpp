/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataStorageSelection.h"
#include <itkCommand.h>
#include <mitkBaseProperty.h>
#include <mitkPropertyList.h>

namespace mitk
{

  DataStorageSelection::DataStorageSelection(mitk::DataStorage* _DataStorage, bool _AutoAddNodes)
    : m_DataStorage(0), m_Predicate(0), m_SelfCall(false), m_AutoAddNodes(_AutoAddNodes)
  {
    this->SetDataStorage(_DataStorage);
  }

  DataStorageSelection::DataStorageSelection(mitk::DataStorage* _DataStorage
                                                           , mitk::NodePredicateBase* _Predicate
                                                           , bool _AutoAddNodes)
    : m_DataStorage(0), m_Predicate(_Predicate), m_SelfCall(false), m_AutoAddNodes(_AutoAddNodes)
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

  mitk::DataNode::Pointer DataStorageSelection::GetNode() const
  {
    return this->GetNode(0);
  }

  mitk::DataNode::Pointer DataStorageSelection::GetNode(unsigned int index) const
  {
    return (index < m_Nodes.size())? m_Nodes.at(index): 0;
  }

  std::vector<mitk::DataNode*> DataStorageSelection::GetNodes() const
  {
    return m_Nodes;
  }


  bool DataStorageSelection::DoesAutoAddNodes() const
  {
    return m_AutoAddNodes;
  }

  DataStorageSelection& DataStorageSelection::operator=(mitk::DataNode* node)
  {
    this->RemoveAllNodes();
    this->AddNode(node);
    return *this;
  }

  DataStorageSelection& DataStorageSelection::operator=(mitk::DataNode::Pointer node)
  {
    *this = node.GetPointer();
    return *this;
  }

  void DataStorageSelection::SetDataStorage(mitk::DataStorage* _DataStorage)
  {
    // only proceed if we have a new datastorage
    if(m_DataStorage != _DataStorage)
    {
      // if a data storage was set before remove old event listeners
      if(m_DataStorage != 0)
      {
        if(m_AutoAddNodes)
          this->m_DataStorage->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataNode*>( this, &DataStorageSelection::AddNode ) );

        this->m_DataStorage->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataNode*>( this, &DataStorageSelection::RemoveNode ) );

        m_DataStorage->RemoveObserver(m_DataStorageDeletedTag);
        m_DataStorageDeletedTag = 0;
      }

      // set new data storage
      m_DataStorage = _DataStorage;

      // if new storage is not 0 subscribe for events
      if(m_DataStorage != 0)
      {
        // subscribe for node added/removed events
        if(m_AutoAddNodes)
          this->m_DataStorage->AddNodeEvent.AddListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataNode*>( this, &DataStorageSelection::AddNode ) );

        this->m_DataStorage->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<DataStorageSelection
          , const mitk::DataNode*>( this, &DataStorageSelection::RemoveNode ) );

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

  void DataStorageSelection::AddNode(const mitk::DataNode* node)
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

    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    // add listener
    this->AddListener(nonConstNode);

    // add node
    m_Nodes.push_back(nonConstNode);


    NodeAdded.Send(node);
  }


  void DataStorageSelection::RemoveNode(const mitk::DataNode* node)
  {
    if(m_SelfCall)
      return;

    // find corresponding node
    std::vector<mitk::DataNode*>::iterator nodeIt
      = std::find(m_Nodes.begin(), m_Nodes.end(), node);

    if(nodeIt == m_Nodes.end())
      return;

    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    // add listener
    this->RemoveListener(nonConstNode);

    // remove node
    m_Nodes.erase(nodeIt);

    NodeRemoved.Send(node);

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
    const mitk::DataNode* node = dynamic_cast<const mitk::DataNode*>(caller);
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

    if(prop && node)
    {
      PropertyChanged.Send(node, prop);
    }
    else if(node)
    {
      NodeChanged.Send(node);
    }
  }

  //# protected
  mitk::DataNode::Pointer DataStorageSelection::FindNode(const mitk::BaseProperty* prop) const
  {
    mitk::DataNode* node = 0;
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

  mitk::DataNode::Pointer DataStorageSelection::FindNode(const mitk::PropertyList* propList) const
  {
    mitk::DataNode* node = 0;
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
      mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = 0;
      if(m_AutoAddNodes && m_Predicate.IsNotNull())
        // get subset
        _NodeSet = m_DataStorage->GetSubset(m_Predicate);
      // if predicate is NULL, select all nodes
      else if(m_AutoAddNodes)
      {
        _NodeSet = m_DataStorage->GetAll();
      }
      else
        return;
      // finally add all nodes to the model
      for(mitk::DataStorage::SetOfObjects::const_iterator it=_NodeSet->begin(); it!=_NodeSet->end()
        ; it++)
      {
        // save node
        this->AddNode(*it);
      }
    }
  }

  void DataStorageSelection::RemoveListener(mitk::DataNode* node)
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

  void DataStorageSelection::AddListener(mitk::DataNode* node)
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
