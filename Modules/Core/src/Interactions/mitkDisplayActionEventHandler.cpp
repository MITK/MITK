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

#include "mitkDisplayActionEventHandler.h"

void mitk::DisplayActionEventHandler::SetObservableBroadcast(DisplayActionEventBroadcast* observableBroadcast)
{
  if (m_ObservableBroadcast == observableBroadcast)
  {
    // no need to update the broadcast class
    return;
  }

  if (!m_ObservableBroadcast.IsExpired())
  {
    auto observableBroadcastPtr = m_ObservableBroadcast.Lock();

    // remove current observer
    for (const auto& tag : m_ObserverTags)
    {
      observableBroadcastPtr->RemoveObserver(tag);
    }
    m_ObserverTags.clear();
  }

  // set new broadcast class
  m_ObservableBroadcast = observableBroadcast;
}

mitk::DisplayActionEventHandler::OberserverTagType mitk::DisplayActionEventHandler::ConnectDisplayActionEvent(const DisplayActionEvent& displayActionEvent,
  const StdFunctionCommand::ActionFunction& actionFunction,
  const StdFunctionCommand::FilterFunction& filterFunction)
{
  if (m_ObservableBroadcast.IsExpired())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before connecting events.";
  }

  auto observableBroadcast = m_ObservableBroadcast.Lock();
  auto command = StdFunctionCommand::New();
  command->SetCommandAction(actionFunction);
  command->SetCommandFilter(filterFunction);
  OberserverTagType tag = observableBroadcast->AddObserver(displayActionEvent, command);
  m_ObserverTags.push_back(tag);
  return tag;
}

void mitk::DisplayActionEventHandler::DisconnectObserver(OberserverTagType observerTag)
{
  if (m_ObservableBroadcast.IsExpired())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before disconnecting observer.";
  }

  auto observableBroadcast = m_ObservableBroadcast.Lock();
  std::vector<OberserverTagType>::iterator observerTagPosition = std::find(m_ObserverTags.begin(), m_ObserverTags.end(), observerTag);
  if (observerTagPosition != m_ObserverTags.end())
  {
    observableBroadcast->RemoveObserver(observerTag);
    m_ObserverTags.erase(observerTagPosition);
  }
}
