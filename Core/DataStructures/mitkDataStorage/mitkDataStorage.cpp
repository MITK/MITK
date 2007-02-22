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
#include "mitkNodePredicateProperty.h"


mitk::DataStorage::Pointer mitk::DataStorage::s_Instance = NULL;


mitk::DataStorage::DataStorage() 
: itk::Object(), m_ManageCompleteTree(true) // true by default until all Reliver functionalities use the datastorage properly
{
  m_DataTree = NULL;
}


mitk::DataStorage::~DataStorage()
{  
  m_DataTree = NULL; 
}


mitk::DataStorage* mitk::DataStorage::CreateInstance(mitk::DataTree* tree)
{
  s_Instance = NULL;
  try
  {
    s_Instance = mitk::DataStorage::New();
    s_Instance->Initialize(tree);   // If no datastore created yet, and tree is NULL, then this will raise an exception, because the datastorage must be initialized with a datatree
    tree->Register();   // needed to prevent premature destruction of the datatree (e.g. before the static DataStorage is destroyed)
  }
  catch(...)
  {
    s_Instance = NULL;
    throw 1;  // insert exception handling here
  }
  return s_Instance;
}


mitk::DataStorage* mitk::DataStorage::GetInstance()
{
  if (DataStorage::s_Instance.IsNull())
   throw 1;  // insert exception handling here
  else  
    return s_Instance;
}


void mitk::DataStorage::Initialize(mitk::DataTree* tree)
{
  if (tree == NULL)
    throw 1;  // insert exception handling here 
  m_DataTree = tree;
}


inline bool mitk::DataStorage::IsInitialized() const
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
    mitk::DataTreeNode::ConstPointer parent = it.Value();
    mitk::DataStorage::SetOfObjects::ConstPointer derivedObjects = m_DerivedNodes[parent]; // get or create pointer to list of derived objects for that parent node
    if (derivedObjects.IsNull())
      m_DerivedNodes[parent] = mitk::DataStorage::SetOfObjects::New();  // Create a set of Objects, if it does not already exist
    mitk::DataStorage::SetOfObjects* deob = const_cast<mitk::DataStorage::SetOfObjects*>(m_DerivedNodes[parent].GetPointer());  // temporarily get rid of const pointer to insert new element
    deob->InsertElement(deob->Size(), node); // node is derived from parent. Insert it into the parents list of derived objects
  }
}


void mitk::DataStorage::Add(mitk::DataTreeNode* node, mitk::DataTreeNode* parent)
{
  mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
  parents->InsertElement(0, parent);
  this->Add(node, parents);
}


void mitk::DataStorage::Update(mitk::DataTreeNode* node)
{
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSubset(const NodePredicateBase& condition) const
{
  mitk::DataStorage::SetOfObjects::ConstPointer result = this->FilterSetOfObjects(this->GetAll(), &condition);
  return result;
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetAll() const
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


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetRelations(const mitk::DataTreeNode* node, const AdjacencyList& relation, const NodePredicateBase* condition, bool onlyDirectlyRelated) const
{
  if (node == NULL)
    throw 1;

  if (onlyDirectlyRelated)
  {
    AdjacencyList::const_iterator it = relation.find(node); // get parents of current node
    if ((it == relation.end()) || (it->second.IsNull())) // node not found in list or no set of parents
      return SetOfObjects::ConstPointer(mitk::DataStorage::SetOfObjects::New());  // return an empty set
    else
      return this->FilterSetOfObjects(it->second, condition);
  }
  
  std::vector<mitk::DataTreeNode::ConstPointer> resultset;
  std::vector<mitk::DataTreeNode::ConstPointer> openlist;

  /* initialize openlist with node. this will add node to resultset, 
     but that is necessary to detect circular relations that would lead to endless recursion */
  openlist.push_back(node);

  while (openlist.size() > 0)
  {
    mitk::DataTreeNode::ConstPointer current = openlist.back();  // get element that needs to be processed
    openlist.pop_back();            // remove last element, because it gets processed now
    resultset.push_back(current);   // add current element to resultset
    AdjacencyList::const_iterator it = relation.find(current); // get parents of current node
    if (   (it == relation.end()) // if node not found in list
        || (it->second.IsNull())              // or no set of parents available
        || (it->second->Size() == 0))         // or empty set of parents
      continue;                               // then continue with next node in open list
    else
      for (SetOfObjects::ConstIterator parentIt = it->second->Begin(); parentIt != it->second->End(); ++parentIt) // for each parent of current node
      {
        mitk::DataTreeNode::ConstPointer p = parentIt.Value();
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


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSources(const mitk::DataTreeNode* node, const NodePredicateBase* condition, bool onlyDirectSources) const
{
  return this->GetRelations(node, m_SourceNodes, condition, onlyDirectSources);
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetDerivations(const mitk::DataTreeNode* node, const NodePredicateBase* condition, bool onlyDirectDerivations) const
{
  return this->GetRelations(node, m_DerivedNodes, condition, onlyDirectDerivations);
}


mitk::DataTreeNode* mitk::DataStorage::GetNamedNode(const char* name) const
{
  if (name == NULL)
    return NULL;

  mitk::StringProperty s(name);
  mitk::NodePredicateProperty p("name", &s);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(p);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}


mitk::DataTreeNode* mitk::DataStorage::GetNamedDerivedNode(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations) const
{
  if (name == NULL)
    return NULL;

  mitk::StringProperty s(name);
  mitk::NodePredicateProperty p("name", &s);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDerivations(sourceNode, &p, onlyDirectDerivations);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}


void mitk::DataStorage::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  //Superclass::PrintSelf(os, indent);
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
  os << indent << "DataStorage " << this << " is managing " << all->Size() << " objects. List of objects:" << std::endl;
  for (mitk::DataStorage::SetOfObjects::ConstIterator allIt = all->Begin(); allIt != all->End(); allIt++)
  {
    std::string name;
    allIt.Value()->GetName(name);
    std::string datatype;
    if (allIt.Value()->GetData() != NULL)
      datatype = allIt.Value()->GetData()->GetNameOfClass();
    os << indent << " " << allIt.Value().GetPointer() << "<" << datatype << ">: " << name << std::endl;    
    mitk::DataStorage::SetOfObjects::ConstPointer parents = this->GetSources(allIt.Value());
    if (parents->Size() > 0)
    {
      os << indent << "  Direct sources: ";
      for (mitk::DataStorage::SetOfObjects::ConstIterator parentIt = parents->Begin(); parentIt != parents->End(); parentIt++)
        os << parentIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
    mitk::DataStorage::SetOfObjects::ConstPointer derivations = this->GetDerivations(allIt.Value());
    if (derivations->Size() > 0)
    {
      os << indent << "  Direct derivations: ";
      for (mitk::DataStorage::SetOfObjects::ConstIterator derivationIt = derivations->Begin(); derivationIt != derivations->End(); derivationIt++)
        os << derivationIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
  }
  os << std::endl;
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::FilterSetOfObjects(const SetOfObjects* set, const NodePredicateBase* condition) const
{  
  if (set == NULL)
    return NULL;
  if (condition == NULL)
    return set;
  mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = set->Begin(); it != set->End(); it++)
    if (condition->CheckNode(it.Value()) == true)
      result->InsertElement(result->Size(), it.Value());
  return mitk::DataStorage::SetOfObjects::ConstPointer(result);
}
