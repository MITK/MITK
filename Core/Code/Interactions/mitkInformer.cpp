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

#include "mitkInformer.h"
#include <mitkModule.h>
#include <mitkGetModuleContext.h>
#include <mitkModuleRegistry.h>



mitk::InformerService::InformerService()
{
}

void mitk::InformerService::RegisterObserver(EventObserver* eventObserver)
{
  EventObserver::Pointer pEventObserver = eventObserver;
  for (std::list<EventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    if (*it == pEventObserver)
    {
      return;
    }
  }
  m_ListObserver.push_back(pEventObserver);
}

void mitk::InformerService::UnRegisterObserver(EventObserver* eventObserver)
{
  EventObserver::Pointer pEventObserver = eventObserver;
  m_ListObserver.remove(pEventObserver);
}

void mitk::InformerService::NotifyObservers(InteractionEvent::Pointer interactionEvent, bool isHandled)
{
  for (std::list<EventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    (*it)->Notify(interactionEvent, isHandled);
  }
}

mitk::InformerService::~InformerService()
{
  m_ListObserver.clear();
}
