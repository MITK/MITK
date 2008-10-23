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

#include "mitkDataTreeStorage.h"

#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkGroupTagProperty.h"
#include "itkCommand.h"



mitk::DataTreeStorage::DataTreeStorage() 
: mitk::DataStorage(), m_ManageCompleteTree(true), m_DuringRemove(false)
{
  m_DataTree = NULL;
}


mitk::DataTreeStorage::~DataTreeStorage()
{
  if(m_DataTree.IsNotNull())
  {
    m_DataTree->RemoveObserver(m_DeleteInTreeObserverTag);
  }
  m_DataTree = NULL; 
}


void mitk::DataTreeStorage::Initialize(mitk::DataTree* tree)
{
  if (tree == NULL)
    throw 1;  // insert exception handling here 
  m_DataTree = tree;
  itk::ReceptorMemberCommand<Self>::Pointer command = itk::ReceptorMemberCommand<Self>::New();
  command->SetCallbackFunction(this, &mitk::DataTreeStorage::NodeDeletedInTree);
  m_DeleteInTreeObserverTag = m_DataTree->AddObserver(itk::TreeRemoveEvent<mitk::DataTreeBase>(), command);
}

void mitk::DataTreeStorage::NodeDeletedInTree(const itk::EventObject & treeChangedEvent)
{
  if (m_DuringRemove == true) // this notification is from our own Remove() method and should not be processed further)
    return;

  const itk::TreeRemoveEvent<mitk::DataTreeBase>* rme = dynamic_cast< const itk::TreeRemoveEvent<mitk::DataTreeBase>* >(&treeChangedEvent);
  if (rme == NULL)
    return;
  const mitk::DataTreeIteratorClone it(rme->GetChangePosition());
  mitk::DataTreeNode* node = it->Get();
  if (node == NULL)
    return;
  
  /* Notify observers of imminent node removal */
  EmitRemoveNodeEvent(node);

  /* remove the node from our relation, now that it was removed from the tree */
  this->RemoveFromRelation(node, m_SourceNodes);
  this->RemoveFromRelation(node, m_DerivedNodes);
}


bool mitk::DataTreeStorage::IsInitialized() const
{
  return m_DataTree.IsNotNull();
}


void mitk::DataTreeStorage::Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents)
{
  if (!IsInitialized())
    throw 1;  // insert exception handling here
  
  /* Check, if node is already in the DataTree */
  if (m_DataTree->Contains(node))
    throw 2;

  /* check if node is in its own list of sources */
  if ((parents != NULL) && (std::find(parents->begin(), parents->end(), node) != parents->end()))
    throw 3;

  /* save node in tree */
  node->SetProperty("IsDataStoreManaged", mitk::BoolProperty::New(true));
  mitk::DataTreeNode::ConstPointer parent;
  if ((parents != NULL) && (parents->Size() > 0))
    parent = parents->ElementAt(0);
  else 
    parent = NULL;
  DataTreeIteratorClone it = m_DataTree->GetIteratorToNode(parent);
  if (it->IsAtEnd()) 
    it->GoToBegin();
  it->Add(node); 
  /* create parentlist if it does not exist */
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
    mitk::DataTreeNode::ConstPointer parent = it.Value().GetPointer();
    mitk::DataStorage::SetOfObjects::ConstPointer derivedObjects = m_DerivedNodes[parent]; // get or create pointer to list of derived objects for that parent node
    if (derivedObjects.IsNull())
      m_DerivedNodes[parent] = mitk::DataStorage::SetOfObjects::New();  // Create a set of Objects, if it does not already exist
    mitk::DataStorage::SetOfObjects* deob = const_cast<mitk::DataStorage::SetOfObjects*>(m_DerivedNodes[parent].GetPointer());  // temporarily get rid of const pointer to insert new element
    deob->InsertElement(deob->Size(), node); // node is derived from parent. Insert it into the parents list of derived objects
  }
  /* Notify observers */
  EmitAddNodeEvent(node);
}


void mitk::DataTreeStorage::Remove(const mitk::DataTreeNode* node)
{
  if (!IsInitialized())
    throw 1;  // insert exception handling here
  if (node == NULL)
    return;

  mitk::DataTreeIteratorClone it = m_DataTree->GetIteratorToNode(node);   // search node in tree
  if (it->IsAtEnd())
    return;       // node not found
 
  m_DuringRemove = true;

  /* Notify observers of imminent node removal */
  EmitRemoveNodeEvent(node);

  /* remove node from tree, but keep its children */
  if (it->Disconnect() == false)
  {
    m_DuringRemove = false;
    throw 2;
  }
  
  /* remove node from both relation adjacency lists */
  this->RemoveFromRelation(node, m_SourceNodes);
  this->RemoveFromRelation(node, m_DerivedNodes);
  m_DuringRemove = false;

}


