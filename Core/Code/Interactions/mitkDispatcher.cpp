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
  m_ProcessingMode = REGULAR;
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
  InteractionEvent::Pointer p = event;
  bool eventIsHandled = false;
  switch (m_ProcessingMode)
  {
  case CONNECTEDMOUSEACTION:
    // finished connected mouse action
    if (p->GetEventClass() == "MouseReleaseEvent")
    {
      m_ProcessingMode = REGULAR;
      eventIsHandled = m_SelectedInteractor->HandleEvent(event);
    }
    // give event to selected interactor
    if (eventIsHandled == false)
    {
      eventIsHandled = m_SelectedInteractor->HandleEvent(event);
    }
    break;

  case GRABINPUT:
    eventIsHandled = m_SelectedInteractor->HandleEvent(event);
    SetEventProcessingMode(m_SelectedInteractor);
    break;

  case PREFERINPUT:
    if (m_SelectedInteractor->HandleEvent(event) == true)
    {
      SetEventProcessingMode(m_SelectedInteractor);
      eventIsHandled = true;
    }
    break;

  case REGULAR:
    break;
  }
  // Standard behavior. Is executed in STANDARD mode  and PREFERINPUT mode, if preferred interactor rejects event.
  if (m_ProcessingMode == REGULAR || (m_ProcessingMode == PREFERINPUT && eventIsHandled == false))
  {
    // FixME sorting seems not to work
    m_Interactors.sort(cmp());// sorts interactors by layer (descending);
    for (std::list<DataInteractor::Pointer>::iterator it = m_Interactors.begin(); it != m_Interactors.end() && eventIsHandled == false;
        ++it)
    {
      if ((*it)->HandleEvent(event))
      { // if an event is handled several properties are checked, in order to determine the processing mode of the dispatcher
        SetEventProcessingMode(*it);
        if (p->GetEventClass() == "MousePressEvent" && m_ProcessingMode == REGULAR)
        {
          m_SelectedInteractor = *it;
          m_ProcessingMode = CONNECTEDMOUSEACTION;
        }
        eventIsHandled = true;
      }
    }
  }

  // TODO: inform listeners

  // Process event queue
  if (!m_QueuedEvents.empty())
  {
    InteractionEvent::Pointer e = m_QueuedEvents.front();
    m_QueuedEvents.pop_front();
    ProcessEvent(e.GetPointer());
  }
  return eventIsHandled;
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

void mitk::Dispatcher::QueueEvent(InteractionEvent* event)
{
  InteractionEvent::Pointer e = event;
  m_QueuedEvents.push_back(e);
}

void mitk::Dispatcher::SetEventProcessingMode(DataInteractor::Pointer dataInteractor)
{
  if (dataInteractor->GetMode() == "REGULAR")
  {
    m_ProcessingMode = REGULAR;
  }

  // prefer/grab input overrule connected mouse action
  if (dataInteractor->GetMode() == "PREFER_INPUT")
  {
    m_ProcessingMode = PREFERINPUT;
    m_SelectedInteractor = dataInteractor;
  }
  if (dataInteractor->GetMode() == "GRAB_INPUT")
  {
    m_ProcessingMode = GRABINPUT;
    m_SelectedInteractor = dataInteractor;
  }
}
