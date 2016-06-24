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
#include "mitkInternalEvent.h"
#include "mitkInteractionEventObserver.h"
#include "usGetModuleContext.h"

namespace
{
  struct cmp
  {
    bool operator()(mitk::DataInteractor* d1, mitk::DataInteractor* d2)
    {
      return (d1->GetLayer() > d2->GetLayer());
    }
  };
}

mitk::Dispatcher::Dispatcher( const std::string& rendererName )
: m_ProcessingMode(REGULAR)
{
  // LDAP filter string to find all listeners specific for the renderer
  // corresponding to this dispatcher
  std::string specificRenderer = "(rendererName=" + rendererName +")";

  // LDAP filter string to find all listeners that are not specific
  // to any renderer
  std::string anyRenderer = "(!(rendererName=*))";

  // LDAP filter string to find only instances of  InteractionEventObserver
  std::string classInteractionEventObserver = "(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<InteractionEventObserver>() + ")";

  // Configure the LDAP filter to find all instances of InteractionEventObserver
  // that are specific to this dispatcher or unspecific to any dispatchers (real global listener)
  us::LDAPFilter filter( "(&(|"+ specificRenderer  + anyRenderer + ")"+classInteractionEventObserver+")" );

  // Give the filter to the ObserverTracker
  m_EventObserverTracker = new us::ServiceTracker<InteractionEventObserver>(us::GetModuleContext(), filter);
  m_EventObserverTracker->Open();
}

void mitk::Dispatcher::AddDataInteractor(const DataNode* dataNode)
{
  RemoveDataInteractor(dataNode);
  RemoveOrphanedInteractors();

  auto dataInteractor = dataNode->GetDataInteractor().GetPointer();

  if (dataInteractor != nullptr)
    m_Interactors.push_back(dataInteractor);
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
    if ((*it).IsNull() || (*it)->GetDataNode() == nullptr || (*it)->GetDataNode() == dataNode)
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
  m_EventObserverTracker->Close();
  delete m_EventObserverTracker;

  m_Interactors.clear();
}

bool mitk::Dispatcher::ProcessEvent(InteractionEvent* event)
{
  InteractionEvent::Pointer p = event;
  bool eventIsHandled = false;
  /* Filter out and handle Internal Events separately */
  InternalEvent* internalEvent = dynamic_cast<InternalEvent*>(event);
  if (internalEvent != nullptr)
  {
    eventIsHandled = HandleInternalEvent(internalEvent);
    // InternalEvents that are handled are not sent to the listeners
    if (eventIsHandled)
    {
      return true;
    }
  }
  switch (m_ProcessingMode)
  {
  case CONNECTEDMOUSEACTION:
    // finished connected mouse action
    if (std::strcmp(p->GetNameOfClass(), "MouseReleaseEvent") == 0)
    {
      m_ProcessingMode = REGULAR;

      if (m_SelectedInteractor.IsNotNull())
        eventIsHandled = m_SelectedInteractor->HandleEvent(event, m_SelectedInteractor->GetDataNode());

      m_SelectedInteractor = nullptr;
    }
    // give event to selected interactor
    if (eventIsHandled == false && m_SelectedInteractor.IsNotNull())
      eventIsHandled = m_SelectedInteractor->HandleEvent(event, m_SelectedInteractor->GetDataNode());

    break;

  case GRABINPUT:
    if (m_SelectedInteractor.IsNotNull())
    {
      eventIsHandled = m_SelectedInteractor->HandleEvent(event, m_SelectedInteractor->GetDataNode());
      SetEventProcessingMode(m_SelectedInteractor);
    }

    break;

  case PREFERINPUT:
    if (m_SelectedInteractor.IsNotNull() && m_SelectedInteractor->HandleEvent(event, m_SelectedInteractor->GetDataNode()) == true)
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
    if (std::strcmp(p->GetNameOfClass(), "MousePressEvent") == 0)
      event->GetSender()->GetRenderingManager()->SetRenderWindowFocus(event->GetSender()->GetRenderWindow());
    m_Interactors.sort(cmp()); // sorts interactors by layer (descending);

    // copy the list to prevent iterator invalidation as executing actions
    // in HandleEvent() can cause the m_Interactors list to be updated
    const ListInteractorType tmpInteractorList( m_Interactors );
    ListInteractorType::const_iterator it;
    for ( it=tmpInteractorList.cbegin(); it!=tmpInteractorList.cend(); ++it )
    {
      if ((*it).IsNotNull() && (*it)->HandleEvent(event, (*it)->GetDataNode()))
      {
        // Interactor can be deleted during HandleEvent(), so check it again
        if ((*it).IsNotNull()) {
          // if an event is handled several properties are checked, in order to determine the processing mode of the dispatcher
          SetEventProcessingMode(*it);
        }
        if (std::strcmp(p->GetNameOfClass(), "MousePressEvent") == 0 && m_ProcessingMode == REGULAR)
        {
          m_SelectedInteractor = *it;
          m_ProcessingMode = CONNECTEDMOUSEACTION;
        }
        eventIsHandled = true;
        break;
      }
    }
  }

  /* Notify InteractionEventObserver  */
  const std::vector<us::ServiceReference<InteractionEventObserver> > listEventObserver =
      m_EventObserverTracker->GetServiceReferences();
  for (std::vector<us::ServiceReference<InteractionEventObserver> >::const_iterator it = listEventObserver.cbegin();
       it != listEventObserver.cend(); ++it)
  {
    InteractionEventObserver* interactionEventObserver = m_EventObserverTracker->GetService(*it);
    if (interactionEventObserver != nullptr)
    {
      if (interactionEventObserver->IsEnabled())
      {
        interactionEventObserver->Notify(event, eventIsHandled);
      }
    }
  }

  // Process event queue
  if (!m_QueuedEvents.empty())
  {
    InteractionEvent::Pointer e = m_QueuedEvents.front();
    m_QueuedEvents.pop_front();
    ProcessEvent(e);
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
    if ((*it).IsNull())
    {
      it = m_Interactors.erase(it);
    }
    else
    {
      DataNode::Pointer node = (*it)->GetDataNode();

      if (node.IsNull())
      {
        it = m_Interactors.erase(it);
      }
      else
      {
        DataInteractor::Pointer interactor = node->GetDataInteractor();

        if (interactor != it->GetPointer())
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
}

void mitk::Dispatcher::QueueEvent(InteractionEvent* event)
{
  m_QueuedEvents.push_back(event);
}

void mitk::Dispatcher::SetEventProcessingMode(DataInteractor* dataInteractor)
{
  m_ProcessingMode = dataInteractor->GetMode();
  if (dataInteractor->GetMode() != REGULAR)
  {
    m_SelectedInteractor = dataInteractor;
  }
}

bool mitk::Dispatcher::HandleInternalEvent(InternalEvent* internalEvent)
{
  if (internalEvent->GetSignalName() == DataInteractor::IntDeactivateMe &&
      internalEvent->GetTargetInteractor() != nullptr)
  {
    internalEvent->GetTargetInteractor()->GetDataNode()->SetDataInteractor(nullptr);
    internalEvent->GetTargetInteractor()->SetDataNode(nullptr);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  return false;
}
