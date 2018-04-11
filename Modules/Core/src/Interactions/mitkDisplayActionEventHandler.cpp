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

void mitk::DisplayActionEventHandler::SetObservableBroadcast(mitk::DisplayActionEventBroadcast* observableBroadcast)
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
    m_ObserverTags.clear();
  }

  // set new broadcast class (possibly nullptr)
  m_ObservableBroadcast = observableBroadcast;
}

mitk::DisplayActionEventHandler::OberserverTagType mitk::DisplayActionEventHandler::ConnectDisplayActionEvent(const mitk::DisplayActionEvent& displayActionEvent,
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
  OberserverTagType tag = m_ObservableBroadcast->AddObserver(displayActionEvent, command);
  m_ObserverTags.push_back(tag);
  return tag;
}

void mitk::DisplayActionEventHandler::DisconnectObserver(OberserverTagType observerTag)
{
  // #TODO: change function call for new mitk::WeakPointer
  if (m_ObservableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before disconnecting observer.";
  }

  std::vector<OberserverTagType>::iterator observerTagPosition = std::find(m_ObserverTags.begin(), m_ObserverTags.end(), observerTag);
  if (observerTagPosition != m_ObserverTags.end())
  {
    m_ObservableBroadcast->RemoveObserver(observerTag);
    m_ObserverTags.erase(observerTagPosition);
  }
}
