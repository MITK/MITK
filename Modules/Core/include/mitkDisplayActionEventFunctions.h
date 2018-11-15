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

#include <MitkCoreExports.h>

#include "mitkStdFunctionCommand.h"

namespace mitk
{
  namespace DisplayActionEventFunctions
  {
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayMoveEvent'.
    *   The function performs a move of the camera controller of the sending renderer by a vector
    *   that was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction MoveSenderCameraAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplaySetCrosshairEvent'.
    *   The function performs a slice selection of the slice navigation controller and will set
    *   the cross hair for all 2D-render windows.
    *   The new position was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetCrosshairAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayZoomEvent'.
    *   The function performs a zoom of the camera controller of the sending renderer by a zoom factor
    *   that was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ZoomSenderCameraAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayScrollEvent'.
    *   The function performs a slice scrolling of the slice navigation controller of the sending renderer.
    *   The new position was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ScrollSliceStepperAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplaySetLevelWindowEvent'.
    *   The function sets the 'levelwindow' property of the topmost visible image that is display by the sending renderer.
    *   The level and window value for this property were previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetLevelWindowAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayMoveEvent'.
    *   The function performs a move of the camera controller of all renderer (synchronized)
    *   by a vector that was previously determined by the mouse interaction event.
    *   The renderer need to be managed by the same rendering manager.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction MoveCameraSynchronizedAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplaySetCrosshairEvent'.
    *   The function performs a slice selection of the slice navigation controller and will set
    *   the cross hair for all 2D-render windows.
    *   The new position was previously determined by the mouse interaction event.
    *   #TODO: currently there is no need to distinguish between this and the non-synchronized version
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction SetCrosshairSynchronizedAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayZoomEvent'.
    *   The function performs a zoom of the camera controller of all 2D-renderer (synchronized)
    *   by a zoom factor that was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ZoomCameraSynchronizedAction();
    /**
    * @brief Returns an 'std::function' that can be used  to react on the 'DisplayScrollEvent'.
    *   The function performs a slice scrolling of the slice navigation controller of all 2D-renderer (synchronized).
    *   The new position was previously determined by the mouse interaction event.
    */
    MITKCORE_EXPORT StdFunctionCommand::ActionFunction ScrollSliceStepperSynchronizedAction();

  } // end namespace DisplayActionEventFunctions
} // end namespace mitk

#endif // MITKDISPLAYACTIONEVENTFUNCTIONS_H
