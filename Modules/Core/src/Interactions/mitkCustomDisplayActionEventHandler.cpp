/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkCustomDisplayActionEventHandler.h"

// c++
#include <functional>

void mitk::CustomDisplayActionEventHandler::SetObservableBroadcast(mitk::DisplayActionEventBroadcast* observableBroadcast)
{
  if (m_ObservableBroadcast == observableBroadcast)
  {
    // no need to update the broadcast class
    return;
  }

  if (m_ObservableBroadcast.IsNotNull())
  {
    // remove current observer
    for (const auto& tag : m_ObserverTags)
    {
      m_ObservableBroadcast->RemoveObserver(tag);
    }
    // #TODO: decide if we want to move the observer to the new observable broadcast class
    m_ObserverTags.clear();
  }

  // set new broadcast class (possibly nullptr)
  m_ObservableBroadcast = observableBroadcast;
}

unsigned long mitk::CustomDisplayActionEventHandler::ConnectDisplayActionEvent(const mitk::DisplayActionEvent& displayActionEvent,
  const mitk::StdFunctionCommand::ActionFunction& actionFunction,
  const mitk::StdFunctionCommand::FilterFunction& filterFunction)
{
  // #TODO: change function call for new mitk::WeakPointer
  if (m_ObservableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before connecting events.";
  }

  auto command = mitk::StdFunctionCommand::New();
  command->SetCommandAction(actionFunction);
  command->SetCommandFilter(filterFunction);
  unsigned long tag = m_ObservableBroadcast->AddObserver(displayActionEvent, command);
  m_ObserverTags.push_back(tag);
  return tag;
}

void mitk::CustomDisplayActionEventHandler::DisconnectObserver(unsigned long observerTag)
{
  // #TODO: change function call for new mitk::WeakPointer
  if (m_ObservableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before disconnecting observer.";
  }

  std::vector<unsigned long>::iterator observerTagPosition = std::find(m_ObserverTags.begin(), m_ObserverTags.end(), observerTag);
  if (observerTagPosition != m_ObserverTags.end())
  {
    m_ObservableBroadcast->RemoveObserver(observerTag);
    m_ObserverTags.erase(observerTagPosition);
  }
}
