/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDisplayActionEventFunctions.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkDisplayActionEvents.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkTimeNavigationController.h>

namespace
{
  void ThrowOnNullPredicate(const mitk::DisplayActionEventFunctions::TargetPredicate& isTarget,
                            const char* factoryName)
  {
    if (!isTarget)
    {
      mitkThrow() << factoryName << ": the target predicate must not be null. A dimension "
                  << "that is not synchronized is expressed by wiring the sender-only "
                  << "action instead (see DisplayActionEventHandlerSynchronized::Predicates).";
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// STANDARD FUNCTIONS
//////////////////////////////////////////////////////////////////////////
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveSenderCameraAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayMoveEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayMoveEvent* displayActionEvent = dynamic_cast<const DisplayMoveEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      sendingRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
      RenderingManager::GetInstance()->RequestUpdate(sendingRenderer->GetRenderWindow());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      sendingRenderer->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomSenderCameraAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayZoomEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayZoomEvent* displayActionEvent = dynamic_cast<const DisplayZoomEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      if (1.0 != displayActionEvent->GetZoomFactor())
      {
        sendingRenderer->GetCameraController()->Zoom(displayActionEvent->GetZoomFactor(), displayActionEvent->GetStartCoordinate());
        RenderingManager::GetInstance()->RequestUpdate(sendingRenderer->GetRenderWindow());
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      mitk::SliceNavigationController* sliceNavigationController = sendingRenderer->GetSliceNavigationController();
      if (nullptr == sliceNavigationController)
      {
        return;
      }
      if (sliceNavigationController->GetSliceLocked())
      {
        return;
      }
      mitk::Stepper* stepper = sliceNavigationController->GetStepper();
      if (nullptr == stepper)
      {
        return;
      }

      // if only a single slice image was loaded, scrolling will affect the time steps
      if (stepper->GetSteps() <= 1)
      {
        auto* timeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
        stepper = timeNavigationController->GetStepper();
      }

      stepper->SetAutoRepeat(displayActionEvent->GetAutoRepeat());
      stepper->MoveSlice(displayActionEvent->GetSliceDelta());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetLevelWindowAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetLevelWindowEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetLevelWindowEvent* displayActionEvent = dynamic_cast<const DisplaySetLevelWindowEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      // get the the topmost visible image of the sending renderer
      DataStorage::Pointer storage = sendingRenderer->GetDataStorage();
      DataStorage::SetOfObjects::ConstPointer allImageNodes = storage->GetSubset(NodePredicateDataType::New("Image"));
      Point3D worldposition;
      const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(displayActionEvent->GetInteractionEvent());
      sendingRenderer->DisplayToWorld(positionEvent->GetPointerPositionOnScreen(), worldposition);
      auto globalCurrentTimePoint = sendingRenderer->GetTime();
      DataNode::Pointer node = FindTopmostVisibleNode(allImageNodes, worldposition, globalCurrentTimePoint, sendingRenderer);
      if (node.IsNull())
      {
        return;
      }

      LevelWindow levelWindow = LevelWindow();
      node->GetLevelWindow(levelWindow);
      ScalarType level = levelWindow.GetLevel();
      ScalarType window = levelWindow.GetWindow();

      level += displayActionEvent->GetLevel();
      window += displayActionEvent->GetWindow();

      levelWindow.SetLevelWindow(level, window);
      auto* levelWindowProperty = dynamic_cast<LevelWindowProperty*>(node->GetProperty("levelwindow"));
      if (nullptr != levelWindowProperty)
      {
        levelWindowProperty->SetLevelWindow(levelWindow);
        RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  };

  return actionFunction;
}

//////////////////////////////////////////////////////////////////////////
// SYNCHRONIZED FUNCTIONS
//////////////////////////////////////////////////////////////////////////
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction(TargetPredicate isTarget)
{
  ThrowOnNullPredicate(isTarget, "MoveCameraSynchronizedAction");

  auto actionFunction = [isTarget](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayMoveEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayMoveEvent* displayActionEvent = dynamic_cast<const DisplayMoveEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto renderingManager = RenderingManager::GetInstance();
      auto allRenderWindows = renderingManager->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && isTarget(sendingRenderer, targetRenderer))
        {
          targetRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
          renderingManager->RequestUpdate(renderWindow);
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction(const std::string& prefixFilter)
{
  auto actionFunction = [prefixFilter](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer || std::string(sendingRenderer->GetName()).rfind(prefixFilter, 0) != 0)
      {
        return;
      }

      auto allRenderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && std::string(targetRenderer->GetName()).rfind(prefixFilter, 0) == 0)
        {
          targetRenderer->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction(TargetPredicate isTarget)
{
  ThrowOnNullPredicate(isTarget, "SetCrosshairSynchronizedAction");

  auto actionFunction = [isTarget](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto allRenderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && isTarget(sendingRenderer, targetRenderer))
        {
          targetRenderer->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction(TargetPredicate isTarget)
{
  ThrowOnNullPredicate(isTarget, "ZoomCameraSynchronizedAction");

  auto actionFunction = [isTarget](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayZoomEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayZoomEvent* displayActionEvent = dynamic_cast<const DisplayZoomEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      if (1.0 != displayActionEvent->GetZoomFactor())
      {
        auto renderingManager = RenderingManager::GetInstance();
        auto allRenderWindows = renderingManager->GetAllRegisteredRenderWindows();
        for (auto renderWindow : allRenderWindows)
        {
          auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
          if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
              && isTarget(sendingRenderer, targetRenderer))
          {
            targetRenderer->GetCameraController()->Zoom(displayActionEvent->GetZoomFactor(), displayActionEvent->GetStartCoordinate());
            renderingManager->RequestUpdate(renderWindow);
          }
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetLevelWindowSynchronizedAction(TargetPredicate isTarget)
{
  ThrowOnNullPredicate(isTarget, "SetLevelWindowSynchronizedAction");

  auto actionFunction = [isTarget](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetLevelWindowEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetLevelWindowEvent* displayActionEvent = dynamic_cast<const DisplaySetLevelWindowEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }
      DataStorage::Pointer storage = sendingRenderer->GetDataStorage();
      if (storage.IsNull())
      {
        return;
      }
      const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(displayActionEvent->GetInteractionEvent());
      if (nullptr == positionEvent)
      {
        return;
      }

      // Resolve the gesture's node exactly like the node-global action: the
      // topmost image visible in the sending renderer under the pointer.
      DataStorage::SetOfObjects::ConstPointer allImageNodes = storage->GetSubset(NodePredicateDataType::New("Image"));
      Point3D worldposition;
      sendingRenderer->DisplayToWorld(positionEvent->GetPointerPositionOnScreen(), worldposition);
      const auto globalCurrentTimePoint = sendingRenderer->GetTime();
      DataNode::Pointer node = FindTopmostVisibleNode(allImageNodes, worldposition, globalCurrentTimePoint, sendingRenderer);
      if (node.IsNull())
      {
        return;
      }

      if (!isTarget(sendingRenderer, sendingRenderer))
      {
        // Sender not level-window-linked: the classic node-global write keeps
        // ungrouped renderers coupled to the global level/window controls.
        LevelWindow levelWindow;
        node->GetLevelWindow(levelWindow);
        levelWindow.SetLevelWindow(levelWindow.GetLevel() + displayActionEvent->GetLevel(),
                                   levelWindow.GetWindow() + displayActionEvent->GetWindow());
        auto* levelWindowProperty = dynamic_cast<LevelWindowProperty*>(node->GetProperty("levelwindow"));
        if (nullptr != levelWindowProperty)
        {
          levelWindowProperty->SetLevelWindow(levelWindow);
          RenderingManager::GetInstance()->RequestUpdateAll();
        }
        return;
      }

      auto renderingManager = RenderingManager::GetInstance();
      auto allRenderWindows = renderingManager->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && isTarget(sendingRenderer, targetRenderer))
        {
          // Delta on each member's own current value (renderer-specific,
          // falling back to node-global), written renderer-specific: members
          // keep their relative differences and the mapper prefers the
          // renderer-specific property from now on.
          LevelWindow levelWindow;
          node->GetLevelWindow(levelWindow, targetRenderer);
          levelWindow.SetLevelWindow(levelWindow.GetLevel() + displayActionEvent->GetLevel(),
                                     levelWindow.GetWindow() + displayActionEvent->GetWindow());
          node->SetProperty("levelwindow", LevelWindowProperty::New(levelWindow), targetRenderer);
          renderingManager->RequestUpdate(renderWindow);
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(TargetPredicate isTarget)
{
  ThrowOnNullPredicate(isTarget, "ScrollSliceStepperSynchronizedAction");

  auto actionFunction = [isTarget](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto allRenderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && isTarget(sendingRenderer, targetRenderer))
        {
          // A member that cannot take this scroll is skipped; the rest of the
          // group still receives the event.
          SliceNavigationController* sliceNavigationController = targetRenderer->GetSliceNavigationController();
          if (nullptr == sliceNavigationController)
          {
            continue;
          }
          if (sliceNavigationController->GetSliceLocked())
          {
            continue;
          }
          mitk::Stepper* stepper = sliceNavigationController->GetStepper();
          if (nullptr == stepper)
          {
            continue;
          }

          if (stepper->GetSteps() <= 1)
          {
            // Group propagation must never leak into application-global time,
            // so a single-slice member is simply not scrolled. Only the
            // sender's own gesture keeps the classic single-slice behavior of
            // scrolling the time steps instead.
            if (targetRenderer != sendingRenderer)
            {
              continue;
            }
            auto* timeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
            stepper = timeNavigationController->GetStepper();
          }

          stepper->SetAutoRepeat(displayActionEvent->GetAutoRepeat());
          stepper->MoveSlice(displayActionEvent->GetSliceDelta());
        }
      }
    }
  };

  return actionFunction;
}
