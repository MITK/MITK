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

#include "QmitkFunctionality.h"
#include "internal/QmitkFunctionalityUtil.h"

#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>

#include <mitkDataNodeObject.h>

std::vector<mitk::DataNode*> QmitkFunctionality::GetCurrentSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return this->DataNodeSelectionToVector(currentSelection);
}

std::vector<mitk::DataNode*> QmitkFunctionality::GetDataManagerSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return this->DataNodeSelectionToVector(currentSelection);
}

void QmitkFunctionality::OnSelectionChanged(std::vector<mitk::DataNode*> /*nodes*/)
{
}

std::vector<mitk::DataNode*> QmitkFunctionality::DataNodeSelectionToVector(mitk::DataNodeSelection::ConstPointer currentSelection) const
{

  std::vector<mitk::DataNode*> selectedNodes;
  if(currentSelection.IsNull())
    return selectedNodes;

  mitk::DataNodeObject* _DataNodeObject = 0;
  mitk::DataNode* _DataNode = 0;

  for(mitk::DataNodeSelection::iterator it = currentSelection->Begin();
    it != currentSelection->End(); ++it)
  {
    _DataNodeObject = dynamic_cast<mitk::DataNodeObject*>((*it).GetPointer());
    if(_DataNodeObject)
    {
      _DataNode = _DataNodeObject->GetDataNode();
      if(_DataNode)
        selectedNodes.push_back(_DataNode);
    }
  }

  return selectedNodes;
}

void QmitkFunctionality::NodeAddedProxy( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeAdded(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }

}

void QmitkFunctionality::NodeAdded( const mitk::DataNode*  /*node*/ )
{

}

void QmitkFunctionality::NodeRemovedProxy( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeRemoved(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }
}

void QmitkFunctionality::NodeRemoved( const mitk::DataNode*  /*node*/ )
{

}

void QmitkFunctionality::NodeChanged( const mitk::DataNode* /*node*/ )
{

}

void QmitkFunctionality::NodeChangedProxy( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeChanged(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }
}

void QmitkFunctionality::FireNodeSelected( mitk::DataNode* node )
{
  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);
  this->FireNodesSelected(nodes);
}

void QmitkFunctionality::FireNodesSelected( std::vector<mitk::DataNode*> nodes )
{
  if( !m_SelectionProvider )
    return;

  std::vector<mitk::DataNode::Pointer> nodesSmartPointers;
  for (std::vector<mitk::DataNode*>::iterator it = nodes.begin()
    ; it != nodes.end(); it++)
  {
    nodesSmartPointers.push_back( *it );
  }
  m_SelectionProvider->FireNodesSelected(nodesSmartPointers);

}
