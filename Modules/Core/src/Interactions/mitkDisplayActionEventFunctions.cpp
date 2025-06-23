/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDisplayActionEventFunctions.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkDisplayActionEvents.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkTimeNavigationController.h>

//////////////////////////////////////////////////////////////////////////
// STANDARD FUNCTIONS
//////////////////////////////////////////////////////////////////////////
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveSenderCameraAction(std::string prefixFilter)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairAction(std::string prefixFilter)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomSenderCameraAction(std::string prefixFilter)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperAction(std::string prefixFilter)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetLevelWindowAction(std::string prefixFilter)
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
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction(std::string prefixFilter)
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

      auto renderingManager = RenderingManager::GetInstance();
      auto allRenderWindows = renderingManager->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && std::string(targetRenderer->GetName()).rfind(prefixFilter, 0) == 0)
        {
          targetRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
          renderingManager->RequestUpdate(renderWindow);
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction(std::string prefixFilter)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction(std::string prefixFilter)
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
        auto renderingManager = RenderingManager::GetInstance();
        auto allRenderWindows = renderingManager->GetAllRegisteredRenderWindows();
        for (auto renderWindow : allRenderWindows)
        {
          auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
          if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
              && std::string(targetRenderer->GetName()).rfind(prefixFilter, 0) == 0)
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

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(std::string prefixFilter)
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

      auto allRenderWindows = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        auto targetRenderer = BaseRenderer::GetInstance(renderWindow);
        if (targetRenderer->GetMapperID() == BaseRenderer::Standard2D
            && std::string(targetRenderer->GetName()).rfind(prefixFilter, 0) == 0)
        {
          SliceNavigationController* sliceNavigationController = targetRenderer->GetSliceNavigationController();
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
      }
    }
  };

  return actionFunction;
}
