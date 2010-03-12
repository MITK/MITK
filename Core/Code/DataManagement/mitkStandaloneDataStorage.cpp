/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkStandaloneDataStorage.h"

#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkGroupTagProperty.h"
#include "itkSimpleFastMutexLock.h"
#include "itkMutexLockHolder.h"


mitk::StandaloneDataStorage::StandaloneDataStorage() 
: mitk::DataStorage()
{
}


mitk::StandaloneDataStorage::~StandaloneDataStorage()
{
  for(AdjacencyList::iterator it = m_SourceNodes.begin();
    it != m_SourceNodes.end(); it++)
  {
    this->RemoveListeners(it->first);
  }
}




bool mitk::StandaloneDataStorage::IsInitialized() const
{
  return true;
}


void mitk::StandaloneDataStorage::Add(mitk::DataNode* node, const mitk::DataStorage::SetOfObjects* parents)
{
  {
    itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_Mutex);
    if (!IsInitialized())
      throw std::logic_error("DataStorage not initialized");
    /* check if node is in its own list of sources */
    if ((parents != NULL) && (std::find(parents->begin(), parents->end(), node) != parents->end()))
      throw std::invalid_argument("Node is it's own parent"); 
    /* check if node already exists in StandaloneDataStorage */
    if (m_SourceNodes.find(node) != m_SourceNodes.end())
      throw std::invalid_argument("Node is already in DataStorage");

    /* create parent list if it does not exist */
    mitk::DataStorage::SetOfObjects::ConstPointer sp;
    if (parents != NULL)
      sp = parents;
    else
      sp = mitk::DataStorage::SetOfObjects::New();
    /* Store node and parent list in sources adjacency list */
    m_SourceNodes.insert(std::make_pair(node, sp));

    /* Store node and an empty children list in derivations adjacency list */
    mitk::DataStorage::SetOfObjects::Pointer children = mitk::DataStorage::SetOfObjects::New();
    m_DerivedNodes.insert(std::make_pair(node, children));

    /* create entry in derivations adjacency list for each parent of the new node */
    for (SetOfObjects::ConstIterator it = sp->Begin(); it != sp->End(); it++)
    {
      mitk::DataNode::ConstPointer parent = it.Value().GetPointer();
      mitk::DataStorage::SetOfObjects::ConstPointer derivedObjects = m_DerivedNodes[parent]; // get or create pointer to list of derived objects for that parent node
      if (derivedObjects.IsNull())
        m_DerivedNodes[parent] = mitk::DataStorage::SetOfObjects::New();  // Create a set of Objects, if it does not already exist
      mitk::DataStorage::SetOfObjects* deob = const_cast<mitk::DataStorage::SetOfObjects*>(m_DerivedNodes[parent].GetPointer());  // temporarily get rid of const pointer to insert new element
      deob->InsertElement(deob->Size(), node); // node is derived from parent. Insert it into the parents list of derived objects
    }

    // register for ITK changed events
    this->AddListeners(node);
  }

  /* Notify observers */
  EmitAddNodeEvent(node);
  
}


void mitk::StandaloneDataStorage::Remove(const mitk::DataNode* node)
{
  if (!IsInitialized())
    throw std::logic_error("DataStorage not initialized");
  if (node == NULL)
    return;

  // remove ITK modified event listener
  this->RemoveListeners(node);

  /* Notify observers of imminent node removal */
  EmitRemoveNodeEvent(node);
  {
    itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_Mutex);
    /* remove node from both relation adjacency lists */
    this->RemoveFromRelation(node, m_SourceNodes);
    this->RemoveFromRelation(node, m_DerivedNodes);
  }
}

bool mitk::StandaloneDataStorage::Exists(const mitk::DataNode* node) const
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_Mutex);
  return (m_SourceNodes.find(node) != m_SourceNodes.end());
}

