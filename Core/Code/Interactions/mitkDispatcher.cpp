/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkDispatcher.h"
#include "mitkInteractionEvent.h"

mitk::Dispatcher::Dispatcher()
{
}

void mitk::Dispatcher::AddDataInteractor(const DataNode* dataNode)
{
  RemoveDataInteractor(dataNode);
  RemoveOrphanedInteractors();
  DataInteractor::Pointer dataInteractor = dataNode->GetDataInteractor();
  if (dataInteractor.IsNotNull())
  {
    m_Interactors.push_back(dataInteractor);
  }
}

/*
 * Note: One DataInteractor can only have one DataNode and vice versa,
 * BUT the m_Interactors list may contain another DataInteractor that is still connected to this DataNode,
 * in this case we have to remove >1 DataInteractor. (Some special case of switching DataNodes between DataInteractors and registering a
 * DataNode to a DataStorage after assigning it to an DataInteractor)
 */

void mitk::Dispatcher::RemoveDataInteractor(const DataNode* dataNode)
{
  for (ListInteractorType::iterator it = m_Interactors.begin(); it != m_Interactors.end();)
  {
    if ((*it)->GetDataNode() == dataNode)
    {
      it = m_Interactors.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

size_t mitk::Dispatcher::GetNumberOfInteractors()
{
  return m_Interactors.size();
}

mitk::Dispatcher::~Dispatcher()
{
}

bool mitk::Dispatcher::ProcessEvent(InteractionEvent* event)
{
//  MITK_INFO << "Event received " << event->GetEventClass();
  InteractionEvent::Pointer p = event;
  m_Interactors.sort(); // sorts interactors by layer (descending);
  for (std::list<DataInteractor::Pointer>::iterator it = m_Interactors.begin(); it != m_Interactors.end(); ++it)
  {
    if ((*it)->HandleEvent(event))
    {
      return true;
    }
  }
  return false;
}

/*
 * Checks if DataNodes associated with DataInteractors point back to them.
 * If not remove the DataInteractors. (This can happen when s.o. tries to set DataNodes to multiple DataInteractors)
 */
void mitk::Dispatcher::RemoveOrphanedInteractors()
{
  for (ListInteractorType::iterator it = m_Interactors.begin(); it != m_Interactors.end();)
  {
    DataNode::Pointer dn = (*it)->GetDataNode();
    if (dn.IsNull())
    {
      it = m_Interactors.erase(it);
    }
    else
    {
      mitk::DataInteractor::Pointer interactor = dn->GetDataInteractor();
      if (!(interactor == (*it).GetPointer()))
      {
        it = m_Interactors.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}