void mitk::DataTreeStorage::RemoveFromRelation(const mitk::DataTreeNode* node, AdjacencyList& relation)
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


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataTreeStorage::GetAll() const
{
  if (!IsInitialized())
    throw 1;  // insert exception handling here
  
  mitk::DataTreePreOrderIterator it(m_DataTree);
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  /* Fill resultset with all objects that are managed by the DataTreeStorage object */
  unsigned int index = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); it++)
  {
    mitk::DataTreeNode* node = it.Get();
    if (node == NULL) 
      continue;
    if (m_ManageCompleteTree == true)
      resultset->InsertElement(index++, node);
    else
      if (node->IsOn("IsDataStoreManaged",NULL, false) == true)  // check if node is managed by the DataTreeStorage object
        resultset->InsertElement(index++, node);
  }  
  return SetOfObjects::ConstPointer(resultset);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataTreeStorage::GetRelations(const mitk::DataTreeNode* node, const AdjacencyList& relation, const NodePredicateBase* condition, bool onlyDirectlyRelated) const
{
  if (node == NULL)
    throw 1;

  if ((m_ManageCompleteTree == false) && (node->IsOn("IsDataStoreManaged",NULL, false) == false))   // node is not managed by DataTreeStorage
    throw 2;

  if (m_DataTree->GetIteratorToNode(node)->IsAtEnd()) // node not in tree
  {
    return SetOfObjects::ConstPointer( SetOfObjects::New() );
    //throw 3;
  }

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
  std::vector<mitk::DataTreeNode::ConstPointer> resultset;
  std::vector<mitk::DataTreeNode::ConstPointer> openlist;

  /* Initialize openlist with node. this will add node to resultset, 
     but that is necessary to detect circular relations that would lead to endless recursion */
  openlist.push_back(node);

  while (openlist.size() > 0)
  {
    mitk::DataTreeNode::ConstPointer current = openlist.back();  // get element that needs to be processed
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
        mitk::DataTreeNode::ConstPointer p = parentIt.Value().GetPointer();
        if (   !(std::find(resultset.begin(), resultset.end(), p) != resultset.end())   // if it is not already in resultset 
            && !(std::find(openlist.begin(), openlist.end(), p) != openlist.end()))     // and not already in openlist
          openlist.push_back(p);                                                        // then add it to openlist, so that it can be processed
      }
  }

  /* now finally copy the results to a proper SetOfObjects variable exluding the initial node and checking the condition if any is given */
  mitk::DataStorage::SetOfObjects::Pointer realResultset = mitk::DataStorage::SetOfObjects::New();
  if (condition != NULL)
  {
    for (std::vector<mitk::DataTreeNode::ConstPointer>::iterator resultIt = resultset.begin(); resultIt != resultset.end(); resultIt++)
      if ((*resultIt != node) && (condition->CheckNode(*resultIt) == true))
        realResultset->InsertElement(realResultset->Size(), mitk::DataTreeNode::Pointer(const_cast<mitk::DataTreeNode*>((*resultIt).GetPointer())));
  }
  else
  {
    for (std::vector<mitk::DataTreeNode::ConstPointer>::iterator resultIt = resultset.begin(); resultIt != resultset.end(); resultIt++)
      if (*resultIt != node)
        realResultset->InsertElement(realResultset->Size(), mitk::DataTreeNode::Pointer(const_cast<mitk::DataTreeNode*>((*resultIt).GetPointer())));
  }
  return SetOfObjects::ConstPointer(realResultset);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataTreeStorage::GetSources(const mitk::DataTreeNode* node, const NodePredicateBase* condition, bool onlyDirectSources) const
{
  return this->GetRelations(node, m_SourceNodes, condition, onlyDirectSources);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataTreeStorage::GetDerivations(const mitk::DataTreeNode* node, const NodePredicateBase* condition, bool onlyDirectDerivations) const
{
  return this->GetRelations(node, m_DerivedNodes, condition, onlyDirectDerivations);
}


void mitk::DataTreeStorage::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "DataTreeStorage:\n";
  os << indent << "Using DataTree: " << m_DataTree << "\n";
  Superclass::PrintSelf(os, indent);
}