void mitk::StandaloneDataStorage::RemoveFromRelation(const mitk::DataNode* node, AdjacencyList& relation)
{
  for (AdjacencyList::const_iterator mapIter = relation.begin(); mapIter != relation.end(); ++mapIter)  // for each node in the relation
    if (mapIter->second.IsNotNull())      // if node has a relation list
    {
      SetOfObjects::Pointer s = const_cast<SetOfObjects*>(mapIter->second.GetPointer());   // search for node to be deleted in the relation list
      SetOfObjects::STLContainerType::iterator relationListIter = std::find(s->begin(),  s->end(), node);   // this assumes, that the relation list does not contain duplicates (which should be safe to assume)
      if (relationListIter != s->end())     // if node to be deleted is in relation list
        s->erase(relationListIter);         // remove it from parentlist
    }
  /* now remove node from the relation */
  AdjacencyList::iterator adIt;
  adIt = relation.find(node);
  if (adIt != relation.end())
    relation.erase(adIt);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::StandaloneDataStorage::GetAll() const
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock > locked(m_Mutex);
  if (!IsInitialized())
    throw std::logic_error("DataStorage not initialized");
    
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  /* Fill resultset with all objects that are managed by the StandaloneDataStorage object */
  unsigned int index = 0;
  for (AdjacencyList::const_iterator it = m_SourceNodes.begin(); it != m_SourceNodes.end(); ++it)
    if (it->first.IsNull()) 
      continue;
    else
      resultset->InsertElement(index++, const_cast<mitk::DataNode*>(it->first.GetPointer()));

  return SetOfObjects::ConstPointer(resultset);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::StandaloneDataStorage::GetRelations(const mitk::DataNode* node, const AdjacencyList& relation, const NodePredicateBase* condition, bool onlyDirectlyRelated) const
{
  if (node == NULL)
    throw std::invalid_argument("invalid node");

  /* Either read direct relations directly from adjacency list */
  if (onlyDirectlyRelated)  
  {
    AdjacencyList::const_iterator it = relation.find(node); // get parents of current node
    if ((it == relation.end()) || (it->second.IsNull())) // node not found in list or no set of parents
      return SetOfObjects::ConstPointer(mitk::DataStorage::SetOfObjects::New());  // return an empty set
    else
      return this->FilterSetOfObjects(it->second, condition);
  }
  
  /* Or traverse adjacency list to collect all related nodes */
  std::vector<mitk::DataNode::ConstPointer> resultset;
  std::vector<mitk::DataNode::ConstPointer> openlist;

  /* Initialize openlist with node. this will add node to resultset, 
     but that is necessary to detect circular relations that would lead to endless recursion */
  openlist.push_back(node);

  while (openlist.size() > 0)
  {
    mitk::DataNode::ConstPointer current = openlist.back();  // get element that needs to be processed
    openlist.pop_back();                      // remove last element, because it gets processed now
    resultset.push_back(current);             // add current element to resultset
    AdjacencyList::const_iterator it = relation.find(current); // get parents of current node
    if (   (it == relation.end())             // if node not found in list
        || (it->second.IsNull())              // or no set of parents available
        || (it->second->Size() == 0))         // or empty set of parents
      continue;                               // then continue with next node in open list
    else
      for (SetOfObjects::ConstIterator parentIt = it->second->Begin(); parentIt != it->second->End(); ++parentIt) // for each parent of current node
      {
        mitk::DataNode::ConstPointer p = parentIt.Value().GetPointer();
        if (   !(std::find(resultset.begin(), resultset.end(), p) != resultset.end())   // if it is not already in resultset 
            && !(std::find(openlist.begin(), openlist.end(), p) != openlist.end()))     // and not already in openlist
          openlist.push_back(p);                                                        // then add it to openlist, so that it can be processed
      }
  }

  /* now finally copy the results to a proper SetOfObjects variable exluding the initial node and checking the condition if any is given */
  mitk::DataStorage::SetOfObjects::Pointer realResultset = mitk::DataStorage::SetOfObjects::New();
  if (condition != NULL)
  {
    for (std::vector<mitk::DataNode::ConstPointer>::iterator resultIt = resultset.begin(); resultIt != resultset.end(); resultIt++)
      if ((*resultIt != node) && (condition->CheckNode(*resultIt) == true))
        realResultset->InsertElement(realResultset->Size(), mitk::DataNode::Pointer(const_cast<mitk::DataNode*>((*resultIt).GetPointer())));
  }
  else
  {
    for (std::vector<mitk::DataNode::ConstPointer>::iterator resultIt = resultset.begin(); resultIt != resultset.end(); resultIt++)
      if (*resultIt != node)
        realResultset->InsertElement(realResultset->Size(), mitk::DataNode::Pointer(const_cast<mitk::DataNode*>((*resultIt).GetPointer())));
  }
  return SetOfObjects::ConstPointer(realResultset);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::StandaloneDataStorage::GetSources(const mitk::DataNode* node, const NodePredicateBase* condition, bool onlyDirectSources) const
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_Mutex);
  return this->GetRelations(node, m_SourceNodes, condition, onlyDirectSources);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::StandaloneDataStorage::GetDerivations(const mitk::DataNode* node, const NodePredicateBase* condition, bool onlyDirectDerivations) const
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock>locked(m_Mutex);
  return this->GetRelations(node, m_DerivedNodes, condition, onlyDirectDerivations);
}


void mitk::StandaloneDataStorage::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "StandaloneDataStorage:\n";
  Superclass::PrintSelf(os, indent);
}
