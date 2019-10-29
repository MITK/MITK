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

#include "mitkDisplayActionEventFunctions.h"

// mitk core
#include "mitkBaseRenderer.h"
#include "mitkCameraController.h"
#include "mitkDisplayActionEvents.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkLevelWindow.h"
#include "mitkLevelWindowProperty.h"
#include "mitkNodePredicateDataType.h"

//////////////////////////////////////////////////////////////////////////
// STANDARD FUNCTIONS
//////////////////////////////////////////////////////////////////////////
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveSenderCameraAction()
{
  mitk::StdFunctionCommand::ActionFunction actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayMoveEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayMoveEvent* displayActionEvent = dynamic_cast<const DisplayMoveEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      sendingRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
      sendingRenderer->GetRenderingManager()->RequestUpdate(sendingRenderer->GetRenderWindow());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      BaseRenderer::GetInstance(sendingRenderer->GetRenderWindow())->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomSenderCameraAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
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
        sendingRenderer->GetCameraController()->Zoom(displayActionEvent->GetZoomFactor(), displayActionEvent->GetStartCoordinate());
        sendingRenderer->GetRenderingManager()->RequestUpdate(sendingRenderer->GetRenderWindow());
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
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
      mitk::Stepper* sliceStepper = sliceNavigationController->GetSlice();
      if (nullptr == sliceStepper)
      {
        return;
      }

      // if only a single slice image was loaded, scrolling will affect the time steps
      if (sliceStepper->GetSteps() <= 1)
      {
        sliceStepper = sliceNavigationController->GetTime();
      }

      sliceStepper->MoveSlice(displayActionEvent->GetSliceDelta());
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetLevelWindowAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetLevelWindowEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetLevelWindowEvent* displayActionEvent = dynamic_cast<const DisplaySetLevelWindowEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
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
        sendingRenderer->GetRenderingManager()->RequestUpdateAll();
      }
    }
  };

  return actionFunction;
}

//////////////////////////////////////////////////////////////////////////
// SYNCHRONIZED FUNCTIONS
//////////////////////////////////////////////////////////////////////////
mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveCameraSynchronizedAction()
{
  mitk::StdFunctionCommand::ActionFunction actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayMoveEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayMoveEvent* displayActionEvent = dynamic_cast<const DisplayMoveEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto allRenderWindows = sendingRenderer->GetRenderingManager()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        if (BaseRenderer::GetInstance(renderWindow)->GetMapperID() == BaseRenderer::Standard2D)
        {
          BaseRenderer* currentRenderer = BaseRenderer::GetInstance(renderWindow);
          currentRenderer->GetCameraController()->MoveBy(displayActionEvent->GetMoveVector());
          currentRenderer->GetRenderingManager()->RequestUpdate(currentRenderer->GetRenderWindow());
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairSynchronizedAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplaySetCrosshairEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplaySetCrosshairEvent* displayActionEvent = dynamic_cast<const DisplaySetCrosshairEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto allRenderWindows = sendingRenderer->GetRenderingManager()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        if (BaseRenderer::GetInstance(renderWindow)->GetMapperID() == BaseRenderer::Standard2D)
        {
          BaseRenderer::GetInstance(renderWindow)->GetSliceNavigationController()->SelectSliceByPoint(displayActionEvent->GetPosition());
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomCameraSynchronizedAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
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
        auto allRenderWindows = sendingRenderer->GetRenderingManager()->GetAllRegisteredRenderWindows();
        for (auto renderWindow : allRenderWindows)
        {
          if (BaseRenderer::GetInstance(renderWindow)->GetMapperID() == BaseRenderer::Standard2D)
          {
            BaseRenderer* currentRenderer = BaseRenderer::GetInstance(renderWindow);
            currentRenderer->GetCameraController()->Zoom(displayActionEvent->GetZoomFactor(), displayActionEvent->GetStartCoordinate());
            currentRenderer->GetRenderingManager()->RequestUpdate(currentRenderer->GetRenderWindow());
          }
        }
      }
    }
  };

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetSender();
      if (nullptr == sendingRenderer)
      {
        return;
      }

      auto allRenderWindows = sendingRenderer->GetRenderingManager()->GetAllRegisteredRenderWindows();
      for (auto renderWindow : allRenderWindows)
      {
        if (BaseRenderer::GetInstance(renderWindow)->GetMapperID() == BaseRenderer::Standard2D)
        {
          mitk::SliceNavigationController* sliceNavigationController = BaseRenderer::GetInstance(renderWindow)->GetSliceNavigationController();
          if (nullptr == sliceNavigationController)
          {
            return;
          }
          if (sliceNavigationController->GetSliceLocked())
          {
            return;
          }
          mitk::Stepper* sliceStepper = sliceNavigationController->GetSlice();
          if (nullptr == sliceStepper)
          {
            return;
          }

          // if only a single slice image was loaded, scrolling will affect the time steps
          if (sliceStepper->GetSteps() <= 1)
          {
            sliceStepper = sliceNavigationController->GetTime();
          }

          sliceStepper->MoveSlice(displayActionEvent->GetSliceDelta());
        }
      }
    }
  };

  return actionFunction;
}
