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
#include "mitkDisplayActionEventDefaultFunctions.h"

// itk
#include <itkEventObject.h>

void mitk::StdDisplayActionEventHandler::InitStdActions()
{
  // #TODO: change function call for new mitk::WeakPointer
  if (m_ObservableBroadcast.IsNull())
  {
    mitkThrow() << "No display action event broadcast class set to observe. Use 'SetObservableBroadcast' before initializing actions.";
  }

  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventDefaultFunctions::DefaultMoveAction();
  ConnectDisplayActionEvent(DisplayMoveEvent(nullptr, Vector2D()), actionFunction);

  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventDefaultFunctions::DefaultSetCrosshairAction();
  ConnectDisplayActionEvent(DisplaySetCrosshairEvent(nullptr, Point3D()), actionFunction);

  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventDefaultFunctions::DefaultZoomAction();
  ConnectDisplayActionEvent(DisplayZoomEvent(nullptr, 0.0, Point2D()), actionFunction);

  StdFunctionCommand::ActionFunction actionFunction = DisplayActionEventDefaultFunctions::DefaultScrollAction();
  ConnectDisplayActionEvent(DisplayScrollEvent(nullptr, 0), actionFunction);
}
