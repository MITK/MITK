/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
