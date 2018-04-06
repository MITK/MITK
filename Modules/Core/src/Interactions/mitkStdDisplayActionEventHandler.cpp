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

#include "mitkStdDisplayActionEventHandler.h"

// mitk core
#include "mitkBaseRenderer.h"
#include "mitkCameraController.h"
#include "mitkDisplayActionEvents.h"
#include "mitkStdFunctionCommand.h"

// itk
#include <itkEventObject.h>

void mitk::StdDisplayActionEventHandler::SetObservableBroadcast(mitk::DisplayActionEventBroadcast* observableBroadcast)
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

  // set new (possibly nullptr) broadcast class
  m_ObservableBroadcast = observableBroadcast;
}

void mitk::StdDisplayActionEventHandler::InitStdActions()
{
  // #TODO: change function call for new mitk::WeakPointer
  if (m_ObservableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before initializing actions.";
  }

  ConnectDisplayMoveEvent();
  ConnectDisplaySetCrosshairEvent();
  ConnectDisplayZoomEvent();
  ConnectDisplayScrollEvent();
}

void mitk::StdDisplayActionEventHandler::ConnectDisplayMoveEvent()
{
  auto command = StdFunctionCommand::New();
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayMoveEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayMoveEvent* displayActionEvent = dynamic_cast<const DisplayMoveEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetInteractionEvent()->GetSender();

      // concrete action
      sendingRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
      sendingRenderer->GetRenderingManager()->RequestUpdate(sendingRenderer->GetRenderWindow());
    }
  };

  command->SetCommandAction(actionFunction);
  unsigned long tag = m_ObservableBroadcast->AddObserver(DisplayMoveEvent(nullptr, Vector2D()), command);
  m_ObserverTags.push_back(tag);
}

void mitk::StdDisplayActionEventHandler::ConnectDisplaySetCrosshairEvent()
{
  auto command = StdFunctionCommand::New();
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetInteractionEvent()->GetSender();

      // concrete action
      auto allRenderWindows = sendingRenderer->GetRenderingManager()->GetAllRegisteredRenderWindows();
      for (auto renWin : allRenderWindows)
      {
        if (BaseRenderer::GetInstance(renWin)->GetMapperID() == BaseRenderer::Standard2D && renWin != sendingRenderer->GetRenderWindow())
        {
          BaseRenderer::GetInstance(renWin)->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
        }
      }
    }
  };

  command->SetCommandAction(actionFunction);
  unsigned long tag = m_ObservableBroadcast->AddObserver(DisplaySetCrosshairEvent(nullptr, Point3D()), command);
  m_ObserverTags.push_back(tag);
}

void mitk::StdDisplayActionEventHandler::ConnectDisplayZoomEvent()
{
  auto command = StdFunctionCommand::New();
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayZoomEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayZoomEvent* displayActionEvent = dynamic_cast<const DisplayZoomEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetInteractionEvent()->GetSender();

      // concrete action
      if (1.0 != displayActionEvent->GetZoomFactor())
      {
        sendingRenderer->GetCameraController()->Zoom(displayActionEvent->GetZoomFactor(), displayActionEvent->GetStartCoordinate());
        sendingRenderer->GetRenderingManager()->RequestUpdate(sendingRenderer->GetRenderWindow());
      }
    }
  };

  command->SetCommandAction(actionFunction);
  unsigned long tag = m_ObservableBroadcast->AddObserver(DisplayZoomEvent(nullptr, 0.0, Point2D()), command);
  m_ObserverTags.push_back(tag);
}

void mitk::StdDisplayActionEventHandler::ConnectDisplayScrollEvent()
{
  auto command = StdFunctionCommand::New();
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetInteractionEvent()->GetSender();

      // concrete action
      sendingRenderer->GetSliceNavigationController()->GetSlice()->SetPos(displayActionEvent->GetPosition());
    }
  };

  command->SetCommandAction(actionFunction);
  unsigned long tag = m_ObservableBroadcast->AddObserver(DisplayScrollEvent(nullptr, 0), command);
  m_ObserverTags.push_back(tag);
}