/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDisplayActionEventHandler.h"

mitk::DisplayActionEventHandler::~DisplayActionEventHandler()
{
  auto observableBroadcastPtr = m_ObservableBroadcast.Lock();

  if (observableBroadcastPtr.IsNotNull())
  {
    // remove current observer
    for (const auto& tag : m_ObserverTags)
    {
      observableBroadcastPtr->RemoveObserver(tag);
    }
    m_ObserverTags.clear();
  }
}

void mitk::DisplayActionEventHandler::SetObservableBroadcast(DisplayActionEventBroadcast* observableBroadcast)
{
  if (m_ObservableBroadcast == observableBroadcast)
  {
    // no need to update the broadcast class
    return;
  }

  auto observableBroadcastPtr = m_ObservableBroadcast.Lock();

  if (observableBroadcastPtr.IsNotNull())
  {
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
  auto observableBroadcast = m_ObservableBroadcast.Lock();

  if (observableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before connecting events.";
  }

  auto command = StdFunctionCommand::New();
  command->SetCommandAction(actionFunction);
  command->SetCommandFilter(filterFunction);
  OberserverTagType tag = observableBroadcast->AddObserver(displayActionEvent, command);
  m_ObserverTags.push_back(tag);
  return tag;
}

void mitk::DisplayActionEventHandler::DisconnectObserver(OberserverTagType observerTag)
{
  auto observableBroadcast = m_ObservableBroadcast.Lock();

  if (observableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before disconnecting observer.";
  }

  std::vector<OberserverTagType>::iterator observerTagPosition = std::find(m_ObserverTags.begin(), m_ObserverTags.end(), observerTag);
  if (observerTagPosition != m_ObserverTags.end())
  {
    observableBroadcast->RemoveObserver(observerTag);
    m_ObserverTags.erase(observerTagPosition);
  }
}

void mitk::DisplayActionEventHandler::InitActions()
{
  auto observableBroadcast = m_ObservableBroadcast.Lock();

  if (observableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before initializing actions.";
  }

  // remove all current display action events as observer
  auto allObserverTags = GetAllObserverTags();
  for (const auto& tag : allObserverTags)
  {
    observableBroadcast->RemoveObserver(tag);
  }
  m_ObserverTags.clear();

  InitActionsImpl();
}
