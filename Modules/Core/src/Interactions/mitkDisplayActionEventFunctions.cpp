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
#include "mitkInteractionEvent.h"

// itk
#include <itkEventObject.h>

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::MoveSenderCameraAction()
{
  mitk::StdFunctionCommand::ActionFunction actionFunction = [](const itk::EventObject& displayInteractorEvent)
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

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::SetCrosshairAction()
{
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

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ZoomSenderCameraAction()
{
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

  return actionFunction;
}

mitk::StdFunctionCommand::ActionFunction mitk::DisplayActionEventFunctions::ScrollSliceStepperAction()
{
  auto actionFunction = [](const itk::EventObject& displayInteractorEvent)
  {
    if (DisplayScrollEvent().CheckEvent(&displayInteractorEvent))
    {
      const DisplayScrollEvent* displayActionEvent = dynamic_cast<const DisplayScrollEvent*>(&displayInteractorEvent);
      const BaseRenderer::Pointer sendingRenderer = displayActionEvent->GetInteractionEvent()->GetSender();

      // concrete action
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
