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

void mitk::Dispatcher::AddEventInteractor(const DataNode* dataNode)
{
  RemoveEventInteractor(dataNode);
  RemoveOrphanedInteractors();
  EventInteractor::Pointer eventInteractor = dataNode->GetDataInteractor();
  if (eventInteractor.IsNotNull())
  {
    m_Interactors.push_back(eventInteractor);
  }
}

/*
 * Note: One EventInteractor can only have one DataNode and vice versa,
 * BUT the m_Interactors list may contain another EventInteractor that is still connected to this DataNode,
 * in this case we have to remove >1 EventInteractor. (Some special case of switching DataNodes between EventInteractors and registering a
 * DataNode to a DataStorage after assigning it to an EventInteractor)
 */

void mitk::Dispatcher::RemoveEventInteractor(const DataNode* dataNode)
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
  InteractionEvent::Pointer p = event;
  m_Interactors.sort();
  return false; // TODO: dummy implementation
}

/*
 * Checks if DataNodes associated with EventInteractors point back to them.
 * If not remove the EventInteractors. (This can happen when s.o. tries to set DataNodes to multiple EventInteractors)
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
      mitk::EventInteractor::Pointer interactor = dn->GetDataInteractor();
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
