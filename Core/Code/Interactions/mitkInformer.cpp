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

void mitk::InformerService::RegisterObserver(InteractionEventObserver* InteractionEventObserver)
{
  // explicitly use smart pointer, else the search for it will fail
  // especially std::find() seems to fail on list of smart pointer objects
  InteractionEventObserver::Pointer pInteractionEventObserver = InteractionEventObserver;
  for (std::list<InteractionEventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    if (*it == pInteractionEventObserver)
    {
      return;
    }
  }
  m_ListObserver.push_back(pInteractionEventObserver);
}

void mitk::InformerService::UnRegisterObserver(InteractionEventObserver* InteractionEventObserver)
{
  InteractionEventObserver::Pointer pInteractionEventObserver = InteractionEventObserver;
  m_ListObserver.remove(pInteractionEventObserver);
}

void mitk::InformerService::NotifyObservers(InteractionEvent::Pointer interactionEvent, bool isHandled)
{
  for (std::list<InteractionEventObserver::Pointer>::iterator it = m_ListObserver.begin(); it != m_ListObserver.end(); ++it)
  {
    (*it)->Notify(interactionEvent, isHandled);
  }
}

mitk::InformerService::~InformerService()
{
  m_ListObserver.clear();
}
