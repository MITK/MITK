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

#ifndef MITKDISPLAYACTIONEVENTFUNCTIONS_H
#define MITKDISPLAYACTIONEVENTFUNCTIONS_H

#include "mitkStdFunctionCommand.h"

namespace mitk
{
  namespace DisplayActionEventFunctions
  {
    /**
    * @brief Returns a command that reacts on the 'DisplayMoveEvent'.
    *   The command  performs a move of the camera controller of the sending renderer by a vector
    *   that was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction MoveSenderCameraAction();
    /**
    * @brief Returns a command that reacts on the 'DisplaySetCrosshairEvent'.
    *   The command performs a slice selection of the slice navigation controller and will set
    *   the cross hair for all render windows.
    *   The new position was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction SetCrosshairAction();
    /**
    * @brief Returns a command that reacts on the 'DisplayZoomEvent'.
    *   The command performs a zoom of the camera controller of the sending renderer by a zoom factor
    *   that was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction ZoomSenderCameraAction();
    /**
    * @brief Returns a command that reacts on the 'DisplayScrollEvent'.
    *   The command performs a slice scrolling of the slice navigation controller of the sending renderer.
    *   The new position was previously determined by the mouse interaction event.
    */
    StdFunctionCommand::ActionFunction ScrollSliceStepperAction();

  } // end namespace DisplayActionEventFunctions
} // end namespace mitk

#endif // MITKDISPLAYACTIONEVENTFUNCTIONS_H
