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

void mitk::InformerService::RegisterObserver(EventObserver::Pointer eventObserver)
{
  for (std::list<EventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    if (*it == eventObserver)
    {
      return;
    }
  }
  m_ListObserver.push_back(eventObserver);
}

void mitk::InformerService::UnRegisterObserver(EventObserver::Pointer eventObserver)
{
  m_ListObserver.remove(eventObserver);
}

void mitk::InformerService::NotifyObservers(InteractionEvent::Pointer interactionEvent)
{
  for (std::list<EventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    (*it)->Notify(interactionEvent);
  }
}

mitk::InformerService::~InformerService()
{
  m_ListObserver.clear();
}
