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

#include "mitkDataStorage.h"

#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkDataTreeStorage.h"

#include "itkCommand.h"

mitk::DataStorage::Pointer mitk::DataStorage::s_Instance = NULL;


mitk::DataStorage::DataStorage() 
: itk::Object()
, m_BlockNodeModifiedEvents(false)
{
}


mitk::DataStorage::~DataStorage()
{
  // workaround for bug #343: do another UnRegister in case we re-create a DataStorage 
  // which happens when a PlugIn is re-initialized within Chili
  if(s_Instance.IsNotNull())
  {
    mitk::DataTreeStorage* dts = dynamic_cast<mitk::DataTreeStorage*>(s_Instance.GetPointer());
    if(dts)
      dts->m_DataTree->UnRegister();
  }
}


mitk::DataStorage* mitk::DataStorage::CreateInstance(mitk::DataTree* tree)
{
  // workaround for bug #343: do another UnRegister in case we re-create a DataStorage 
  // which happens when a PlugIn is re-initialized within Chili
  if(s_Instance.IsNotNull())
  {
    mitk::DataTreeStorage* dts = dynamic_cast<mitk::DataTreeStorage*>(s_Instance.GetPointer());
    if (dts != NULL)
      dts->m_DataTree->UnRegister();
  }
  s_Instance = NULL;
  try
  {
    mitk::DataTreeStorage::Pointer dts = mitk::DataTreeStorage::New();
    dts->Initialize(tree);   // If no DataStorage created yet, and tree is NULL, then this will raise an exception, because the DataStorage must be initialized with a DataTree
    /// \todo additional register on tree needed to prevent crash in itkTimeStamp.cxx (see bug #343)
    tree->Register();
    s_Instance = dts;
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
  {
      itkGenericOutputMacro("Trying to use mitk::DataStorage::GetInstance() " 
        << "without an available singleton instance. Either no instance has "
        << "been created (use DataStorage::CreateInstance) or it has already "
        << "been destroyed.");
      return NULL;
  }
  else  
    return s_Instance;
}


void mitk::DataStorage::Add(mitk::DataTreeNode* node, mitk::DataTreeNode* parent)
{
  mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
  parents->InsertElement(0, parent);
  this->Add(node, parents);
}


void mitk::DataStorage::Remove(const mitk::DataStorage::SetOfObjects* nodes)
{
  if (nodes == NULL)
    return;
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); it++)
    this->Remove(it.Value());
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSubset(const NodePredicateBase& condition) const
{
  mitk::DataStorage::SetOfObjects::ConstPointer result = this->FilterSetOfObjects(this->GetAll(), &condition);
  return result;
}


mitk::DataTreeNode* mitk::DataStorage::GetNamedNode(const char* name) const
{
  if (name == NULL)
    return NULL;

  mitk::StringProperty::Pointer s(mitk::StringProperty::New(name));
  mitk::NodePredicateProperty p("name", s);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(p);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}


mitk::DataTreeNode* mitk::DataStorage::GetNode(const NodePredicateBase* condition) const
{
  if (condition == NULL)
    return NULL;
  
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(*condition);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}

mitk::DataTreeNode* mitk::DataStorage::GetNamedDerivedNode(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations) const
{
  if (name == NULL)
    return NULL;

  mitk::StringProperty::Pointer s(mitk::StringProperty::New(name));
  mitk::NodePredicateProperty p("name", s);
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


const mitk::DataTreeNode::GroupTagList mitk::DataStorage::GetGroupTags() const
{
  DataTreeNode::GroupTagList result;
  SetOfObjects::ConstPointer all = this->GetAll();
  if (all.IsNull())
    return result;

  for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = all->Begin(); nodeIt != all->End(); nodeIt++)  // for each node
  {
    mitk::PropertyList* pl = nodeIt.Value()->GetPropertyList();
    for (mitk::PropertyList::PropertyMap::const_iterator propIt = pl->GetMap()->begin(); propIt != pl->GetMap()->end(); propIt++)
      if (dynamic_cast<mitk::GroupTagProperty*>(propIt->second.first.GetPointer()) != NULL)
        result.insert(propIt->first);
  }

  return result;
}


void mitk::DataStorage::ShutdownSingleton()
{
  if(s_Instance.IsNotNull())
  {
    mitk::DataTreeStorage* dts = dynamic_cast<mitk::DataTreeStorage*>(s_Instance.GetPointer());
    dts->m_DataTree->UnRegister();
  }
  s_Instance = NULL; 
}


void mitk::DataStorage::EmitAddNodeEvent(const mitk::DataTreeNode* node)
{
  AddNodeEvent.Send(node);
}


void mitk::DataStorage::EmitRemoveNodeEvent(const mitk::DataTreeNode* node)
{
  RemoveNodeEvent.Send(node);
}

void mitk::DataStorage::OnNodeModified( const itk::Object *caller, const itk::EventObject &event )
{
  if(m_BlockNodeModifiedEvents)
    return;

  const mitk::DataTreeNode* modifiedNode = dynamic_cast<const mitk::DataTreeNode*>(caller);
  if(modifiedNode)
  {
    ChangedNodeEvent.Send(modifiedNode);
  }
}

void mitk::DataStorage::AddModifiedListener( const mitk::DataTreeNode* _Node )
{
  // node must not be 0 and must not be yet registered
  if(_Node && m_NodeModifiedObserverTags.find(_Node) == m_NodeModifiedObserverTags.end())
  {
    itk::MemberCommand<mitk::DataStorage>::Pointer nodeModifiedCommand =
      itk::MemberCommand<mitk::DataStorage>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &DataStorage::OnNodeModified);
    m_NodeModifiedObserverTags[_Node] 
      = _Node->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);
  }
}

void mitk::DataStorage::RemoveModifiedListener( const mitk::DataTreeNode* _Node )
{
  // node must not be 0 and must be registered
  if(_Node && m_NodeModifiedObserverTags.find(_Node) != m_NodeModifiedObserverTags.end())
  {
    unsigned long tag = m_NodeModifiedObserverTags.find(_Node)->second;
    // const cast is bad! but sometimes it is necessary. removing an observer does not really
    // touch the internal state
    (const_cast<mitk::DataTreeNode*>(_Node))->RemoveObserver(tag);
  }
}