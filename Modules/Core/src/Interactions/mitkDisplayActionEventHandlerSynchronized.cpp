/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDisplayActionEventHandlerSynchronized.h"

// mitk core
#include "mitkDisplayActionEventFunctions.h"

// itk
#include <itkEventObject.h>

void mitk::DisplayActionEventHandlerSynchronized::InitActionsImpl(std::string prefixFilter /* = "" */)
{
  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventFunctions::MoveCameraSynchronizedAction(prefixFilter);
  ConnectDisplayActionEvent(DisplayMoveEvent(nullptr, Vector2D()), actionFunction);

  actionFunction = DisplayActionEventFunctions::SetCrosshairSynchronizedAction(prefixFilter);
  ConnectDisplayActionEvent(DisplaySetCrosshairEvent(nullptr, Point3D()), actionFunction);

  actionFunction = DisplayActionEventFunctions::ZoomCameraSynchronizedAction(prefixFilter);
  ConnectDisplayActionEvent(DisplayZoomEvent(nullptr, 0.0, Point2D()), actionFunction);

  actionFunction = DisplayActionEventFunctions::ScrollSliceStepperSynchronizedAction(prefixFilter);
  ConnectDisplayActionEvent(DisplayScrollEvent(nullptr, 0, true), actionFunction);

  actionFunction = mitk::DisplayActionEventFunctions::SetLevelWindowAction(prefixFilter);
  ConnectDisplayActionEvent(mitk::DisplaySetLevelWindowEvent(nullptr, mitk::ScalarType(), mitk::ScalarType()), actionFunction);
}
