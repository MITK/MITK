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

#include "mitkDisplayActionEventHandlerStd.h"

// mitk core
#include "mitkDisplayActionEventFunctions.h"

// itk
#include <itkEventObject.h>

void mitk::DisplayActionEventHandlerStd::InitActionsImpl()
{
  // synchronized action event function
  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventFunctions::SetCrosshairSynchronizedAction();
  ConnectDisplayActionEvent(DisplaySetCrosshairEvent(nullptr, Point3D()), actionFunction);

  // desynchronized action event function
  actionFunction = DisplayActionEventFunctions::MoveSenderCameraAction();
  ConnectDisplayActionEvent(DisplayMoveEvent(nullptr, Vector2D()), actionFunction);

  // desynchronized action event function
  actionFunction = DisplayActionEventFunctions::ZoomSenderCameraAction();
  ConnectDisplayActionEvent(DisplayZoomEvent(nullptr, 0.0, Point2D()), actionFunction);

  // desynchronized action event function
  actionFunction = DisplayActionEventFunctions::ScrollSliceStepperAction();
  ConnectDisplayActionEvent(DisplayScrollEvent(nullptr, 0), actionFunction);

  actionFunction = mitk::DisplayActionEventFunctions::SetLevelWindowAction();
  ConnectDisplayActionEvent(mitk::DisplaySetLevelWindowEvent(nullptr, mitk::ScalarType(), mitk::ScalarType()), actionFunction);
}
