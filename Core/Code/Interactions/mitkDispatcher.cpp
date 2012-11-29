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

typedef std::list<mitk::EventInteractor::Pointer> ListInteractor;

mitk::Dispatcher::Dispatcher()
{
}

/*
 * Sets/Adds the EventInteractor that is associated with the DataNode to the Dispatcher Queue.
 * If there already exists an EventInteractor that has a reference to the same DataNode, is is removed.
 *
 */
void mitk::Dispatcher::SetEventInteractor(const DataNode* dataNode)
{
  RemoveEventInteractor(dataNode);
  RemoveAbandonedInteractors();
  mitk::EventInteractor* eventInteractor = dynamic_cast<mitk::EventInteractor*>(dataNode->GetDataInteractor());
  if (eventInteractor != NULL)
  {
    m_Interactors.push_back(eventInteractor);
  }
}

/*
 * Remove all Interactors related to this Node, to prevent double entries and dead references.
 */
void mitk::Dispatcher::RemoveEventInteractor(const DataNode* dataNode)
{
  for (ListInteractor::iterator it = m_Interactors.begin(); it != m_Interactors.end();)
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
  // TODO Auto-generated destructor stub
}

bool mitk::Dispatcher::ProcessEvent(mitk::InteractionEvent* event)
{
  mitk::InteractionEvent::Pointer p = event;
  m_Interactors.sort();
  return false; // TODO: dummy implementation
}

/**
 * Checks if DataNodes associated with Interactors point back to them.
 * If not remove the Interactor. (This can happen when s.o. tries to set DataNodes to multiple Interactors)
 */
void mitk::Dispatcher::RemoveAbandonedInteractors()
{
  for (ListInteractor::iterator it = m_Interactors.begin(); it != m_Interactors.end();)
  {
    mitk::DataNode::Pointer dn = (*it)->GetDataNode();
    if (dn.IsNull()) {
      it = m_Interactors.erase(it);
    } else {
    mitk::EventInteractor* interactor = dn->GetDataInteractor();
    if (!(interactor == (*it).GetPointer()))
      it = m_Interactors.erase(it);
    else
      ++it;
    }
  }
}
