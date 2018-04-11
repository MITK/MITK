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

#include "DisplayActionEventDefaultFunctions.h"

// itk
#include <itkEventObject.h>

StdFunctionCommand::ActionFunction mitk::DisplayActionEventDefaultFunctions::DefaultMoveAction()
{
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

  return actionFunction;
}

StdFunctionCommand::ActionFunction mitk::DisplayActionEventDefaultFunctions::DefaultSetCrosshairAction()
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

void mitk::DisplayActionEventDefaultFunctions::DefaultZoomAction()
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

void mitk::DisplayActionEventDefaultFunctions::DefaultScrollAction()
{
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

  return;
}
