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
  m_CreatedByRelations.insert(std::make_pair(node, parents)); 

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
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  //std::vector<mitk::DataTreeNode::Pointer> stlresult = resultset->CastToSTLContainer();

  AdjacencyMatrix::iterator it = m_CreatedByRelations.find(node); // check, if node is in the relations data structure
  if (    (it == m_CreatedByRelations.end()) // node not found in list
       || (it->second.IsNull())              // or no set of parents
       || (it->second->Size() == 0))         // or empty set
    return SetOfObjects::ConstPointer(resultset);  // return an empty set (stop criterion for recursive call)
  
  SetOfObjects::ConstPointer parents = it->second;  // get parents of current node

  if (onlyDirectSources == true)
    return parents;  // return the nodes direct parents
  else  // recursively collect parents of parents too
  {
    for (SetOfObjects::ConstIterator parentIt = parents->Begin(); parentIt != parents->End(); parentIt++) // for each parent
    {
      mitk::DataTreeNode::Pointer parent = parentIt.Value();
      mitk::DataStorage::SetOfObjects::ConstPointer s = this->GetSources(parent, false);  // get all parents of our parent
      //stlresult.push_back(parent); // resultset is the current parent 
      resultset->InsertElement(resultset->Size(), parent); // add current parent to resultset
      for (SetOfObjects::ConstIterator parentsParentIt = s->Begin(); parentsParentIt != s->End(); parentsParentIt++)  // plus all parents of current parent
        resultset->InsertElement(resultset->Size(), parentsParentIt->Value());
        //stlresult.push_back(parentsParentIt->Value());
    }
    return SetOfObjects::ConstPointer(resultset);  // return all parents
  }
}
