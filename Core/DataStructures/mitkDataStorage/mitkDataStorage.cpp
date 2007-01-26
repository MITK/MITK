/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkDataStorage.h"

#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"

mitk::DataStorage::DataStorage() 
: itk::Object(), m_ManageCompleteTree(true) // true by default until all Reliver functionalities use the datastorage properly
{
  m_DataTree = NULL;
}

mitk::DataStorage::~DataStorage()
{
  m_DataTree = NULL;
}

void mitk::DataStorage::Initialize(mitk::DataTree* tree)
{
  if (tree == NULL)
    throw 1;  // insert exception handling here 
  m_DataTree = tree;
}

inline bool mitk::DataStorage::IsInitialized()
{
  return m_DataTree.IsNotNull();
}

void mitk::DataStorage::Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents)
{
  if (!IsInitialized())
    throw 1;  // insert exception handling here
  /* Check, if node is already in the DataTree */
  if (m_DataTree->Contains(node))
    throw 2;
  /* save node in tree */
  mitk::DataTreePreOrderIterator it(m_DataTree);
  node->SetProperty("IsDataStoreManaged", new mitk::BoolProperty(true));
  it.Add(node);
  /* save node and parentlist in relations map */
  m_CreatedByRelations.insert(std::make_pair(node, SetOfObjects::ConstPointer(parents))); 

}

void mitk::DataStorage::Update(mitk::DataTreeNode* node)
{
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSubset(const NodePredicateBase& condition)
{
  if (!IsInitialized())
    throw 1;  // insert exception handling here

  /* Fill resultset with objects that fullfill the condition */
  mitk::DataTreePreOrderIterator it(m_DataTree);
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  unsigned int index = 0;
  if (m_ManageCompleteTree == true)
    for (it.GoToBegin(); !it.IsAtEnd(); it++)
    {
      mitk::DataTreeNode* node = it.Get();
      if (node == NULL)
        continue;
      if (condition.CheckNode(node) == true)  // check all elements in the datatree
        resultset->InsertElement(index++, node);
    } 
  else
    for (it.GoToBegin(); !it.IsAtEnd(); it++)
    {
      mitk::DataTreeNode* node = it.Get();
      if (node == NULL)
        continue;
      if ((node->IsOn("IsDataStoreManaged",NULL, false) == true) && (condition.CheckNode(node) == true))  // check if node is managed by the datastorage object
        resultset->InsertElement(index++, node);
    } 
  return SetOfObjects::ConstPointer( resultset );
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetAll()
{
  mitk::DataTreePreOrderIterator it(m_DataTree);
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  /* Fill resultset with all objects that are managed by the DataStorage object */
  unsigned int index = 0;
  if (m_ManageCompleteTree == true)
    for (it.GoToBegin(); !it.IsAtEnd(); it++)
    {
      mitk::DataTreeNode* node = it.Get();
      if (node == NULL) 
        continue;
      resultset->InsertElement(index++, node);
    }  
  else
    for (it.GoToBegin(); !it.IsAtEnd(); it++)
    {
      mitk::DataTreeNode* node = it.Get();
      if (node == NULL) 
        continue;
      if (node->IsOn("IsDataStoreManaged",NULL, false) == true)  // check if node is managed by the datastorage object
        resultset->InsertElement(index++, node);
    }  
  return SetOfObjects::ConstPointer(resultset);
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSources(mitk::DataTreeNode::Pointer node, bool onlyDirectSources)
{
  if (node.IsNull())
    throw 1;

  if (onlyDirectSources)
  {
    AdjacencyList::iterator it = m_CreatedByRelations.find(node); // get parents of current node
    if (   (it == m_CreatedByRelations.end()) // node not found in list
        || (it->second.IsNull())              // or no set of parents
        || (it->second->Size() == 0))         // or empty set
      return SetOfObjects::ConstPointer(mitk::DataStorage::SetOfObjects::New());  // return an empty set
    else
      return it->second;
  }
  
  std::vector<mitk::DataTreeNode::Pointer> resultset;
  std::vector<mitk::DataTreeNode::Pointer> openlist;

  /* initialize openlist with node. this will add node to resultset, 
     but that is necessary to detect circular relations that would lead to endless recursion */
  openlist.push_back(node);

  while (openlist.size() > 0)
  {
    mitk::DataTreeNode::Pointer current = openlist.back();  // get element that needs to be processed
    openlist.pop_back();            // remove last element, because it gets processed now
    resultset.push_back(current);   // add current element to resultset
    AdjacencyList::iterator it = m_CreatedByRelations.find(current); // get parents of current node
    if (   (it == m_CreatedByRelations.end()) // if node not found in list
        || (it->second.IsNull())              // or no set of parents available
        || (it->second->Size() == 0))         // or empty set of parents
      continue;                               // then continue with next node in open list
    else
      for (SetOfObjects::ConstIterator parentIt = it->second->Begin(); parentIt != it->second->End(); ++parentIt) // for each parent of current node
      {
        mitk::DataTreeNode::Pointer p = parentIt.Value();
        if (   !(std::find(resultset.begin(), resultset.end(), p) != resultset.end())   // if it is not already in resultset 
            && !(std::find(openlist.begin(), openlist.end(), p) != openlist.end()))     // and not already in openlist
          openlist.push_back(p);                                                        // then add it to openlist, so that it can be processed
      }
  }
  /* now finally copy the results to a proper SetOfObjects variable exluding the initial node */
  mitk::DataStorage::SetOfObjects::Pointer realResultset = mitk::DataStorage::SetOfObjects::New();
  for (std::vector<mitk::DataTreeNode::Pointer>::iterator resultIt = resultset.begin(); resultIt != resultset.end(); resultIt++)
    if (*resultIt != node)
      realResultset->InsertElement(realResultset->Size(), *resultIt);
  
  return SetOfObjects::ConstPointer(realResultset); // return a const pointer
}
